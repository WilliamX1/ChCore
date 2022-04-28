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
#include <common/kprint.h>
#include <arch/machine/smp.h>
#include <common/macro.h>
#include <mm/kmalloc.h>
#include "tests.h"
#include "barrier.h"

struct lock test_lock;

void init_test(void)
{
        u32 ret = 0;

        global_barrier_init();
        ret = lock_init(&test_lock);
        BUG_ON(ret != 0);
}

void run_test(void)
{
        init_test();
        tst_mutex();
        tst_sched_cooperative();
        tst_sched_preemptive();
        tst_sched_affinity();
        tst_sched();
}
