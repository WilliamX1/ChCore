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

#include <chcore/thread.h>
#include <chcore/capability.h>
#include <chcore/memory.h>
#include <malloc.h>
#include <chcore/assert.h>
#include <chcore/internal/mem_layout.h>
#include <chcore/internal/raw_syscall.h>

static int thread_num_in_cap_group = 0;

struct thread_entry_arg {
        void *(*func)(void *);
        void *arg;
};

/* Real thread entry here */
void *thread_entry(void *arg)
{
        struct thread_entry_arg *entry_args = arg;

        entry_args->func(entry_args->arg);
        /* Free the thread_entry_arg */
        free(entry_args);
        __chcore_sys_thread_exit(); /* no ret */
        chcore_bug("Should not reach here!\n");
        return 0;
}

int chcore_thread_create(void *(*func)(void *), u64 arg, u32 prio, u32 type)
{
        int child_stack_pmo_cap = 0;
        int child_thread_cap = 0;
        int ret = 0;
        int tid = 0; /* Thread ID */
        u64 thread_stack_base = 0;
        u64 thread_stack_top = 0;
        struct thread_args args;
        struct thread_entry_arg *entry_args;

        tid = __sync_fetch_and_add(&thread_num_in_cap_group, 1);

        child_stack_pmo_cap =
                chcore_pmo_create(CHILD_THREAD_STACK_SIZE, PMO_ANONYM);
        if (child_stack_pmo_cap < 0)
                return child_stack_pmo_cap;

        thread_stack_base =
                CHILD_THREAD_STACK_BASE + tid * CHILD_THREAD_STACK_SIZE;

        ret = chcore_pmo_map(SELF_CAP,
                             child_stack_pmo_cap,
                             thread_stack_base,
                             VM_READ | VM_WRITE);
        if (ret < 0)
                return ret;

        /*
         * GCC assumes the stack frame are aligned to 16-byte.
         * When invoking a function with call instruction,
         * the stack should align to 8-byte (not 16).
         * Otherwise, alignment error may occur
         * (at least on x86_64).
         * Besides, ChCore disables alignment check on aarch64.
         *
         * Thus, we deliberately set the initial SP here.
         */
        thread_stack_top = thread_stack_base + CHILD_THREAD_STACK_SIZE;
        thread_stack_top -= 8;

        args.cap_group_cap = SELF_CAP;
        args.stack = thread_stack_top;
        args.prio = prio;
        args.type = type;

        /*
         * For normal user thread, we use thread_entry as the
         * entrance of the thread. For shadow thread or register
         * thread, we use the specified func as the entrance.
         */
        if (type == TYPE_USER) {
                /*
                 * Save the entry func and the arg to entry_args
                 * Free when thread exit
                 */
                entry_args = malloc(sizeof(struct thread_entry_arg));
                entry_args->func = func;
                entry_args->arg = (void *)arg;
                args.pc = (u64)thread_entry;
                args.arg = (u64)entry_args;
        } else {
                args.pc = (u64)func;
                args.arg = arg;
        }

        child_thread_cap = __chcore_sys_create_thread((u64)&args);

        return child_thread_cap;
}
