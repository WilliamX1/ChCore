#pragma once

#include <common/types.h>

void enable_cpu_cnt(void);
void disable_cpu_cnt(void);
void pmu_init(void);

static inline u64 pmu_read_real_cycle(void)
{
        s64 tv;
        asm volatile("mrs %0, pmccntr_el0" : "=r"(tv));
        return tv;
}

static inline void pmu_clear_cnt(void)
{
        asm volatile("msr pmccntr_el0, %0" ::"r"(0));
}