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

#define LOCK_TEST_NUM 1000000

volatile int mutex_start_flag = 0;
volatile int mutex_finish_flag = 0;

/* Mutex test count */
unsigned long mutex_test_count = 0;

void tst_mutex(void)
{
        /* ============ Start Barrier ============ */
        lock_kernel();
        mutex_start_flag++;
        unlock_kernel();
        while (mutex_start_flag != PLAT_CPU_NUM)
                ;
        /* ============ Start Barrier ============ */

        /* Mutex Lock */
        for (int i = 0; i < LOCK_TEST_NUM; i++) {
                if (i % 2)
                        while (try_lock(&big_kernel_lock) != 0)
                                ;
                else
                        lock_kernel();
                /* Critical Section */
                mutex_test_count++;
                unlock_kernel();
        }

        /* ============ Finish Barrier ============ */
        lock_kernel();
        mutex_finish_flag++;
        unlock_kernel();
        while (mutex_finish_flag != PLAT_CPU_NUM)
                ;
        /* ============ Finish Barrier ============ */

        /* Check */
        BUG_ON(mutex_test_count != PLAT_CPU_NUM * LOCK_TEST_NUM);
        if (smp_get_cpu_id() == 0) {
                lock_kernel();
                kinfo("Pass tst_mutex!\n");
                unlock_kernel();
        }
}
