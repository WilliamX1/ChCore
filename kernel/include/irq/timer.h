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

#include <common/types.h>
#include <common/list.h>
#include <common/lock.h>
#include <machine.h>
#include <posix/time.h>

struct thread;

#define NS_IN_S  (1000000000UL)
#define US_IN_S  (1000000UL)
#define NS_IN_US (1000UL)

extern u64 tick_per_us;

void timer_init(void);
void plat_timer_init(void);
void plat_set_next_timer(u64 tick_delta);
void handle_timer_irq(void);
void plat_handle_timer_irq(u64 tick_delta);

u64 plat_get_mono_time(void);
u64 plat_get_current_tick(void);

/* Syscalls */
int sys_clock_gettime(clockid_t clock, struct timespec *ts);
