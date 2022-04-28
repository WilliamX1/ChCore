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

#include <common/types.h>
#include <mm/vmspace.h>
#include <mm/kmalloc.h>
#include <mm/mm.h>

/* In cache.c */
void cache_setup(void);

void arch_mm_init(void)
{
        cache_setup();
}

/*
 * ASID:
 * An ASID is a 16-bit identifer.
 * The current ASID is the value of bits 63:48 of TTBR0_EL1.
 * To prevent misunderstanding, we use 'pcid' here instead of 'asid'.
 *
 * ASID configuration is configured in el1_mmu_activate
 */
#define ASID_SHIFT 48

void arch_vmspace_init(struct vmspace *vmspace)
{
        /* In aarch64, this function is not needed. */
}

struct vmspace *create_idle_vmspace(void)
{
        struct vmspace *vmspace;

        vmspace = (struct vmspace *)kzalloc(sizeof(*vmspace));
        /*
         * An idle thread on aarch64 does not require a pgtbl
         * for user-space.
         */
        return vmspace;
}

/* Change vmspace to the target one */
void switch_vmspace_to(struct vmspace *vmspace)
{
        paddr_t pa;

        pa = virt_to_phys(vmspace->pgtbl);
        /* The upper 16 bits of TTBR0_EL1 represent ASID */
        pa |= (u64)(vmspace->pcid) << ASID_SHIFT;
        set_page_table(pa);
}
