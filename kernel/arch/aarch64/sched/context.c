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

#include <object/thread.h>
#include <sched/sched.h>
#include <arch/machine/registers.h>
#include <arch/machine/smp.h>
#include <mm/kmalloc.h>

void init_thread_ctx(struct thread *thread, u64 stack, u64 func, u32 prio,
                     u32 type, s32 aff)
{
        /* Fill the context of the thread */
        thread->thread_ctx->ec.reg[SP_EL0] = stack;
        thread->thread_ctx->ec.reg[ELR_EL1] = func;
        thread->thread_ctx->ec.reg[SPSR_EL1] = SPSR_EL1_EL0t;

        /* Set the priority and state of the thread */
        thread->thread_ctx->prio = prio;
        thread->thread_ctx->state = TS_INIT;

        /* Set thread type */
        thread->thread_ctx->type = type;

        /* Set the cpuid and affinity */
        thread->thread_ctx->affinity = aff;

        /* Set the budget of the thread */
        if (thread->thread_ctx->sc != NULL) {
                thread->thread_ctx->sc->budget = DEFAULT_BUDGET;
        }
        /* Set exiting state */
        thread->thread_ctx->thread_exit_state = TE_RUNNING;
}

void arch_mask_interrupts_for_thread(struct thread *thread)
{
        /* Set the four interrupt mask bits. */
        thread->thread_ctx->ec.reg[SPSR_EL1] = SPSR_EL1_DEBUG | SPSR_EL1_SERROR
                                               | SPSR_EL1_IRQ | SPSR_EL1_FIQ;
}

u64 arch_get_thread_stack(struct thread *thread)
{
        return thread->thread_ctx->ec.reg[SP_EL0];
}

void arch_set_thread_stack(struct thread *thread, u64 stack)
{
        thread->thread_ctx->ec.reg[SP_EL0] = stack;
}

void arch_set_thread_return(struct thread *thread, u64 ret)
{
        thread->thread_ctx->ec.reg[X0] = ret;
}

void arch_set_thread_next_ip(struct thread *thread, u64 ip)
{
        /* Currently, we use fault PC to store the next ip */
        // thread->thread_ctx->ec.reg[FaultPC] = ip;
        /* Only required when we need to change PC */
        /* Maybe update ELR_EL1 directly */
        thread->thread_ctx->ec.reg[ELR_EL1] = ip;
}

u64 arch_get_thread_next_ip(struct thread *thread)
{
        return thread->thread_ctx->ec.reg[ELR_EL1];
}

void arch_set_thread_info_page(struct thread *thread, u64 info_page_addr)
{
        thread->thread_ctx->ec.reg[X0] = info_page_addr;
}

/* First argument in X0 */
void arch_set_thread_arg0(struct thread *thread, u64 arg)
{
        thread->thread_ctx->ec.reg[X0] = arg;
}

/* Second argument in X1 */
void arch_set_thread_arg1(struct thread *thread, u64 pid)
{
        thread->thread_ctx->ec.reg[X1] = pid;
}

/* set arch-specific thread state */
void set_thread_arch_spec_state(struct thread *thread)
{
        /* Currently, nothing need to be done in aarch64. */
}

void arch_enable_interrupt(struct thread *thread)
{
        thread->thread_ctx->ec.reg[SPSR_EL1] &= ~SPSR_EL1_IRQ;
}

void arch_disable_interrupt(struct thread *thread)
{
        thread->thread_ctx->ec.reg[SPSR_EL1] |= SPSR_EL1_IRQ;
}
