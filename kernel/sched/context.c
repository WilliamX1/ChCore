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

#include <machine.h>
#include <object/thread.h>
#include <sched/sched.h>
#include <mm/kmalloc.h>
#include <common/util.h>
#include <common/kprint.h>

/*
 * The kernel stack for a thread looks like:
 *
 * ---------- thread_ctx end: kernel_stack_base + DEFAULT_KERNEL_STACK_SZ
 * (0x1000)
 *    ...
 * other fields in the thread_ctx:
 * e.g., sched_ctx
 *    ...
 * execution context (e.g., registers)
 * ---------- thread_ctx
 *    ...
 *    ...
 * stack in use for when the thread enters kernel
 *    ...
 *    ...
 * ---------- kernel_stack_base
 *
 */
struct thread_ctx *create_thread_ctx(u32 type)
{
        void *kernel_stack;
        struct thread_ctx *ctx;
        sched_cont_t *sc;

        kernel_stack = kzalloc(DEFAULT_KERNEL_STACK_SZ);
        if (kernel_stack == NULL) {
                kwarn("create_thread_ctx fails due to lack of memory\n");
                return NULL;
        }
        ctx = (struct thread_ctx *)(kernel_stack + DEFAULT_KERNEL_STACK_SZ
                                    - sizeof(struct thread_ctx));

        /* Threads whose types are SHADOW don't have scheduling
         * context. */
        if (type == TYPE_SHADOW) {
                ctx->sc = NULL;
        } else {
                /* Allocate a scheduling context for threads of other types */
                sc = kzalloc(sizeof(sched_cont_t));
                if (sc == NULL) {
                        kwarn("create_thread_ctx fails due to lack of memory\n");
                        kfree(kernel_stack);
                        return NULL;
                }
                ctx->sc = sc;
        }

        return ctx;
}

void destroy_thread_ctx(struct thread *thread)
{
        void *kernel_stack;

        BUG_ON(!thread->thread_ctx);

        /* Register or shadow threads do not have scheduling contexts */
        if (thread->thread_ctx->type != TYPE_SHADOW) {
                BUG_ON(!thread->thread_ctx->sc);
                kfree(thread->thread_ctx->sc);
        }

        kernel_stack = (void *)thread->thread_ctx - DEFAULT_KERNEL_STACK_SZ
                       + sizeof(struct thread_ctx);
        kfree(kernel_stack);
}
