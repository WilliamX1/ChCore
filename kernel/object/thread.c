/*
 * Copyright (c) 2022 Institute of Parallel And Distributed Systems (IPADS)
 * ChCore-Lab is licensed under the Mulan PSL v1.
 * You can use this software according to the terms and conditions of the Mulan PSL v1.
 * You may obtain a copy of Mulan PSL v1 at:
 *     http://license.coscl.org.cn/MulanPSL
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR
 * PURPOSE.
 * See the Mulan PSL v1 for more details.
 */

#include <common/kprint.h>
#include <common/macro.h>
#include <common/types.h>
#include <common/util.h>
#include <lib/elf.h>
#include <mm/kmalloc.h>
#include <mm/mm.h>
#include <mm/uaccess.h>
#include <object/thread.h>
#include <sched/context.h>
#include <arch/machine/registers.h>
#include <arch/machine/smp.h>
#include <arch/time.h>
#include <irq/ipi.h>

#include "thread_env.h"

static int thread_init(struct thread *thread, struct cap_group *cap_group,
                       u64 stack, u64 pc, u32 prio, u32 type, s32 aff)
{
        thread->cap_group =
                obj_get(cap_group, CAP_GROUP_OBJ_ID, TYPE_CAP_GROUP);
        thread->vmspace = obj_get(cap_group, VMSPACE_OBJ_ID, TYPE_VMSPACE);
        obj_put(thread->cap_group);
        obj_put(thread->vmspace);

        /* Thread context is used as the kernel stack for that thread */
        thread->thread_ctx = create_thread_ctx(type);
        if (!thread->thread_ctx)
                return -ENOMEM;
        init_thread_ctx(thread, stack, pc, prio, type, aff % PLAT_CPU_NUM);

        /*
         * Field prev_thread records the previous thread runs
         * just before this thread. Obviously, it is NULL at the beginning.
         */
        thread->prev_thread = NULL;

        /* The ipc_config will be allocated on demand */
        thread->general_ipc_config = NULL;
        return 0;
}

void thread_deinit(void *thread_ptr)
{
        struct thread *thread;
        // struct cap_group *cap_group;

        thread = (struct thread *)thread_ptr;

        BUG_ON(thread->thread_ctx->thread_exit_state != TE_EXITED);
        if (thread->thread_ctx->state != TS_EXIT)
                kwarn("thread ctx->state is %d\n", thread->thread_ctx->state);

        // cap_group = thread->cap_group;
        list_del(&thread->node);

        if (thread->general_ipc_config)
                kfree(thread->general_ipc_config);

        destroy_thread_ctx(thread);

        /* The thread struct itself will be freed in __free_object */
}

#define PFLAGS2VMRFLAGS(PF)                                       \
        (((PF)&PF_X ? VMR_EXEC : 0) | ((PF)&PF_W ? VMR_WRITE : 0) \
         | ((PF)&PF_R ? VMR_READ : 0))

#define OFFSET_MASK (0xFFF)

/* load binary into some process (cap_group) */
static u64 load_binary(struct cap_group *cap_group, struct vmspace *vmspace,
                       const char *bin, struct process_metadata *metadata)
{
        struct elf_file *elf;
        vmr_prop_t flags;
        int i, r;
        size_t seg_sz, seg_map_sz;
        u64 p_vaddr;

        int *pmo_cap;
        struct pmobject *pmo;
        u64 ret;

        elf = elf_parse_file(bin);
        pmo_cap = kmalloc(elf->header.e_phnum * sizeof(*pmo_cap));
        if (!pmo_cap) {
                r = -ENOMEM;
                goto out_fail;
        }

        /* load each segment in the elf binary */
        for (i = 0; i < elf->header.e_phnum; ++i) {
                pmo_cap[i] = -1;
                if (elf->p_headers[i].p_type == PT_LOAD) {
                        seg_sz = elf->p_headers[i].p_memsz;
                        p_vaddr = elf->p_headers[i].p_vaddr;
                        /* LAB 3 TODO BEGIN */

                        seg_map_sz = ROUND_UP(seg_sz + p_vaddr, PAGE_SIZE) - ROUND_DOWN(p_vaddr, PAGE_SIZE);
                        pmo = obj_alloc(TYPE_PMO, sizeof(*pmo));

                        memset((void*)pmo, 0, sizeof(*pmo));
                        pmo->size = seg_map_sz;
                        pmo->type = PMO_DATA;
                        pmo->start = (paddr_t)virt_to_phys(kmalloc(seg_map_sz));
                        pmo_cap[i] = cap_alloc(cap_group, pmo, 0);

                        memset((void *)phys_to_virt(pmo->start), 0, pmo->size);
                        memcpy((void *)phys_to_virt(pmo->start) + (elf->p_headers[i].p_vaddr & OFFSET_MASK), bin + elf->p_headers[i].p_offset, elf->p_headers[i].p_filesz);

                        flags = PFLAGS2VMRFLAGS(elf->p_headers[i].p_flags);
                        ret = vmspace_map_range(vmspace, ROUND_DOWN(p_vaddr, PAGE_SIZE), seg_map_sz, flags, pmo);

                        /* LAB 3 TODO END */
                        BUG_ON(ret != 0);
                }
        }

        /* return binary metadata */
        if (metadata != NULL) {
                metadata->phdr_addr =
                        elf->p_headers[0].p_vaddr + elf->header.e_phoff;
                metadata->phentsize = elf->header.e_phentsize;
                metadata->phnum = elf->header.e_phnum;
                metadata->flags = elf->header.e_flags;
                metadata->entry = elf->header.e_entry;
        }

        /* PC: the entry point */
        return elf->header.e_entry;
out_free_cap:
        for (--i; i >= 0; i--) {
                if (pmo_cap[i] != 0)
                        cap_free(cap_group, pmo_cap[i]);
        }
out_fail:
        return r;
}

/* Defined in page_table.S (maybe required on aarch64) */
extern void flush_idcache(void);

/* Required by LibC */
extern void prepare_env(char *env, u64 top_vaddr, struct process_metadata *meta,
                        char *name);
/*
 * This is for creating the first thread in the first (init) user process.
 * So, __create_root_thread needs to load the code/data as well.
 */
static int __create_root_thread(struct cap_group *cap_group, u64 stack_base,
                                u64 stack_size, u32 prio, u32 type, s32 aff,
                                const char *bin_start, char *bin_name)
{
        int ret, thread_cap, stack_pmo_cap;
        struct thread *thread;
        struct pmobject *stack_pmo;
        struct vmspace *init_vmspace;
        struct process_metadata meta;
        u64 stack;
        u64 pc;
        vaddr_t kva;

        init_vmspace = obj_get(cap_group, VMSPACE_OBJ_ID, TYPE_VMSPACE);
        obj_put(init_vmspace);

        /* Allocate and setup a user stack for the init thread */
        stack_pmo_cap =
                create_pmo(stack_size, PMO_ANONYM, cap_group, &stack_pmo);
        if (stack_pmo_cap < 0) {
                ret = stack_pmo_cap;
                goto out_fail;
        }

        ret = vmspace_map_range(init_vmspace,
                                stack_base,
                                stack_size,
                                VMR_READ | VMR_WRITE,
                                stack_pmo);
        BUG_ON(ret != 0);

        /* Allocate the init thread */
        thread = obj_alloc(TYPE_THREAD, sizeof(*thread));
        if (!thread) {
                ret = -ENOMEM;
                goto out_free_cap_pmo;
        }

        /* Fill the parameter of the thread struct */
        pc = load_binary(cap_group, init_vmspace, bin_start, &meta);
        stack = stack_base + stack_size;

        /* Allocate a physical for the main stack for prepare_env */
        kva = (vaddr_t)get_pages(0);
        BUG_ON(kva == 0);
        commit_page_to_pmo(stack_pmo,
                           stack_size / PAGE_SIZE - 1,
                           virt_to_phys((void *)kva));

        prepare_env((char *)kva, stack, &meta, bin_name);
        stack -= ENV_SIZE;

        ret = thread_init(thread, cap_group, stack, pc, prio, type, aff);
        BUG_ON(ret != 0);

        /* Add the thread into the thread_list of the cap_group */
        list_add(&thread->node, &cap_group->thread_list);
        cap_group->thread_cnt += 1;

        /* Allocate the cap for the init thread */
        thread_cap = cap_alloc(cap_group, thread, 0);
        if (thread_cap < 0) {
                ret = thread_cap;
                goto out_free_obj_thread;
        }

        /* L1 icache & dcache have no coherence on aarch64 */
        flush_idcache();

        return thread_cap;

out_free_obj_thread:
        obj_free(thread);
out_free_cap_pmo:
        cap_free(cap_group, stack_pmo_cap);
out_fail:
        return ret;
}

/*
 * exported functions
 */
void switch_thread_vmspace_to(struct thread *thread)
{
        switch_vmspace_to(thread->vmspace);
}

/* Arguments for the inital thread */
#define ROOT_THREAD_STACK_BASE (0x500000000000UL)
#define ROOT_THREAD_STACK_SIZE (0x800000UL)
#define ROOT_THREAD_PRIO       MAX_PRIO - 1

char ROOT_NAME[] = "/" CHCORE_ROOT_PROGRAM;
/* defined in asm code generated from incbin.tpl.S */
extern const char __binary_root_start;
void test_root_thread_basic(const struct cap_group *ptr)
{
        BUG_ON(ptr == NULL);
        BUG_ON(container_of(ptr, struct object, opaque)->type
               != TYPE_CAP_GROUP);
        kinfo("Cap_create Pretest Ok!\n");
}

void test_root_thread_after_create(const struct cap_group *ptr,
                                   const int thread_cap)
{
        BUG_ON(ptr->thread_cnt == 0);
        BUG_ON(thread_cap == 0);
}

/*
 * The root_thread is actually a first user thread
 * which has no difference with other user threads
 */
void create_root_thread(void)
{
        struct cap_group *root_cap_group;
        int thread_cap;
        struct thread *root_thread;

        root_cap_group = create_root_cap_group(ROOT_NAME, strlen(ROOT_NAME));
        test_root_thread_basic(root_cap_group);
        thread_cap = __create_root_thread(root_cap_group,
                                          ROOT_THREAD_STACK_BASE,
                                          ROOT_THREAD_STACK_SIZE,
                                          ROOT_THREAD_PRIO,
                                          TYPE_USER,
                                          smp_get_cpu_id(),
                                          &__binary_root_start,
                                          ROOT_NAME);
        test_root_thread_after_create(root_cap_group, thread_cap);

        root_thread = obj_get(root_cap_group, thread_cap, TYPE_THREAD);
        /* Enqueue: put init thread into the ready queue */
        BUG_ON(sched_enqueue(root_thread));
        obj_put(root_thread);
}

/*
 * create a thread in some process
 * return the thread_cap in the target cap_group
 */

static int create_thread(struct cap_group *cap_group, u64 stack, u64 pc,
                         u64 arg, u32 prio, u32 type)
{
        struct thread *thread;
        int cap, ret = 0;

        if (!cap_group) {
                ret = -ECAPBILITY;
                goto out_fail;
        }
        thread = obj_alloc(TYPE_THREAD, sizeof(*thread));
        if (!thread) {
                ret = -ENOMEM;
                goto out_obj_put;
        }
        ret = thread_init(thread, cap_group, stack, pc, prio, type, NO_AFF);
        if (ret != 0)
                goto out_free_obj;

        list_add(&thread->node, &cap_group->thread_list);
        cap_group->thread_cnt += 1;

        arch_set_thread_arg0(thread, arg);

        /* set arch-specific thread state */
        set_thread_arch_spec_state(thread);

        /* cap is thread_cap in the target cap_group */
        cap = cap_alloc(cap_group, thread, 0);
        if (cap < 0) {
                ret = cap;
                goto out_free_obj;
        }
        /* ret is thread_cap in the current_cap_group */
        cap = cap_copy(cap_group, current_cap_group, cap);
        if (type == TYPE_USER) {
                thread->thread_ctx->state = TS_INTER;
                BUG_ON(sched_enqueue(thread));
        } else if (type == TYPE_SHADOW) {
                thread->thread_ctx->state = TS_WAITING;
        }
        return cap;

out_free_obj:
        obj_free(thread);
out_obj_put:
        obj_put(cap_group);
out_fail:
        return ret;
}

/**
 * FIXME(MK): This structure is duplicated in chcore musl headers.
 */
struct thread_args {
        u64 cap_group_cap;
        u64 stack;
        u64 pc;
        u64 arg;
        u32 prio;
        /* 0: TYPE_USER; 1: TYPE_SHADOW */
        u32 type;
};

/*
 * Create a pthread in some process
 * return the thread_cap in the target cap_group
 */
int sys_create_thread(u64 thread_args_p)
{
        struct thread_args args = {0};
        struct cap_group *cap_group;
        int thread_cap;
        int r;
        u32 type;

        r = copy_from_user((char *)&args, (char *)thread_args_p, sizeof(args));
        BUG_ON(r);

        cap_group =
                obj_get(current_cap_group, args.cap_group_cap, TYPE_CAP_GROUP);

        switch (args.type) {
        case 0:
                type = TYPE_USER;
                break;
        case 1:
                type = TYPE_SHADOW;
                break;
        default:
                kinfo("%s: invalid thread type.\n", __func__);
                thread_cap = -EINVAL;
                goto out;
        }

        thread_cap = create_thread(
                cap_group, args.stack, args.pc, args.arg, args.prio, type);

out:
        obj_put(cap_group);
        return thread_cap;
}

/* Exit the current running thread */
void sys_thread_exit(void)
{
#ifdef CHCORE_LAB3_TEST
        printk("\nBack to kernel.\n");
#endif
        /* LAB 3 TODO BEGIN */

        current_thread->thread_ctx->state = TS_EXIT;
        current_thread = NULL;
        
        /* LAB 3 TODO END */
        /* Reschedule */
        sched();
        eret_to_thread(switch_context());
}

/*
 * Lab4
 * Finish the sys_set_affinity
 * You do not need to schedule out current thread immediately,
 * as it is the duty of sys_yield()
 */
int sys_set_affinity(u64 thread_cap, s32 aff)
{
        struct thread *thread = NULL;
        int ret = 0;

        /* Check aff */
        if (aff >= PLAT_CPU_NUM) {
                ret = -EINVAL;
                goto out;
        }

        /* ChCore uses -1 to represent the current thread */
        if (thread_cap == -1) {
                thread = current_thread;
        } else {
                thread = obj_get(current_cap_group, thread_cap, TYPE_THREAD);
        }

        if (thread == NULL) {
                ret = -ECAPBILITY;
                goto out;
        }

        /* LAB 4 TODO BEGIN */
        thread->thread_ctx->affinity = aff;
        /* LAB 4 TODO END */
        if (thread_cap != -1)
                obj_put((void *)thread);
out:
        return ret;
}

s32 sys_get_affinity(u64 thread_cap)
{
        struct thread *thread = NULL;
        s32 aff = 0;

        /* ChCore use -1 to represent the current thread */
        if (thread_cap == -1) {
                thread = current_thread;
                BUG_ON(!thread);
        } else {
                thread = obj_get(current_cap_group, thread_cap, TYPE_THREAD);
        }
        if (thread == NULL)
                return -ECAPBILITY;
        /* LAB 4 TODO BEGIN */
        aff = thread->thread_ctx->affinity;
        /* LAB 4 TODO END */

        if (thread_cap != -1)
                obj_put((void *)thread);
        return aff;
}
