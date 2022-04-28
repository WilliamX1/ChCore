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

#include <chcore/internal/raw_syscall.h>
#include <chcore/thread.h>
#include <chcore/assert.h>
#include <stdio.h>

#define PRIO 255
#define INFO 233

#define THREAD_NUM 4

volatile u64 start_flags[THREAD_NUM];

void *thread_routine(void *arg)
{
        u64 thread_id = (u64)arg;

        while (start_flags[thread_id] == 0)
                ;
        __chcore_sys_yield();
        printf("Hello, I am thread %u on cpu %u\n",
               thread_id,
               __chcore_sys_get_cpu_id());

        start_flags[(thread_id + 1) % THREAD_NUM] = 1;
        return 0;
}

int main(int argc, char *argv[])
{
        int child_thread_cap;
        int i;
        u64 thread_i;

        for (thread_i = 0; thread_i < THREAD_NUM; ++thread_i) {
                start_flags[thread_i] = 0;
                child_thread_cap = chcore_thread_create(
                        thread_routine, thread_i, PRIO, TYPE_USER);
                if (child_thread_cap < 0)
                        printf("Create thread failed, return %d\n",
                               child_thread_cap);
                __chcore_sys_set_affinity(child_thread_cap, thread_i % 4);
        }

        start_flags[0] = 1;
        return 0;
}
