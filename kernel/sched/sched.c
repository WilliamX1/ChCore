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

/* Scheduler related functions are implemented here */
#include <sched/sched.h>
#include <arch/machine/smp.h>
#include <common/kprint.h>
#include <machine.h>
#include <mm/kmalloc.h>
#include <common/list.h>
#include <common/util.h>
#include <object/thread.h>
#include <common/macro.h>
#include <common/errno.h>
#include <object/thread.h>
#include <irq/irq.h>
#include <sched/context.h>

struct thread *current_threads[PLAT_CPU_NUM];

/* Chosen Scheduling Policies */
struct sched_ops *cur_sched_ops;

char thread_type[][TYPE_STR_LEN] = {
        "IDLE",
        "KERNEL",
        "USER",
        "SHADOW",
        "REGISTER",
        "TESTS",
};

char thread_state[][STATE_STR_LEN] = {
        "TS_INIT      ",
        "TS_READY     ",
        "TS_INTER     ",
        "TS_RUNNING   ",
        "TS_EXIT      ",
        "TS_WAITING   ",
};

void print_thread(struct thread *thread)
{
        printk("Thread %p\tType: %s\tState: %s\tCPU %d\tAFF %d\t"
               "Budget %d\tPrio: %d\tIP: %p\tCMD: %s\n",
               thread,
               thread_type[thread->thread_ctx->type],
               thread_state[thread->thread_ctx->state],
               thread->thread_ctx->cpuid,
               thread->thread_ctx->affinity,
               /* REGISTER and SHADOW threads may have no sc, so just print -1.
                */
               thread->thread_ctx->sc ? thread->thread_ctx->sc->budget : -1,
               thread->thread_ctx->prio,
               arch_get_thread_next_ip(thread),
               thread->cap_group->cap_group_name);
}

int sched_is_running(struct thread *target)
{
        BUG_ON(!target);
        BUG_ON(!target->thread_ctx);

        if (target->thread_ctx->state == TS_RUNNING)
                return 1;
        return 0;
}

/*
 * Switch Thread to the specified one.
 * Set the correct thread state to running and
 * the per_cpu varible `current_thread`.
 *
 * Note: the switch is between current_thread and target.
 */
int switch_to_thread(struct thread *target)
{
        BUG_ON(!target);
        BUG_ON(!target->thread_ctx);
        BUG_ON((target->thread_ctx->state == TS_READY));
        BUG_ON((target->thread_ctx->thread_exit_state == TE_EXITED));

        /* No thread switch happens actually */
        if (target == current_thread) {
                target->thread_ctx->state = TS_RUNNING;

                /* The previous thread is the thread itself */
                target->prev_thread = THREAD_ITSELF;
                return 0;
        }

        target->thread_ctx->cpuid = smp_get_cpu_id();
        target->thread_ctx->state = TS_RUNNING;
        /* Record the thread transferring the CPU */
        target->prev_thread = current_thread;
        current_thread = target;
        return 0;
}

/*
 * An externeal interface for used in other places of the kernel,
 * e.g., IPC.
 * Note that this function never return back.
 */
void sched_to_thread(struct thread *target)
{
        BUG_ON((target->thread_ctx->state != TS_WAITING)
               && (target->thread_ctx->state != TS_INTER));
        /* Switch to itself? */
        BUG_ON(target == current_thread);

        /* If current thread has not been set to TS_WAITING,
         * put it into the ready queue before switching to
         * the target thread.
         */
        if (current_thread->thread_ctx->state != TS_WAITING)
                BUG_ON(sched_enqueue(current_thread));
        switch_to_thread(target);
        eret_to_thread(switch_context());
        /* The control flow will never return back. */
}

/*
 * Switch vmspace and arch-related stuff
 * Return the context pointer which should be set to stack pointer register
 */
u64 switch_context(void)
{
        struct thread *target_thread;
        struct thread_ctx *target_ctx;

        target_thread = current_thread;
        BUG_ON(!target_thread);
        BUG_ON(!target_thread->thread_ctx);

        target_ctx = target_thread->thread_ctx;

        if (target_thread->prev_thread == THREAD_ITSELF)
                return (u64)target_ctx;

        /* TYPE_TESTS threads do not have vmspace. */
        if (target_thread->thread_ctx->type != TYPE_TESTS) {
                BUG_ON(!target_thread->vmspace);
                switch_thread_vmspace_to(target_thread);
        }

        arch_switch_context(target_thread);

        return (u64)target_ctx;
}

/*
 * lab4: handle timer irq
 * Hints: Should check current_thread and its sc first
 */
void sched_handle_timer_irq(void)
{
        /* LAB 4 TODO BEGIN */
        if (current_thread && current_thread->thread_ctx && current_thread->thread_ctx->sc && current_thread->thread_ctx->sc->budget && current_thread->thread_ctx->sc->budget > 0) {
                --current_thread->thread_ctx->sc->budget;
        }
        /* LAB 4 TODO END */
}

/* SYSCALL functions */

void sys_yield(void)
{
        /* LAB 4 TODO BEGIN */
        if (current_thread && current_thread->thread_ctx && current_thread->thread_ctx->sc 
                && current_thread->thread_ctx->sc->budget) {
                current_thread->thread_ctx->sc->budget = 0;
        }
	sched();
	eret_to_thread(switch_context());
        /* LAB 4 TODO END */
        BUG("Should not return!\n");
}

void sys_top(void)
{
        cur_sched_ops->sched_top();
}

int sched_init(struct sched_ops *sched_ops)
{
        BUG_ON(sched_ops == NULL);

        cur_sched_ops = sched_ops;
        cur_sched_ops->sched_init();
        return 0;
}
