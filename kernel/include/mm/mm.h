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

#include <common/vars.h>
#include <common/types.h>
#include <arch/mmu.h>
#include <mm/vmspace.h>

#define PAGE_SIZE (0x1000)

void arch_mm_init(void);
void mm_init(void);
void set_page_table(paddr_t pgtbl);
void flush_tlbs(struct vmspace*, u64, u64);

static inline bool is_user_addr(vaddr_t vaddr)
{
        return vaddr < KBASE;
}

static inline bool is_user_addr_range(vaddr_t vaddr, size_t len)
{
        return (vaddr + len >= vaddr) && is_user_addr(vaddr + len);
}
