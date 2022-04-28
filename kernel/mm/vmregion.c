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

#include <common/macro.h>
#include <common/util.h>
#include <common/list.h>
#include <common/errno.h>
#include <common/kprint.h>
#include <mm/vmspace.h>
#include <mm/kmalloc.h>
#include <mm/mm.h>
#include <arch/mmu.h>

/* Local functions */

static struct vmregion *alloc_vmregion(void)
{
        struct vmregion *vmr;

        vmr = kmalloc(sizeof(*vmr));
        return vmr;
}

static void free_vmregion(struct vmregion *vmr)
{
        kfree((void *)vmr);
}

/*
 * Returns 0 when no intersection detected.
 */
static int check_vmr_intersect(struct vmspace *vmspace,
                               struct vmregion *vmr_to_add)
{
        struct vmregion *vmr;
        vaddr_t new_start, start;
        vaddr_t new_end, end;

        if (unlikely(vmr_to_add->size == 0))
                return 0;

        new_start = vmr_to_add->start;
        new_end = new_start + vmr_to_add->size - 1;

        for_each_in_list (vmr, struct vmregion, node, &(vmspace->vmr_list)) {
                start = vmr->start;
                end = start + vmr->size;
                if (!((new_start >= end) || (new_end <= start))) {
                        kwarn("new_start: %p, new_ned: %p, start: %p, end: %p\n",
                              new_start,
                              new_end,
                              start,
                              end);
                        return 1;
                }
        }

        return 0;
}

static int is_vmr_in_vmspace(struct vmspace *vmspace, struct vmregion *vmr)
{
        struct vmregion *iter;

        for_each_in_list (iter, struct vmregion, node, &(vmspace->vmr_list)) {
                if (iter == vmr)
                        return 1;
        }
        return 0;
}

static int add_vmr_to_vmspace(struct vmspace *vmspace, struct vmregion *vmr)
{
        if (check_vmr_intersect(vmspace, vmr) != 0) {
                kwarn("Detecting: vmr overlap\n");
                BUG_ON(1);
                return -EINVAL;
        }

        list_add(&(vmr->node), &(vmspace->vmr_list));
        return 0;
}

static void del_vmr_from_vmspace(struct vmspace *vmspace, struct vmregion *vmr)
{
        if (is_vmr_in_vmspace(vmspace, vmr))
                list_del(&(vmr->node));
        free_vmregion(vmr);
}

static int fill_page_table(struct vmspace *vmspace, struct vmregion *vmr)
{
        size_t pm_size;
        paddr_t pa;
        vaddr_t va;
        int ret;

        pm_size = vmr->pmo->size;
        pa = vmr->pmo->start;
        va = vmr->start;

        ret = map_range_in_pgtbl(vmspace->pgtbl, va, pa, pm_size, vmr->perm);

        return ret;
}

static int unmap_vmrs(struct vmspace *vmspace, vaddr_t va, size_t len)
{
        struct vmregion *vmr;
        struct pmobject *pmo;

        if (len == 0)
                return 0;

        vmr = find_vmr_for_va(vmspace, va);
        if (vmr == NULL) {
                // kwarn("%s: no vmr found for the va 0x%lx, len 0x%lx.\n",
                // __func__, va, len); kwarn("TODO: we need to unmap any address
                // in [va, va+len)\n");
                return 0;
        }

        pmo = vmr->pmo;

        if (vmr->size > len) {
                kwarn("%s: shrink one vmr (va is 0x%lx).\n", __func__, va);
                if (va != vmr->start)
                        return 0;

                /*
                 * FIXME: only support **shrink** a vmr whose pmo is PMO_ANONYM.
                 * shrink: unmap the starting part of the vmr.
                 */
                BUG_ON(pmo->type != PMO_ANONYM);

                /* Modify the vmregion start and size */
                vmr->size -= len;
                vmr->start += len;

                unmap_range_in_pgtbl(vmspace->pgtbl, va, len);

                /* Flush TLBs */
                flush_tlbs(vmspace, va, len);

                return 0;
        }

        /* delete the vmr from the vmspace */
        del_vmr_from_vmspace(vmspace, vmr);

        /* Umap a whole vmr */
        unmap_range_in_pgtbl(vmspace->pgtbl, va, len);

        flush_tlbs(vmspace, va, len);

        va += vmr->size;
        len -= vmr->size;
        return unmap_vmrs(vmspace, va, len);
}

/* End of local functions */

/*
 * Tracing/debugging:
 * This function is for dumping the vmr_list after a thread crashes.
 */
void kprint_vmr(struct vmspace *vmspace)
{
        struct vmregion *vmr;
        vaddr_t start, end;

        for_each_in_list (vmr, struct vmregion, node, &(vmspace->vmr_list)) {
                start = vmr->start;
                end = start + vmr->size;
                kinfo("[vmregion] start=%p end=%p. vmr->pmo->type=%d\n",
                      start,
                      end,
                      vmr->pmo->type);
        }
}

struct vmregion *find_vmr_for_va(struct vmspace *vmspace, vaddr_t addr)
{
        struct vmregion *vmr;
        vaddr_t start, end;

        for_each_in_list (vmr, struct vmregion, node, &(vmspace->vmr_list)) {
                start = vmr->start;
                end = start + vmr->size;
                if (addr >= start && addr < end)
                        return vmr;
        }
        return NULL;
}

int vmspace_map_range(struct vmspace *vmspace, vaddr_t va, size_t len,
                      vmr_prop_t flags, struct pmobject *pmo)
{
        struct vmregion *vmr;
        int ret;

        /* Check whether the pmo type is supported */
        BUG_ON((pmo->type != PMO_DATA) && (pmo->type != PMO_DATA_NOCACHE)
               && (pmo->type != PMO_ANONYM) && (pmo->type != PMO_DEVICE)
               && (pmo->type != PMO_SHM) && (pmo->type != PMO_FORBID));

        /* Align a vmr to PAGE_SIZE */
        va = ROUND_DOWN(va, PAGE_SIZE);
        if (len < PAGE_SIZE)
                len = PAGE_SIZE;

        vmr = alloc_vmregion();
        if (!vmr) {
                ret = -ENOMEM;
                goto out_fail;
        }
        vmr->start = va;
        vmr->size = len;
        vmr->perm = flags;
        if (unlikely(pmo->type == PMO_DEVICE)) {
                vmr->perm |= VMR_DEVICE;
        } else if (unlikely(pmo->type == PMO_DATA_NOCACHE)) {
                vmr->perm |= VMR_NOCACHE;
        }

        /* Currently, one vmr has exactly one pmo */
        vmr->pmo = pmo;

        ret = add_vmr_to_vmspace(vmspace, vmr);

        if (ret < 0) {
                kwarn("add_vmr_to_vmspace fails\n");
                goto out_free_vmr;
        }

        /*
         * Case-1:
         * If the pmo type is PMO_DATA or PMO_DEVICE, we directly add mappings
         * in the page table because the corresponding physical pages are
         * prepared. In this case, early mapping avoids page faults and brings
         * better performance.
         *
         * Case-2:
         * Otherwise (for PMO_ANONYM and PMO_SHM), we use on-demand mapping.
         * In this case, lazy mapping reduces the usage of physical memory
         * resource.
         */
        if ((pmo->type == PMO_DATA) || (pmo->type == PMO_DATA_NOCACHE)
            || (pmo->type == PMO_DEVICE))
                fill_page_table(vmspace, vmr);

        /* On success */
        return 0;
out_free_vmr:
        free_vmregion(vmr);
out_fail:
        return ret;
}

int vmspace_unmap_range(struct vmspace *vmspace, vaddr_t va, size_t len)
{
        struct vmregion *vmr;
        vaddr_t start;
        size_t size;
        struct pmobject *pmo;
        int ret;

        vmr = find_vmr_for_va(vmspace, va);
        if (!vmr) {
                kwarn("unmap a non-exist vmr.\n");
                ret = -1;
                goto out;
        }
        start = vmr->start;
        size = vmr->size;

        /* Sanity check: unmap the whole vmr */
        if (((va != start) || (len != size)) && (len != 0)) {
                kdebug("va: %p, start: %p, len: %p, size: %p\n",
                       va,
                       start,
                       len,
                       size);
                kwarn("Only support unmapping a whole vmregion now.\n");
                BUG_ON(1);
        }

        del_vmr_from_vmspace(vmspace, vmr);

        pmo = vmr->pmo;
        /* No pmo is mapped */
        if (pmo == NULL) {
                ret = 0;
                goto out;
        }

        /*
         * Remove the mappings in the page table.
         * When the pmo-type is DATA/DEVICE, each mapping must exist.
         *
         * Otherwise, the mapping is added on demand, which may not exist.
         * However, simply clearing non-present ptes is OK.
         */

        if (likely(len != 0)) {
                unmap_range_in_pgtbl(vmspace->pgtbl, va, len);

                flush_tlbs(vmspace, va, len);
        }

        /*
         * Now, we defer the free of physical pages in the PMO
         * to the recycle procedure of a process.
         */

        ret = 0;
out:
        return ret;
}

/*
 * Remove the mapping if a vmregion in the given vmspace points to the pmo.
 * If a process wants to map pmos to another process`s vmspace and
 * free these pmo_caps in its own cap group. It may use this function to
 * remove the mappings in its own vmspace
 */
int unmap_pmo_in_vmspace(struct vmspace *vmspace, struct pmobject *pmo)
{
        int ret;
        struct vmregion *iter_vmr;
        struct vmregion *vmr = NULL;

        u64 flush_va_start;
        u64 flush_len;

        /* Find the corresponding vmr of the given pmo */
        for_each_in_list (
                iter_vmr, struct vmregion, node, &(vmspace->vmr_list)) {
                if (iter_vmr->pmo == pmo) {
                        vmr = iter_vmr;
                        break;
                }
        }

        if (!vmr) {
                ret = -ENOENT;
                goto out;
        }

        flush_va_start = vmr->start;
        flush_len = vmr->size;
        /* Remove the vmr from the given vmspace */
        del_vmr_from_vmspace(vmspace, vmr);

        /* Remove the mapping in page table */
        unmap_range_in_pgtbl(vmspace->pgtbl, flush_va_start, flush_len);

        flush_tlbs(vmspace, flush_va_start, flush_len);

        return 0;
out:
        return ret;
}

/*
 * The heap region of each process starts at HEAP_START and can at most grow
 * to (MMAP_START-1). (up to 16 TB)
 *
 * The mmap region of each process starts at MMAP_START and can at most grow
 * to USER_SPACE_END. (up to 16 TB)
 *
 * For x86_64:
 * In 64-bit mode, an address is considered to be in canonical form
 * if address bits 63 through to the most-significant implemented bit
 * by the microarchitecture are set to either all ones or all zeros.
 * The kernel and user share the 48-bit address (0~2^48-1).
 * As usual, we let the kernel use the top half and the user use the
 * bottom half.
 * So, the user address is 0 ~ 2^47-1 (USER_SPACE_END).
 *
 *
 * For aarch64:
 * With 4-level page tables:
 * TTBR0_EL1 translates 0 ~ 2^48-1 .
 * TTBR1_EL1 translates (0xFFFF) + 0 ~ 2^48-1.
 * But, we only use 0 ~ USER_SPACE_END (as x86_64).
 * With 3-level page tables:
 * The same as x86_64.
 */

#define HEAP_START     (0x600000000000UL)
#define MMAP_START     (0x700000000000UL)
#define USER_SPACE_END (0x800000000000UL)

/* Each process has a vmr specially for heap */
struct vmregion *init_heap_vmr(struct vmspace *vmspace, vaddr_t va,
                               struct pmobject *pmo)
{
        struct vmregion *vmr;
        int ret;

        vmr = alloc_vmregion();
        if (!vmr) {
                kwarn("%s fails\n", __func__);
                goto out_fail;
        }
        vmr->start = va;
        vmr->size = 0;
        vmr->perm = VMR_READ | VMR_WRITE;
        vmr->pmo = pmo;

        ret = add_vmr_to_vmspace(vmspace, vmr);

        if (ret < 0)
                goto out_free_vmr;

        return vmr;

out_free_vmr:
        free_vmregion(vmr);
out_fail:
        return NULL;
}

u64 vmspace_mmap_with_pmo(struct vmspace *vmspace, struct pmobject *pmo,
                          size_t len, vmr_prop_t perm)
{
        struct vmregion *vmr;
        int ret;

        vmr = alloc_vmregion();
        if (!vmr) {
                kwarn("%s fails\n", __func__);
                goto out_fail;
        }

        vmr->start = vmspace->user_current_mmap_addr;

        BUG_ON(len % PAGE_SIZE);
        /* for simplicity, just keep increasing the mmap_addr now */
        vmspace->user_current_mmap_addr += len;

        vmr->size = len;
        vmr->perm = perm;

        /*
         * Currently, we restrict the pmo types, which must be
         * pmo_anonym or pmo_shm.
         */
        BUG_ON((pmo->type != PMO_ANONYM) && (pmo->type != PMO_SHM));

        vmr->pmo = pmo;

        ret = add_vmr_to_vmspace(vmspace, vmr);

        if (ret < 0)
                goto out_free_vmr;

        return vmr->start;

out_free_vmr:
        free_vmregion(vmr);
out_fail:
        return (u64)-1L;
}

int vmspace_munmap_with_addr(struct vmspace *vmspace, vaddr_t va, size_t len)
{
        return unmap_vmrs(vmspace, va, len);
}

int vmspace_unmap_shm_vmr(struct vmspace *vmspace, vaddr_t va)
{
        struct vmregion *vmr;
        struct pmobject *pmo;

        u64 flush_va_start;
        u64 flush_len;

        vmr = find_vmr_for_va(vmspace, va);
        if (vmr == NULL) {
                kwarn("%s: no vmr found for the va 0x%lx.\n", __func__, va);
                goto fail_out;
        }

        pmo = vmr->pmo;

        /* Sanity check */
        /* check-1: this interface is only used for shmdt */
        BUG_ON(pmo->type != PMO_SHM);
        /* check-2: the va should be the start address of the shm */
        BUG_ON(va != vmr->start);

        /* Umap a whole vmr */
        unmap_range_in_pgtbl(vmspace->pgtbl, vmr->start, vmr->size);
        flush_va_start = vmr->start;
        flush_len = vmr->size;

        /*
         * Physical resources free should be done when the shm object is
         * removed by shmctl.
         */

        /* Delete the vmr from the vmspace */
        del_vmr_from_vmspace(vmspace, vmr);

        /* Flush TLBs without holding locks */
        flush_tlbs(vmspace, flush_va_start, flush_len);

        return 0;

fail_out:
        return -EINVAL;
}

extern void arch_vmspace_init(struct vmspace *);

int vmspace_init(struct vmspace *vmspace)
{
        init_list_head(&vmspace->vmr_list);
        /* Allocate the root page table page */
        vmspace->pgtbl = get_pages(0);
        BUG_ON(vmspace->pgtbl == NULL);
        memset(vmspace->pgtbl, 0, PAGE_SIZE);

        /* Architecture-dependent initilization */
        arch_vmspace_init(vmspace);

        vmspace->user_current_heap = HEAP_START;
        vmspace->user_current_mmap_addr = MMAP_START;

        return 0;
}

void vmspace_deinit(void *ptr)
{
        struct vmspace *vmspace;
        struct vmregion *vmr;
        struct vmregion *tmp;

        vmspace = (struct vmspace *)ptr;

        /*
         * Free each vmregion in vmspace->vmr_list.
         * Only invoked when a process exits.
         */
        for_each_in_list_safe (vmr, tmp, node, &(vmspace->vmr_list)) {
                free_vmregion(vmr);
        }

        extern void free_page_table(void *);
        free_page_table(vmspace->pgtbl);

        // extern void flush_tlb_of_vmspace(struct vmspace *);
        // flush_tlb_of_vmspace(vmspace);
}
