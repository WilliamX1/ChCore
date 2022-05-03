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

#include "buf.h"
#include <chcore/internal/raw_syscall.h>
#include <chcore/thread.h>
#include <stdio.h>
#include "prodcons.h"

#define PRIO 255

volatile int global_exit = 0;
s32 empty_slot;
s32 filled_slot;

void *producer(void *arg)
{
        int new_msg;
        int i = 0;

        while (i < PROD_ITEM_CNT) {
                /* LAB 4 TODO BEGIN */
                __chcore_sys_wait_sem(empty_slot, true);
                /* LAB 4 TODO END */
                new_msg = produce_new();
                buffer_add_safe(new_msg);
                /* LAB 4 TODO BEGIN */
                __chcore_sys_signal_sem(filled_slot);
                /* LAB 4 TODO END */
                i++;
        }
        __sync_fetch_and_add(&global_exit, 1);
        return 0;
}

void *consumer(void *arg)
{
        int cur_msg;
        int i = 0;

        while (i < COSM_ITEM_CNT) {
                /* LAB 4 TODO BEGIN */
                __chcore_sys_wait_sem(filled_slot, true);
                /* LAB 4 TODO END */
                cur_msg = buffer_remove_safe();
                /* LAB 4 TODO BEGIN */
                __chcore_sys_signal_sem(empty_slot);
                /* LAB 4 TODO END */
                consume_msg(cur_msg);
                i++;
        }
        __sync_fetch_and_add(&global_exit, 1);
        return 0;
}
