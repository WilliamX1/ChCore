#pragma once

#include <common/vars.h>
#include <common/types.h>
#include <arch/mmu.h>

#define PAGE_SIZE (0x1000)

void arch_mm_init(void);
void mm_init(void);
void set_page_table(paddr_t pgtbl);
