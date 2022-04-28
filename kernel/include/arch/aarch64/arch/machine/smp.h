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
