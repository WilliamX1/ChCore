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

#include <irq/irq.h>
#include <irq/timer.h>
#include <sched/sched.h>
#include <arch/machine/smp.h>
#include <object/thread.h>
#include <common/kprint.h>
#include <common/list.h>
#include <posix/time.h>
#include <mm/uaccess.h>
#include <sched/context.h>

/* Per-core timer states */
struct time_state {
        /* The tick when the next timer irq will occur */
        u64 next_expire;
};

struct time_state time_states[PLAT_CPU_NUM];

void timer_init(void)
{
        /* Per-core timer init */
        plat_timer_init();
}

static u64 get_next_tick_delta()
{
        u64 waiting_tick;

        /* Default tick */
        waiting_tick = TICK_MS * 1000 * tick_per_us;
        return waiting_tick;
}

void handle_timer_irq(void)
{
        u64 current_tick, tick_delta;

        /* Remove the thread to wakeup from sleep list */
        current_tick = plat_get_current_tick();

        /* Set when the next timer irq will arrive */
        tick_delta = get_next_tick_delta();

        time_states[smp_get_cpu_id()].next_expire = current_tick + tick_delta;
        plat_handle_timer_irq(tick_delta);
        sched_handle_timer_irq();
}

/*
 * clock_gettime:
 * - the return time is caculated from the system boot
 */
int sys_clock_gettime(clockid_t clock, struct timespec *ts)
{
        struct timespec ts_k;
        u64 mono_ns;

        if (!ts)
                return -1;

        copy_from_user((char *)&ts_k, (char *)ts, sizeof(ts_k));
        mono_ns = plat_get_mono_time();

        ts_k.tv_sec = mono_ns / NS_IN_S;
        ts_k.tv_nsec = mono_ns % NS_IN_S;

        copy_to_user((char *)ts, (char *)&ts_k, sizeof(ts_k));

        return 0;
}
