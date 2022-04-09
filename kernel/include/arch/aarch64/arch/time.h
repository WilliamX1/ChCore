#pragma once

#include <common/types.h>

static inline u64 get_cycles(void)
{
        u64 tsc;

        asm volatile("mrs %0, pmccntr_el0" : "=r"(tsc));
        return tsc;
}
