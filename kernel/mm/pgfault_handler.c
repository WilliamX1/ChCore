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
#include <common/types.h>
#include <lib/printk.h>
#include <mm/vmspace.h>
#include <mm/kmalloc.h>
#include <mm/mm.h>
#include <mm/vmspace.h>
#include <arch/mmu.h>
#include <object/thread.h>
#include <object/cap_group.h>
#include <sched/context.h>


// #define ENABLE_MEMORY_PAGE_SWITCH

#ifdef ENABLE_MEMORY_PAGE_SWITCH

#define simulate_page_order 1
#define simulate_page_num (1 << simulate_page_order)

static paddr_t simulate_pa;

static bool is_mapped[simulate_page_num];
static vaddr_t origin_va[simulate_page_num];
static paddr_t origin_pa[simulate_page_num];
static vmr_prop_t origin_perm[simulate_page_num];

static u64 access_time[simulate_page_num];
static u64 current_time = 0;

static void init_memory_page(void) {
        static bool has_init = false;
        if (!has_init) {
                has_init = true;
                simulate_pa = virt_to_phys(get_pages(simulate_page_order));
        }
}

static void alloc_page_in_memory(void *pgtbl, vaddr_t va, paddr_t pa, vmr_prop_t perm) {
        u64 min_time = -1;
        u64 alloc_index;

        for (u64 i = 0; i < simulate_page_num; ++i) {
                if (is_mapped[i]) {
                        if (min_time > access_time[i]) {
                                min_time = access_time[i];
                                alloc_index = i;
                        }
                }
                else {
                        alloc_index = i;
                        break;
                }
        }

        if (is_mapped[alloc_index]) {
                // swap out
                unmap_range_in_pgtbl(pgtbl, origin_va[alloc_index], PAGE_SIZE);
                memcpy(phys_to_virt(origin_pa[alloc_index]), phys_to_virt(simulate_pa + alloc_index * PAGE_SIZE), PAGE_SIZE);
                map_range_in_pgtbl(pgtbl, origin_va[alloc_index], origin_pa[alloc_index], PAGE_SIZE, origin_perm[alloc_index]);
        }
        
        // swap in
        is_mapped[alloc_index] = true;
        origin_pa[alloc_index] = pa;
        origin_va[alloc_index] = va;
        origin_perm[alloc_index] = perm;

        access_time[alloc_index] = current_time++;

        memcpy(phys_to_virt(simulate_pa + alloc_index * PAGE_SIZE), phys_to_virt(pa), PAGE_SIZE);
        map_range_in_pgtbl(pgtbl, va, simulate_pa + alloc_index * PAGE_SIZE, PAGE_SIZE, perm);
}

#endif


int handle_trans_fault(struct vmspace *vmspace, vaddr_t fault_addr)
{
        struct vmregion *vmr;
        struct pmobject *pmo;
        paddr_t pa;
        u64 offset;
        u64 index;
        int ret = 0;

        vmr = find_vmr_for_va(vmspace, fault_addr);
        if (vmr == NULL) {
                printk("handle_trans_fault: no vmr found for va 0x%lx!\n",
                       fault_addr);
                kinfo("process: %p\n", current_cap_group);
                print_thread(current_thread);
                kinfo("faulting IP: 0x%lx, SP: 0x%lx\n",
                      arch_get_thread_next_ip(current_thread),
                      arch_get_thread_stack(current_thread));

                kprint_vmr(vmspace);
                kwarn("TODO: kill such faulting process.\n");
                return -ENOMAPPING;
        }

#ifdef ENABLE_MEMORY_PAGE_SWITCH
        init_memory_page();
#endif

        pmo = vmr->pmo;
        switch (pmo->type) {
        case PMO_ANONYM:
        case PMO_SHM: {
                vmr_prop_t perm;

                perm = vmr->perm;

                /* Get the offset in the pmo for faulting addr */
                offset = ROUND_DOWN(fault_addr, PAGE_SIZE) - vmr->start;
                BUG_ON(offset >= pmo->size);

                /* Get the index in the pmo radix for faulting addr */
                index = offset / PAGE_SIZE;

                fault_addr = ROUND_DOWN(fault_addr, PAGE_SIZE);
                /* LAB 3 TODO BEGIN */

                pa = get_page_from_pmo(pmo, index);

                /* LAB 3 TODO END */
                if (pa == 0) {
                        /* Not committed before. Then, allocate the physical
                         * page. */
                        /* LAB 3 TODO BEGIN */

                        pa = virt_to_phys(get_pages(0));
                        memset(phys_to_virt(pa), 0, PAGE_SIZE);
                        commit_page_to_pmo(pmo, index, pa);

#ifdef ENABLE_MEMORY_PAGE_SWITCH
                        alloc_page_in_memory(vmspace->pgtbl, fault_addr, pa, perm);
#else
                        map_range_in_pgtbl(vmspace->pgtbl, fault_addr, pa, PAGE_SIZE, perm);
#endif

                        /* LAB 3 TODO END */
#ifdef CHCORE_LAB3_TEST
                        printk("Test: Test: Successfully map\n");
#endif
                } else {
                        /*
                         * pa != 0: the faulting address has be committed a
                         * physical page.
                         *
                         * For concurrent page faults:
                         *
                         * When type is PMO_ANONYM, the later faulting threads
                         * of the process do not need to modify the page
                         * table because a previous faulting thread will do
                         * that. (This is always true for the same process)
                         * However, if one process map an anonymous pmo for
                         * another process (e.g., main stack pmo), the faulting
                         * thread (e.g, in the new process) needs to update its
                         * page table.
                         * So, for simplicity, we just update the page table.
                         * Note that adding the same mapping is harmless.
                         *
                         * When type is PMO_SHM, the later faulting threads
                         * needs to add the mapping in the page table.
                         * Repeated mapping operations are harmless.
                         */
                        /* LAB 3 TODO BEGIN */
                        
#ifdef ENABLE_MEMORY_PAGE_SWITCH
                        alloc_page_in_memory(vmspace->pgtbl, fault_addr, pa, perm);
#else
                        map_range_in_pgtbl(vmspace->pgtbl, fault_addr, pa, PAGE_SIZE, perm);
#endif

                        /* LAB 3 TODO END */
#ifdef CHCORE_LAB3_TEST
                        printk("Test: Test: Successfully map for pa not 0\n");
#endif
                }

                /* Cortex A53 has VIPT I-cache which is inconsistent with
                 * dcache. */
#ifdef CHCORE_ARCH_AARCH64
                if (vmr->perm & VMR_EXEC) {
                        extern void arch_flush_cache(u64, s64, int);
                        /*
                         * Currently, we assume the fauling thread is running on
                         * the CPU. Thus, we flush cache by using user VA.
                         */
                        BUG_ON(current_thread->vmspace != vmspace);
                        /* 4 means flush idcache. */
                        arch_flush_cache(fault_addr, PAGE_SIZE, 4);
                }
#endif

                break;
        }
        case PMO_FORBID: {
                kinfo("Forbidden memory access (pmo->type is PMO_FORBID).\n");
                BUG_ON(1);
                break;
        }
        default: {
                kinfo("handle_trans_fault: faulting vmr->pmo->type"
                      "(pmo type %d at 0x%lx)\n",
                      vmr->pmo->type,
                      fault_addr);
                kinfo("Currently, this pmo type should not trigger pgfaults\n");
                kprint_vmr(vmspace);
                BUG_ON(1);
                return -ENOMAPPING;
        }
        }

        return ret;
}
