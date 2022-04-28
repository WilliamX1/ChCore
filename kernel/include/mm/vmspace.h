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

#pragma once

#include <common/list.h>
#include <common/radix.h>
#include <arch/mmu.h>
#include <machine.h>

struct vmregion {
        struct list_head node; /* vmr_list */
        vaddr_t start;
        size_t size;
        vmr_prop_t perm;
        struct pmobject *pmo;
};

struct vmspace {
        /* List head of vmregion (vmr_list) */
        struct list_head vmr_list;
        /* Root page table */
        void *pgtbl;

        u64 pcid;

        /* Heap-related: only used for user processes */
        struct vmregion *heap_vmr;
        vaddr_t user_current_heap;

        /* For the virtual address of mmap */
        vaddr_t user_current_mmap_addr;
};

typedef u64 pmo_type_t;
#define PMO_ANONYM       0 /* lazy allocation */
#define PMO_DATA         1 /* immediate allocation */
#define PMO_SHM          3 /* shared memory */
#define PMO_DEVICE       5 /* memory mapped device registers */
#define PMO_DATA_NOCACHE 6 /* non-cacheable immediate allocation */

#define PMO_FORBID 10 /* Forbidden area: avoid overflow */

struct pmobject {
        struct radix *radix; /* record physical pages */
        paddr_t start;
        size_t size;
        pmo_type_t type;
};

struct cap_group;
int create_pmo(u64 size, u64 type, struct cap_group *cap_group,
               struct pmobject **new_pmo);

int vmspace_init(struct vmspace *vmspace);

int vmspace_map_range(struct vmspace *vmspace, vaddr_t va, size_t len,
                      vmr_prop_t flags, struct pmobject *pmo);
int vmspace_unmap_range(struct vmspace *vmspace, vaddr_t va, size_t len);
int unmap_pmo_in_vmspace(struct vmspace *vmspace, struct pmobject *pmo);

struct vmregion *find_vmr_for_va(struct vmspace *vmspace, vaddr_t addr);

void switch_vmspace_to(struct vmspace *);

void commit_page_to_pmo(struct pmobject *pmo, u64 index, paddr_t pa);
paddr_t get_page_from_pmo(struct pmobject *pmo, u64 index);

struct vmregion *init_heap_vmr(struct vmspace *vmspace, vaddr_t va,
                               struct pmobject *pmo);

void kprint_vmr(struct vmspace *vmspace);
