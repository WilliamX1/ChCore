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

#include <common/lock.h>
#include <arch/machine/smp.h>
#include <common/macro.h>
#include <mm/kmalloc.h>

#include "barrier.h"

volatile u64 cpu_barrier[PLAT_CPU_NUM];

#define PRIMARY_CPU_ID 0

void global_barrier_init(void)
{
        int i;
        for (i = 0; i < PLAT_CPU_NUM; i++) {
                cpu_barrier[i] = 0;
        }
}

void global_barrier(void)
{
        if (smp_get_cpu_id() == 0)
                global_barrier_primary();
        else
                global_barrier_secondary();
}

void global_barrier_primary(void)
{
        int i;
        u32 cpu_id = smp_get_cpu_id();
        u64 barrier_num;

        BUG_ON(PRIMARY_CPU_ID != cpu_id);

        barrier_num = cpu_barrier[cpu_id];
        for (i = 0; i < PLAT_CPU_NUM; i++) {
                if (i == cpu_id)
                        continue;
                while (cpu_barrier[i] != barrier_num + 1)
                        ;
        }
        cpu_barrier[cpu_id] = barrier_num + 1;
        asm volatile("dsb sy");
}

void global_barrier_secondary(void)
{
        u32 cpu_id = smp_get_cpu_id();
        u64 barrier_num;

        barrier_num = cpu_barrier[cpu_id];
        cpu_barrier[cpu_id]++;
        while (cpu_barrier[PRIMARY_CPU_ID] != barrier_num + 1)
                ;
        asm volatile("dsb sy");
}
