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

#define PRIO 255

#include "mutex.h"
#include <chcore/internal/raw_syscall.h>
#include <chcore/thread.h>
#include <stdio.h>

#define PLAT_CPU_NUM 4
#define THD_PER_CPU  4
#define TEST_NUM     100

struct lock global_lock;
int global_counter = 0;
int exit_sem;

void *routine0(void *arg)
{
        int local_var;
        __chcore_sys_yield();
        for (int i = 0; i < TEST_NUM; i++) {
                lock(&global_lock);
                local_var = global_counter;
                local_var++;
                global_counter = local_var;
                unlock(&global_lock);
        }

        __chcore_sys_signal_sem(exit_sem);
        return 0;
}

int main(int argc, char *argv[], char *envp[])
{
        int thread_cap;
        int global_exit = 0;

        exit_sem = __chcore_sys_create_sem();
        lock_init(&global_lock);

        printf("Begin Mutex Test!\n");
        for (int i = 0; i < PLAT_CPU_NUM * THD_PER_CPU; i++) {
                thread_cap = chcore_thread_create(routine0, i, PRIO, TYPE_USER);
                /* set affinity */
                __chcore_sys_set_affinity(thread_cap, i % PLAT_CPU_NUM);
        }
        while (global_exit < PLAT_CPU_NUM * THD_PER_CPU) {
                __chcore_sys_wait_sem(exit_sem, 1);
                global_exit++;
        }
        printf("Global Count %d\n", global_counter);
        if (global_counter == TEST_NUM * THD_PER_CPU * PLAT_CPU_NUM)
                printf("test_mutex passed!\n");
        else
                printf("Failed!\n");
        return 0;
}
