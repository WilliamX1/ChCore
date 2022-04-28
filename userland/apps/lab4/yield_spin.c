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

bool running = true;

void *thread_routine(void *arg)
{
        u64 thread_id = (u64)arg;

        printf("Hello, I am thread %u\n", thread_id);

        while (running) {
        }
        return 0;
}

int main(int argc, char *argv[])
{
        int child_thread_cap;

        child_thread_cap =
                chcore_thread_create(thread_routine, 1, PRIO, TYPE_USER);
        if (child_thread_cap < 0)
                printf("Create thread failed, return %d\n", child_thread_cap);

        __chcore_sys_yield();

        printf("Successfully regain the control!\n");
        return 0;
}
