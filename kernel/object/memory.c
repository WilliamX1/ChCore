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

#include <object/object.h>
#include <object/thread.h>
#include <object/memory.h>
#include <mm/vmspace.h>
#include <mm/uaccess.h>
#include <mm/mm.h>
#include <mm/kmalloc.h>
#include <arch/mmu.h>
#include <mm/mm_check.h>

#include "mmap.h"

int pmo_init(struct pmobject *pmo, pmo_type_t type, size_t len, paddr_t paddr);

int sys_create_device_pmo(u64 paddr, u64 size)
{
        int cap, r;
        struct pmobject *pmo;

        BUG_ON(size == 0);
        pmo = obj_alloc(TYPE_PMO, sizeof(*pmo));
        if (!pmo) {
                r = -ENOMEM;
                goto out_fail;
        }
        pmo_init(pmo, PMO_DEVICE, size, paddr);
        cap = cap_alloc(current_cap_group, pmo, 0);
        if (cap < 0) {
                r = cap;
                goto out_free_obj;
        }

        return cap;
out_free_obj:
        obj_free(pmo);
out_fail:
        return r;
}

int create_pmo(u64 size, u64 type, struct cap_group *cap_group,
               struct pmobject **new_pmo)
{
        int cap, r;
        struct pmobject *pmo;

        pmo = obj_alloc(TYPE_PMO, sizeof(*pmo));
        if (!pmo) {
                r = -ENOMEM;
                goto out_fail;
        }

        r = pmo_init(pmo, type, size, 0);
        if (r)
                goto out_free_obj;

        cap = cap_alloc(cap_group, pmo, 0);
        if (cap < 0) {
                r = cap;
                goto out_free_obj;
        }

        if (new_pmo != NULL)
                *new_pmo = pmo;

        return cap;

out_free_obj:
        obj_free(pmo);
out_fail:
        return r;
}

int sys_create_pmo(u64 size, u64 type)
{
        BUG_ON(size == 0);
        return create_pmo(size, type, current_cap_group, NULL);
}

struct pmo_request {
        /* args */
        u64 size;
        u64 type;
        /* return value */
        u64 ret_cap;
};

#define MAX_CNT 32

int sys_create_pmos(u64 user_buf, u64 cnt)
{
        u64 size;
        struct pmo_request *requests;
        int i;
        int cap;

        /* in case of integer overflow */
        if (cnt > MAX_CNT) {
                kwarn("create too many pmos for one time (max: %d)\n", MAX_CNT);
                return -EINVAL;
        }

        size = sizeof(*requests) * cnt;
        requests = (struct pmo_request *)kmalloc(size);
        if (requests == NULL) {
                kwarn("cannot allocate more memory\n");
                return -EAGAIN;
        }
        copy_from_user((char *)requests, (char *)user_buf, size);

        for (i = 0; i < cnt; ++i) {
                cap = sys_create_pmo(requests[i].size, requests[i].type);
                requests[i].ret_cap = cap;
        }

        /* return pmo_caps */
        copy_to_user((char *)user_buf, (char *)requests, size);

        /* free temporary buffer */
        kfree(requests);

        return 0;
}

#define WRITE 0
#define READ  1
static int read_write_pmo(u64 pmo_cap, u64 offset, u64 user_buf, u64 size,
                          u64 op_type)
{
        struct pmobject *pmo;
        pmo_type_t pmo_type;
        vaddr_t kva;
        int r;

        r = 0;
        /* Function caller should have the pmo_cap */
        pmo = obj_get(current_cap_group, pmo_cap, TYPE_PMO);
        if (!pmo) {
                r = -ECAPBILITY;
                goto out_fail;
        }

        /* Only PMO_DATA or PMO_ANONYM is allowed with this inferface. */
        pmo_type = pmo->type;
        if ((pmo_type != PMO_DATA) && (pmo_type != PMO_DATA_NOCACHE)
            && (pmo_type != PMO_ANONYM)) {
                r = -EINVAL;
                goto out_obj_put;
        }

        /* Only READ and WRITE operations are allowed. */
        if (op_type != READ && op_type != WRITE) {
                r = -EINVAL;
                goto out_obj_put;
        }

        /* Range check */
        if (offset + size < offset || offset + size > pmo->size) {
                r = -EINVAL;
                goto out_obj_put;
        }

        if (pmo_type == PMO_DATA || pmo_type == PMO_DATA_NOCACHE) {
                kva = phys_to_virt(pmo->start) + offset;
                if (op_type == WRITE)
                        r = copy_from_user((char *)kva, (char *)user_buf, size);
                else // op_type == READ
                        r = copy_to_user((char *)user_buf, (char *)kva, size);
        } else {
                /* PMO_ANONYM */
                u64 index;
                u64 pa;
                u64 to_read_write;
                u64 offset_in_page;

                while (size > 0) {
                        index = ROUND_DOWN(offset, PAGE_SIZE) / PAGE_SIZE;
                        pa = get_page_from_pmo(pmo, index);
                        if (pa == 0) {
                                /* Allocate a physical page for the anonymous
                                 * pmo like a page fault happens.
                                 */
                                kva = (vaddr_t)get_pages(0);
                                BUG_ON(kva == 0);

                                pa = virt_to_phys((void *)kva);
                                memset((void *)kva, 0, PAGE_SIZE);
                                commit_page_to_pmo(pmo, index, pa);

                                /* No need to map the physical page in the page
                                 * table of current process because it uses
                                 * write/read_pmo which means it does not need
                                 * the mappings.
                                 */
                        } else {
                                kva = phys_to_virt(pa);
                        }
                        /* Now kva is the beginning of some page, we should add
                         * the offset inside the page. */
                        offset_in_page = offset - ROUND_DOWN(offset, PAGE_SIZE);
                        kva += offset_in_page;
                        to_read_write = MIN(PAGE_SIZE - offset_in_page, size);

                        if (op_type == WRITE)
                                r = copy_from_user((char *)kva,
                                                   (char *)user_buf,
                                                   to_read_write);
                        else // op_type == READ
                                r = copy_to_user((char *)user_buf,
                                                 (char *)kva,
                                                 to_read_write);

                        offset += to_read_write;
                        size -= to_read_write;
                }
        }

out_obj_put:
        obj_put(pmo);
out_fail:
        return r;
}

/*
 * A process can send a PMO (with msgs) to others.
 * It can write the msgs without mapping the PMO with this function.
 */
int sys_write_pmo(u64 pmo_cap, u64 offset, u64 user_ptr, u64 len)
{
        return read_write_pmo(pmo_cap, offset, user_ptr, len, WRITE);
}

int sys_read_pmo(u64 pmo_cap, u64 offset, u64 user_ptr, u64 len)
{
        return read_write_pmo(pmo_cap, offset, user_ptr, len, READ);
}

/**
 * Given a pmo_cap, return its corresponding start physical address.
 */
int sys_get_pmo_paddr(u64 pmo_cap, u64 user_buf)
{
        struct pmobject *pmo;
        int r = 0;

        /* Caller should have the pmo_cap */
        pmo = obj_get(current_cap_group, pmo_cap, TYPE_PMO);
        if (!pmo) {
                r = -ECAPBILITY;
                goto out_fail;
        }

        /* Only allow to get the address of PMO_DATA for now */
        if (pmo->type != PMO_DATA && pmo->type != PMO_DATA_NOCACHE) {
                r = -EINVAL;
                goto out_obj_put;
        }

        copy_to_user((char *)user_buf, (char *)&pmo->start, sizeof(u64));

out_obj_put:
        obj_put(pmo);
out_fail:
        return r;
}

/*
 * Given a virtual address, return its corresponding physical address.
 * Notice: the virtual address should be page-backed, thus pre-fault could be
 * conducted before using this syscall.
 */
int sys_get_phys_addr(u64 va, u64 *pa_buf)
{
        struct vmspace *vmspace = current_thread->vmspace;
        paddr_t pa;
        int ret;

        extern int query_in_pgtbl(void *, vaddr_t, paddr_t *, void **);
        ret = query_in_pgtbl(vmspace->pgtbl, va, &pa, NULL);

        if (ret < 0)
                return ret;

        copy_to_user((char *)pa_buf, (char *)&pa, sizeof(u64));

        return 0;
}

int trans_uva_to_kva(u64 user_va, u64 *kernel_va)
{
        struct vmspace *vmspace = current_thread->vmspace;
        paddr_t pa;
        int ret;

        extern int query_in_pgtbl(void *, vaddr_t, paddr_t *, void **);
        ret = query_in_pgtbl(vmspace->pgtbl, user_va, &pa, NULL);

        if (ret < 0)
                return ret;

        *kernel_va = phys_to_virt(pa);
        return 0;
}

/*
 * A process can not only map a PMO into its private address space,
 * but also can map a PMO to some others (e.g., load code for others).
 */
int sys_map_pmo(u64 target_cap_group_cap, u64 pmo_cap, u64 addr, u64 perm,
                u64 len)
{
        struct vmspace *vmspace;
        struct pmobject *pmo;
        struct cap_group *target_cap_group;
        int r;

        pmo = obj_get(current_cap_group, pmo_cap, TYPE_PMO);
        if (!pmo) {
                r = -ECAPBILITY;
                goto out_fail;
        }

        /* translate default length (-1) to pmo_size */
        if (likely(len == -1))
                len = pmo->size;

        /* map the pmo to the target cap_group */
        target_cap_group = obj_get(
                current_cap_group, target_cap_group_cap, TYPE_CAP_GROUP);
        if (!target_cap_group) {
                r = -ECAPBILITY;
                goto out_obj_put_pmo;
        }
        vmspace = obj_get(target_cap_group, VMSPACE_OBJ_ID, TYPE_VMSPACE);
        BUG_ON(vmspace == NULL);

        r = vmspace_map_range(vmspace, addr, len, perm, pmo);
        if (r != 0) {
                r = -EPERM;
                goto out_obj_put_vmspace;
        }

        /*
         * when a process maps a pmo to others,
         * this func returns the new_cap in the target process.
         */
        if (target_cap_group != current_cap_group)
                /* if using cap_move, we need to consider remove the mappings */
                r = cap_copy(current_cap_group, target_cap_group, pmo_cap);
        else
                r = 0;

out_obj_put_vmspace:
        obj_put(vmspace);
        obj_put(target_cap_group);
out_obj_put_pmo:
        obj_put(pmo);
out_fail:
        return r;
}

/* Example usage: Used in ipc/connection.c for mapping ipc_shm */
int map_pmo_in_current_cap_group(u64 pmo_cap, u64 addr, u64 perm)
{
        struct vmspace *vmspace;
        struct pmobject *pmo;
        int r;

        pmo = obj_get(current_cap_group, pmo_cap, TYPE_PMO);
        if (!pmo) {
                kinfo("map fails: invalid pmo (cap is %lu)\n", pmo_cap);
                r = -ECAPBILITY;
                goto out_fail;
        }

        vmspace = obj_get(current_cap_group, VMSPACE_OBJ_ID, TYPE_VMSPACE);
        BUG_ON(vmspace == NULL);
        r = vmspace_map_range(vmspace, addr, pmo->size, perm, pmo);
        if (r != 0) {
                kinfo("%s failed: addr 0x%lx, pmo->size 0x%lx\n",
                      __func__,
                      addr,
                      pmo->size);
                r = -EPERM;
                goto out_obj_put_vmspace;
        }

out_obj_put_vmspace:
        obj_put(vmspace);
        obj_put(pmo);
out_fail:
        return r;
}

struct pmo_map_request {
        /* args */
        u64 pmo_cap;
        u64 addr;
        u64 perm;
        /*
         * If you want to free the pmo cap in current cap goup
         * after the pmo was mapped to the vmsapce of another
         * process, please set free_cap to 1.
         */
        u64 free_cap;

        /* return caps or return value */
        u64 ret;
};

int sys_map_pmos(u64 target_cap_group_cap, u64 user_buf, u64 cnt)
{
        u64 size;
        struct pmo_map_request *requests;
        struct vmspace *vmspace;
        struct pmobject *pmo;
        int i;
        int map_ret, ret = 0;

        /* in case of integer overflow */
        if (cnt > MAX_CNT) {
                kwarn("create too many pmos for one time (max: %d)\n", MAX_CNT);
                return -EINVAL;
        }

        size = sizeof(*requests) * cnt;
        requests = (struct pmo_map_request *)kmalloc(size);
        if (requests == NULL) {
                kwarn("cannot allocate more memory\n");
                return -EAGAIN;
        }
        copy_from_user((char *)requests, (char *)user_buf, size);

        for (i = 0; i < cnt; ++i) {
                /*
                 * if target_cap_group is not current_cap_group,
                 * ret is cap on success.
                 */
                map_ret = sys_map_pmo(target_cap_group_cap,
                                      requests[i].pmo_cap,
                                      requests[i].addr,
                                      requests[i].perm,
                                      -1 /* pmo size */);
                requests[i].ret = map_ret;

                /*
                 * One failure will not abort the following request.
                 * Leave user space to handle partial failure.
                 */
                if (map_ret < 0)
                        ret = -EINVAL;

                if (map_ret >= 0 && requests[i].free_cap == 1) {
                        pmo = obj_get(current_cap_group,
                                      requests[i].pmo_cap,
                                      TYPE_PMO);
                        vmspace = obj_get(current_cap_group,
                                          VMSPACE_OBJ_ID,
                                          TYPE_VMSPACE);
                        BUG_ON(pmo == NULL || vmspace == NULL);
                        /*
                         * If the pmo being freed is mapped to a
                         * vmregion in current vmspace, we need
                         * to remove the mapping.
                         */
                        unmap_pmo_in_vmspace(vmspace, pmo);

                        cap_free(current_cap_group, requests[i].pmo_cap);
                        obj_put(vmspace);
                        obj_put(pmo);
                }
        }

        copy_to_user((char *)user_buf, (char *)requests, size);

        kfree(requests);
        return ret;
}

int sys_unmap_pmo(u64 target_cap_group_cap, u64 pmo_cap, u64 addr)
{
        struct vmspace *vmspace;
        struct pmobject *pmo;
        struct cap_group *target_cap_group;
        int ret;

        /* caller should have the pmo_cap */
        pmo = obj_get(current_cap_group, pmo_cap, TYPE_PMO);
        if (!pmo)
                return -EPERM;

        /* map the pmo to the target cap_group */
        target_cap_group = obj_get(
                current_cap_group, target_cap_group_cap, TYPE_CAP_GROUP);
        if (!target_cap_group) {
                ret = -EPERM;
                goto fail1;
        }

        vmspace = obj_get(target_cap_group, VMSPACE_OBJ_ID, TYPE_VMSPACE);
        if (!vmspace) {
                ret = -EPERM;
                goto fail2;
        }

        ret = vmspace_unmap_range(vmspace, addr, pmo->size);
        if (ret != 0)
                ret = -EPERM;

        obj_put(vmspace);
fail2:
        obj_put(target_cap_group);
fail1:
        obj_put(pmo);

        return ret;
}

/*
 * Initialize an allocated pmobject.
 * @paddr is only used when @type == PMO_DEVICE.
 */
int pmo_init(struct pmobject *pmo, pmo_type_t type, size_t len, paddr_t paddr)
{
        memset((void *)pmo, 0, sizeof(*pmo));

        len = ROUND_UP(len, PAGE_SIZE);
        pmo->size = len;
        pmo->type = type;

        switch (type) {
        case PMO_DATA:
        case PMO_DATA_NOCACHE: {
                /*
                 * For PMO_DATA, the user will use it soon (we expect).
                 * So, we directly allocate the physical memory.
                 * Note that kmalloc(>2048) returns continous physical pages.
                 */
                pmo->start = (paddr_t)virt_to_phys(kmalloc(len));
                break;
        }
        case PMO_ANONYM:
        case PMO_SHM: {
                /*
                 * For PMO_ANONYM (e.g., stack and heap) or PMO_SHM,
                 * we do not allocate the physical memory at once.
                 */
                pmo->radix = new_radix();
                init_radix(pmo->radix);
                break;
        }
        case PMO_DEVICE: {
                /*
                 * For device memory (e.g., for DMA).
                 * We must ensure the range [paddr, paddr+len) is not
                 * in the main memory region.
                 */
                pmo->start = paddr;
                break;
        }
        case PMO_FORBID: {
                /* This type marks the corresponding area cannot be accessed */
                break;
        }
        default: {
                kinfo("Unsupported pmo type: %d\n", type);
                BUG_ON(1);
                break;
        }
        }
        return 0;
}

/* Record the physical page allocated to a pmo */
void commit_page_to_pmo(struct pmobject *pmo, u64 index, paddr_t pa)
{
        int ret;

        BUG_ON((pmo->type != PMO_ANONYM) && (pmo->type != PMO_SHM));
        /* The radix interfaces are thread-safe */
        ret = radix_add(pmo->radix, index, (void *)pa);
        BUG_ON(ret != 0);
}

/* Return 0 (NULL) when not found */
paddr_t get_page_from_pmo(struct pmobject *pmo, u64 index)
{
        paddr_t pa;

        /* The radix interfaces are thread-safe */
        pa = (paddr_t)radix_get(pmo->radix, index);
        return pa;
}

static void __free_pmo_page(void *addr)
{
        kfree((void *)phys_to_virt(addr));
}

void pmo_deinit(void *pmo_ptr)
{
        struct pmobject *pmo;
        pmo_type_t type;

        pmo = (struct pmobject *)pmo_ptr;
        type = pmo->type;

        switch (type) {
        case PMO_DATA:
        case PMO_DATA_NOCACHE: {
                paddr_t start_addr;

                /* PMO_DATA contains continous physical pages */
                start_addr = pmo->start;
                kfree((void *)phys_to_virt(start_addr));

                break;
        }
        case PMO_ANONYM:
        case PMO_SHM: {
                struct radix *radix;

                radix = pmo->radix;
                BUG_ON(radix == NULL);
                /*
                 * Set value_deleter to free each memory page during
                 * traversing the radix tree in radix_free.
                 */
                radix->value_deleter = __free_pmo_page;
                radix_free(radix);

                break;
        }
        case PMO_DEVICE:
        case PMO_FORBID: {
                break;
        }
        default: {
                kinfo("Unsupported pmo type: %d\n", type);
                BUG_ON(1);
                break;
        }
        }

        /* The pmo struct itself will be free in __free_object */
}

/*
 * User process heap start: 0x600000000000 (i.e., HEAP_START)
 *
 * defined in mm/vmregion.c
 */
u64 sys_handle_brk(u64 addr)
{
        struct vmspace *vmspace;
        struct pmobject *pmo;
        struct vmregion *vmr;
        size_t len;
        u64 retval;
        int ret;

        vmspace = obj_get(current_cap_group, VMSPACE_OBJ_ID, TYPE_VMSPACE);
        if (addr == 0) {
                /*
                 * Assumation: user/libc invokes brk(0) first.
                 *
                 * return the current heap address
                 */
                retval = vmspace->user_current_heap;

                /* create the heap pmo for the user process */
                len = 0;
                ret = create_pmo(len, PMO_ANONYM, current_cap_group, &pmo);

                if (ret < 0) {
                        kinfo("Fail: cannot create the initial heap pmo.\n");
                        /* Maybe OOM */
                        BUG_ON(1);
                }

                /* setup the vmr for the heap region */
                vmspace->heap_vmr = init_heap_vmr(vmspace, retval, pmo);
        } else {
                vmr = vmspace->heap_vmr;
                /* enlarge the heap vmr and pmo */
                if (addr >= (vmr->start + vmr->size)) {
                        /* add length */
                        len = addr - (vmr->start + vmr->size);
                        vmr->size += len;
                        vmr->pmo->size += len;
                } else {
                        kinfo("Buggy: why shrinking the heap?\n");
                        BUG_ON(1);
                }

                retval = addr;
        }

        /*
         * return origin heap addr on failure;
         * return new heap addr on success.
         */
        obj_put(vmspace);
        return retval;
}

/* A process mmap region start:  MMAP_START (defined in mm/vmregion.c) */
static vmr_prop_t get_vmr_prot(int prot)
{
        vmr_prop_t ret;

        ret = 0;
        if (prot & PROT_READ)
                ret |= VMR_READ;
        if (prot & PROT_WRITE)
                ret |= VMR_WRITE;
        if (prot & PROT_EXEC)
                ret |= VMR_EXEC;

        return ret;
}

extern u64 vmspace_mmap_with_pmo(struct vmspace *vmspace, struct pmobject *pmo,
                                 size_t len, vmr_prop_t perm);
u64 sys_handle_mmap(u64 addr, size_t length, int prot, int flags, int fd,
                    u64 offset)
{
        struct vmspace *vmspace;
        struct pmobject *pmo;
        vmr_prop_t vmr_prot;
        u64 map_addr;
        int new_pmo_cap;

        /* Currently, mmap must takes @fd with -1 (i.e., anonymous mapping) */
        if (fd != -1) {
                kwarn("%s: mmap only supports anonymous mapping with fd -1, but arg fd is %d\n",
                      __func__,
                      fd);
                BUG("");
                goto err_exit;
        }

        /* Check @prot */
        if (prot & PROT_CHECK_MASK) {
                kwarn("%s: mmap cannot support PROT: %d\n", __func__, prot);
                goto err_exit;
        }

        /* Check @flags */
        if (flags != (MAP_ANONYMOUS | MAP_PRIVATE)) {
                kwarn("%s: mmap only supports anonymous and private mapping\n",
                      __func__);
                goto err_exit;
        }

        /* Add one anonymous mapping, i.e., the cap in new_pmo_cap */

        /* Round up @length */
        if (length % PAGE_SIZE) {
                // kwarn("%s: mmap length should align to PAGE_SIZE\n",
                // __func__);
                length = ROUND_UP(length, PAGE_SIZE);
        }

        /* Create the pmo for the mmap area */
        new_pmo_cap = create_pmo(length, PMO_ANONYM, current_cap_group, &pmo);
        if (new_pmo_cap < 0) {
                kinfo("Fail: cannot create the new pmo for mmap\n");
                BUG_ON(1);
        }

        /* Change the vmspace */
        vmspace = obj_get(current_cap_group, VMSPACE_OBJ_ID, TYPE_VMSPACE);
        vmr_prot = get_vmr_prot(prot);
        map_addr = vmspace_mmap_with_pmo(vmspace, pmo, length, vmr_prot);
        obj_put(vmspace);

        kdebug("mmap done: map_addr is %p\n", map_addr);
        return map_addr;

err_exit:
        map_addr = (u64)(-1L);
        return map_addr;
}

/*
 * According to the POSIX specification:
 *
 * The munmap() function shall remove any mappings for those entire pages
 * containing any part of the address space of the process starting at addr and
 * continuing for len bytes.
 *
 * The behavior of this function is unspecified if the mapping was not
 * established by a call to mmap().
 *
 * The address addr  must be a multiple of the page size
 * (but length need not be).
 * All pages containing a part of the indicated range are unmapped,
 * and subsequent references to these pages will generate SIGSEGV.
 * It is not an error if the indicated range does not contain any mapped pages.
 *
 */
extern int vmspace_munmap_with_addr(struct vmspace *vmspace, vaddr_t addr,
                                    size_t len);
int sys_handle_munmap(u64 addr, size_t length)
{
        struct vmspace *vmspace;
        int ret;

        /* What if the len is not aligned */
        if ((addr % PAGE_SIZE) || (length % PAGE_SIZE)) {
                return -EINVAL;
        }

        vmspace = obj_get(current_cap_group, VMSPACE_OBJ_ID, TYPE_VMSPACE);
        ret = vmspace_munmap_with_addr(vmspace, (vaddr_t)addr, length);
        obj_put(vmspace);

        return ret;
}

u64 sys_get_free_mem_size(void)
{
        return get_free_mem_size();
}
