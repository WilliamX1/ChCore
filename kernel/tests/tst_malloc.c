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

#define MALLOC_TEST_NUM   256
#define MALLOC_TEST_ROUND 10

volatile int malloc_start_flag = 0;
volatile int malloc_finish_flag = 0;

void tst_malloc(void)
{
        char *buf[MALLOC_TEST_NUM];
        /* ============ Start Barrier ============ */
        lock(&big_kernel_lock);
        malloc_start_flag++;
        unlock(&big_kernel_lock);
        while (malloc_start_flag != PLAT_CPU_NUM)
                ;
        /* ============ Start Barrier ============ */

        for (int round = 0; round < MALLOC_TEST_ROUND; round++) {
                for (int i = 0; i < MALLOC_TEST_NUM; i++) {
                        int size = 0x1000 + i;
                        buf[i] = kmalloc(size);
                        BUG_ON(!buf[i]);
                        for (int j = 0; j < size; j++) {
                                buf[i][j] = (char)(i + size);
                        }
                }

                for (int i = 0; i < MALLOC_TEST_NUM; i++) {
                        int size = 0x1000 + i;
                        for (int j = 0; j < size; j++) {
                                BUG_ON(buf[i][j] != (char)(i + size));
                        }
                        kfree(buf[i]);
                }
        }

        /* ============ Finish Barrier ============ */
        lock(&big_kernel_lock);
        malloc_finish_flag++;
        unlock(&big_kernel_lock);
        while (malloc_finish_flag != PLAT_CPU_NUM)
                ;
        /* ============ Finish Barrier ============ */

        if (smp_get_cpu_id() == 0) {
                kinfo("[TEST] malloc succ!\n");
        }
}
