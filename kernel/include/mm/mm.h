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
