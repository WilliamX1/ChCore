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

#include <common/vars.h>
#include <common/kprint.h>
#include <mm/mm.h>
#include <arch/machine/smp.h>
#include <common/types.h>
#include <arch/tools.h>
#include <irq/ipi.h>

volatile char cpu_status[PLAT_CPU_NUM] = {cpu_hang};

u64 ctr_el0;

void enable_smp_cores(paddr_t boot_flag)
{
        int i = 0;
        long *secondary_boot_flag;

        /* Set current cpu status */
        cpu_status[smp_get_cpu_id()] = cpu_run;
        secondary_boot_flag = (long *)phys_to_virt(boot_flag);
        for (i = 0; i < PLAT_CPU_NUM; i++) {
                /* Lab4
                 * You should set one flag to enable the APs to continue in
                 * _start. Then, what's the flag?
                 */
                /* LAB 4 TODO BEGIN */
		secondary_boot_flag[i] = 1;
                /* LAB 4 TODO END */

                flush_dcache_area((u64)secondary_boot_flag,
                                  (u64)sizeof(u64) * PLAT_CPU_NUM);
                asm volatile("dsb sy");

                /* Lab4
                 * The BSP waits for the currently initializing AP finishing
                 * before activating the next one
                 */
                /* LAB 4 TODO BEGIN */
		while (cpu_status[i] == cpu_hang);
                /* LAB 4 TODO END */
                if (cpu_status[i] == cpu_run)
                        kinfo("CPU %d is active\n", i);
                else
                        BUG("CPU %d not running!\n", i);
        }
        /* wait all cpu to boot */
        kinfo("All %d CPUs are active\n", PLAT_CPU_NUM);
        init_ipi_data();
}

inline u32 smp_get_cpu_id(void)
{
        u64 cpuid = 0;

        asm volatile("mrs %0, tpidr_el1" : "=r"(cpuid));
        return (u32)cpuid;
}

static inline u64 read_ctr()
{
        u64 reg;

        asm volatile("mrs %0, ctr_el0" : "=r"(reg)::"memory");
        return reg;
}

u64 smp_get_mpidr(void)
{
        u64 mpidr = 0;

        asm volatile("mrs %0, mpidr_el1" : "=r"(mpidr));
        return mpidr;
}
