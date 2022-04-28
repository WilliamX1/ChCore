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
#include <stdio.h>

#define PRIO 255
#define INFO 233

void delay(int time)
{
        for (int i = 0; i < time; i++)
                for (int j = 0; j < 100000; j++)
                        asm volatile("nop");
}

void *thread_routine(void *arg)
{
        u32 times = 0;
        u64 thread_id = (u64)arg;

        printf("Hello, I am thread %u\n", thread_id);
        __chcore_sys_yield();

        while (times++ < 10) {
                printf("Iteration %lu, thread %lu, cpu %u\n",
                       times,
                       thread_id,
                       __chcore_sys_get_cpu_id());
                __chcore_sys_yield();
        }
        return 0;
}

int main(int argc, char *argv[])
{
        int child_thread_cap;
        u64 thread_i;

        printf("Hello from ChCore userland!\n");
        for (thread_i = 0; thread_i < 2; ++thread_i) {
                child_thread_cap = chcore_thread_create(
                        thread_routine, thread_i, PRIO, TYPE_USER);
                if (child_thread_cap < 0)
                        printf("Create thread failed, return %d\n",
                               child_thread_cap);
                delay(10);
        }
        return 0;
}
