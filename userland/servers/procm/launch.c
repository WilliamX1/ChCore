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

#include "launch.h"

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <malloc.h>
#include <chcore/assert.h>
#include <chcore/memory.h>
#include <chcore/thread.h>
#include <chcore/capability.h>
#include <chcore/internal/mem_layout.h>
#include <chcore/internal/raw_syscall.h>
#include <chcore/internal/utils.h>

#define AT_NULL     0 /* end of vector */
#define AT_IGNORE   1 /* entry should be ignored */
#define AT_EXECFD   2 /* file descriptor of program */
#define AT_PHDR     3 /* program headers for program */
#define AT_PHENT    4 /* size of program header entry */
#define AT_PHNUM    5 /* number of program headers */
#define AT_PAGESZ   6 /* system page size */
#define AT_BASE     7 /* base address of interpreter */
#define AT_FLAGS    8 /* flags */
#define AT_ENTRY    9 /* entry point of program */
#define AT_NOTELF   10 /* program is not ELF */
#define AT_UID      11 /* real uid */
#define AT_EUID     12 /* effective uid */
#define AT_GID      13 /* real gid */
#define AT_EGID     14 /* effective gid */
#define AT_PLATFORM 15 /* string identifying CPU for optimizations */
#define AT_HWCAP    16 /* arch dependent hints at CPU capabilities */
#define AT_CLKTCK   17 /* frequency at which times() increments */
/* AT_* values 18 through 22 are reserved */
#define AT_SECURE 23 /* secure mode boolean */
#define AT_BASE_PLATFORM                            \
        24 /* string identifying real platform, may \
            * differ from AT_PLATFORM. */
#define AT_RANDOM 25 /* address of 16 random bytes */
#define AT_HWCAP2 26 /* extension of AT_HWCAP */

#define AT_EXECFN 31 /* filename of program */

char init_env[0x1000];

/**
 * NOTE: The stack format:
 * http://articles.manugarg.com/aboutelfauxiliaryvectors.html
 * The url shows a 32-bit format stack, but we implemented a 64-bit stack below.
 * People as smart as you could understand the difference.
 */
static void construct_init_env(char *env, u64 top_vaddr,
                               struct process_metadata *meta, char *name,
                               int caps[], int nr_caps, int argc, char **argv,
                               int pid)
{
        int i;
        u64 *p;
        char *str;
        size_t l;
        int str_bytes_left;

        /* clear init_env */
        memset(env, 0x1000, 0);

        str_bytes_left = 0x1000 >> 1;
        str = env + (0x1000 >> 1);
#define __ptr2vaddr(ptr) ((top_vaddr)-0x1000 + ((ptr) - (env)))

        /*
         * Prepare the stack env page.
         * Layout:
         *   ENV_MAGIC
         *   nr_caps, ENV_NO_CAPS if none
         *   caps...
         *   ----- below is standard env page
         *   argc
         *   argv...
         *   NULL
         *   envp...
         *   NULL
         *   auxv...
         *   NULL
         */
        p = (u64 *)env;
        *p++ = (u64)ENV_MAGIC;

        /* caps */
        if (nr_caps == 0) {
                /* no caps, set a special number */
                *p++ = (u64)ENV_NO_CAPS;
        } else {
                *p++ = (u64)nr_caps;
                for (i = 0; i < nr_caps; ++i) {
                        *p++ = (u64)caps[i];
                }
        }

        /* argc */
        *p++ = argc;

        /* argv */
        for (i = 0; i < argc; ++i) {
                l = strlen(argv[i]);
                chcore_assert(str_bytes_left > l);
                /* Copy the string. */
                strcpy(str, argv[i]);
                /* Set the pointer. */
                *p++ = __ptr2vaddr(str);
                str += l + 1;
                str_bytes_left -= l + 1;
        }
        *p++ = (u64)NULL;

        /* envp */
        *p++ = (u64)NULL;

        /* auxv */
        *p++ = AT_SECURE;
        *p++ = 0;

        *p++ = AT_PAGESZ;
        *p++ = 0x1000;

        *p++ = AT_PHDR;
        *p++ = meta->phdr_addr;

        *p++ = AT_PHENT;
        *p++ = meta->phentsize;

        *p++ = AT_PHNUM;
        *p++ = meta->phnum;

        *p++ = AT_FLAGS;
        *p++ = meta->flags;

        *p++ = AT_ENTRY;
        *p++ = meta->entry;

        *p++ = AT_UID;
        *p++ = 1000;

        *p++ = AT_EUID;
        *p++ = 1000;

        *p++ = AT_GID;
        *p++ = 1000;

        *p++ = AT_EGID;
        *p++ = 1000;

        *p++ = AT_CLKTCK;
        *p++ = 100;

        *p++ = AT_HWCAP;
        *p++ = 0;

        *p++ = AT_PLATFORM;
        {
                l = strlen(CHCORE_ARCH);
                chcore_assert(str_bytes_left > l);
                /* Copy the PLAT string. */
                strcpy(str, CHCORE_ARCH);
                /* Set the PLAT pointer. */
                *p++ = __ptr2vaddr(str);
                str += l + 1;
                str_bytes_left -= l + 1;
        }

        *p++ = AT_RANDOM;
        *p++ = top_vaddr - 64; /* random 16 bytes */

        /* add more auxv here */

        *p++ = AT_BASE;
        *p++ = 0;

        *p++ = AT_NULL;
        *p++ = 0;
}

/*
 * user_elf: elf struct
 * child_process_cap: if not NULL, set to child_process_cap that can be
 *                    used in current process.
 *
 * child_main_thread_cap: if not NULL, set to child_main_thread_cap
 *                        that can be used in current process.
 *
 * caps, nr_caps: copy from farther process to child process
 *
 * cpuid: affinity
 *
 * argc/argv: the number of arguments and the arguments.
 *
 * pcid: the pcid is fixed for root process and servers,
 * 	 and the rest is generated by procm.
 */
int launch_process(struct launch_process_args *lp_args)
{
        int new_process_cap;
        char *process_name;
        int main_thread_cap;
        int ret;
        long pc;
        int main_stack_cap;
        int forbid_area_cap;
        u64 offset;
        u64 stack_top;
        u64 p_vaddr;
        int i;
        struct pmo_map_request pmo_map_requests[2 + ELF_MAX_LOAD_SEG];
        int *transfer_caps = NULL;
        struct user_elf *user_elf = lp_args->user_elf;
        int *child_process_cap = lp_args->child_process_cap;
        int *child_main_thread_cap = lp_args->child_main_thread_cap;
        int *caps = lp_args->caps;
        int nr_caps = lp_args->nr_caps;
        s32 cpuid = lp_args->cpuid;
        int argc = lp_args->argc;
        char **argv = lp_args->argv;
        int pid = lp_args->pid;
        u64 pcid = lp_args->pcid;
        struct thread_args args;

        process_name = user_elf->path;

        /* create a new process with an empty vmspace */
        new_process_cap = __chcore_sys_create_cap_group(
                pid, (u64)process_name, strlen(process_name), pcid);
        if (new_process_cap < 0) {
                printf("%s: fail to create new_process_cap (ret: %d)\n",
                       __func__,
                       new_process_cap);
                goto fail;
        }

        if (nr_caps > 0) {
                transfer_caps = malloc(sizeof(int) * nr_caps);
                ret = chcore_cap_transfer_multi(
                        new_process_cap, caps, nr_caps, transfer_caps);
                if (ret != 0) {
                        printf("transfer caps ret %d\n", ret);
                        goto fail;
                }
        }

        pc = user_elf->elf_meta.entry;

        /*
         * Lab4: create pmo for the stack of main thread stack in current
         * process.
         *  You do not need to modify code in this scope
         */
        /* LAB 4 TODO BEGIN: create pmo for main_stack_cap */
        main_stack_cap = __chcore_sys_create_pmo(MAIN_THREAD_STACK_SIZE, PMO_ANONYM);
        /* LAB 4 TODO END */
        if (main_stack_cap < 0) {
                printf("%s: fail to pmo_create (ret: %d)\n", __func__, ret);
                goto fail;
        }

        /**
         * Hints: refer to <mem_layout_arch.h>
         * For stack_top, what's the virtual address of top of the main
         * thread's stack?
         *
         * For stack_offset, when the main thread gets
         * to execute the first time, what's the virtual adress the sp
         * register points to?
         * stack_offset is the offset from main thread's stack base to
         * that address.
         */
        /* Prepare the stack */
        /* LAB 4 TODO BEGIN: set stack_top and offset */
        stack_top = MAIN_THREAD_STACK_BASE + MAIN_THREAD_STACK_SIZE;
        offset = MAIN_THREAD_STACK_SIZE - 0x1000;
        /* LAB 4 TODO END */
        construct_init_env(init_env,
                           stack_top,
                           &user_elf->elf_meta,
                           user_elf->path,
                           transfer_caps,
                           nr_caps,
                           argc,
                           argv,
                           pid);
        free(transfer_caps);
        ret = chcore_pmo_write(main_stack_cap, offset, init_env, 0x1000);
        if (ret != 0) {
                printf("%s: fail to write_pmo (ret: %d)\n", __func__, ret);
                goto fail;
        }

        /**
         *  map the the main thread stack's pmo in the new process.
         *  Both VM_READ and VM_WRITE permission should be set.
         */
        /* LAB 4 TODO BEGIN: fill pmo_map_requests */
        pmo_map_requests[0].pmo_cap = main_stack_cap;
        pmo_map_requests[0].addr = MAIN_THREAD_STACK_BASE;
        pmo_map_requests[0].perm = VM_READ | VM_WRITE;
        /* LAB 4 TODO END */

        /* Map each segment in the elf binary */
        for (i = 0; i < ELF_MAX_LOAD_SEG; ++i) {
                if (user_elf->user_elf_seg[i].elf_pmo == -1) {
                        /* reach the last LOAD segment */
                        break;
                }
                pmo_map_requests[1 + i].pmo_cap =
                        user_elf->user_elf_seg[i].elf_pmo;
                pmo_map_requests[1 + i].addr = ROUND_DOWN(
                        user_elf->user_elf_seg[i].p_vaddr, PAGE_SIZE);
                pmo_map_requests[1 + i].perm = user_elf->user_elf_seg[i].flags;
                pmo_map_requests[1 + i].free_cap = 1;
        }

        ret = chcore_pmo_map_multi(new_process_cap, pmo_map_requests, 1 + i);

        if (ret != 0) {
                printf("%s: fail to map_pmos (ret: %d)\n", __func__, ret);
                goto fail;
        }

        /*
         * create main thread in the new process.
         * main_thread_cap is the cap can be used in current process.
         */
        args.cap_group_cap = new_process_cap;
        /* LAB 4 TODO BEGIN: set the stack for main thread */
        args.stack = MAIN_THREAD_STACK_BASE + offset;
        /* LAB 4 TODO END */
        args.pc = pc;
        args.arg = (u64)NULL;
        args.prio = MAX_PRIO;
        args.type = TYPE_USER;
        main_thread_cap = __chcore_sys_create_thread((u64)&args);

        if (child_process_cap)
                *child_process_cap = new_process_cap;
        if (child_main_thread_cap)
                *child_main_thread_cap = main_thread_cap;

        return 0;
fail:
        return -EINVAL;
}
