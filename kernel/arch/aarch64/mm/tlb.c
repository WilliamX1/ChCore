#include <common/types.h>
#include <common/macro.h>
#include <arch/sync.h>

void flush_tlb_all(void)
{
        /* full system barrier */
        dsb(sy);
        asm volatile("tlbi vmalle1is\n\t" : : :);
        dsb(sy);
        isb();
}
