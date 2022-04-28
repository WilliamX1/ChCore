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

#pragma once

#include <common/list.h>
#include <mm/vmspace.h>
#include <sched/sched.h>
#include <object/cap_group.h>
#include <arch/machine/smp.h>
#include <ipc/connection.h>
#include <irq/timer.h>

extern struct thread *current_threads[PLAT_CPU_NUM];
#define current_thread          (current_threads[smp_get_cpu_id()])
#define DEFAULT_KERNEL_STACK_SZ (0x1000)

#define THREAD_ITSELF ((void *)(-1))

struct thread {
        struct list_head node; // link threads in a same cap_group
        struct list_head ready_queue_node; // link threads in a ready queue
        struct list_head sem_queue_node; // <lab4> sem use
        struct thread_ctx *thread_ctx; // thread control block

        /*
         * prev_thread switch CPU to this_thread
         *
         * When previous thread is the thread itself,
         * prev_thread will be set to THREAD_ITSELF.
         */
        struct thread *prev_thread;

        /*
         * vmspace: virtual memory address space.
         * vmspace is also stored in the 2nd slot of capability
         */
        struct vmspace *vmspace;
        struct cap_group *cap_group;

        /*
         * Only exists for threads in a server process.
         * If not NULL, it points to one of the three config types.
         */
        void *general_ipc_config;
        struct ipc_connection *active_conn;
};

void create_root_thread(void);
void switch_thread_vmspace_to(struct thread *);
void thread_deinit(void *thread_ptr);

/* Syscalls */
int sys_create_thread(u64 thread_args_p);
void sys_thread_exit(void);
int sys_set_affinity(u64 thread_cap, s32 aff);
s32 sys_get_affinity(u64 thread_cap);
