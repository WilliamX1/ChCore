#pragma once

#include <common/vars.h>
#include <machine.h>
#include <common/types.h>

enum cpu_state {
        cpu_hang = 0,
        cpu_run = 1,
        cpu_idle = 2,
};

extern volatile char cpu_status[PLAT_CPU_NUM];

void enable_smp_cores(paddr_t boot_flag);
u32 smp_get_cpu_id(void);
u64 smp_get_mpidr(void);
