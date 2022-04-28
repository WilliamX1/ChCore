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
int child_thread_caps[THREAD_NUM];

void *thread_routine(void *arg)
{
        u32 times = 0;
        u64 thread_id = (u64)arg;
        u64 next_thread_id = (thread_id + 1) % THREAD_NUM;
        u64 prev_thread_id = (thread_id + THREAD_NUM - 1) % THREAD_NUM;
        int aff;

        while (times < 3) {
                times++;
                while (start_flags[thread_id] == 0)
                        ;
                start_flags[thread_id] = 0;

                __chcore_sys_yield();

                aff = __chcore_sys_get_affinity(child_thread_caps[thread_id]);

                printf("Iteration %lu, thread %lu, cpu %u, aff %d\n",
                       times,
                       thread_id,
                       __chcore_sys_get_cpu_id(),
                       aff);

                __chcore_sys_set_affinity(child_thread_caps[prev_thread_id],
                                          (thread_id + times) % 4);

                start_flags[next_thread_id] = 1;

                __chcore_sys_yield();
        }
        return 0;
}

int main(int argc, char *argv[])
{
        int i;
        u64 thread_i;

        for (thread_i = 0; thread_i < THREAD_NUM; ++thread_i) {
                start_flags[thread_i] = 0;
                child_thread_caps[thread_i] = chcore_thread_create(
                        thread_routine, thread_i, PRIO, TYPE_USER);
                if (child_thread_caps[thread_i] < 0)
                        printf("Create thread failed, return %d\n",
                               child_thread_caps[thread_i]);
                __chcore_sys_set_affinity(child_thread_caps[thread_i],
                                          thread_i % 4);
        }
        start_flags[0] = 1;
        return 0;
}
