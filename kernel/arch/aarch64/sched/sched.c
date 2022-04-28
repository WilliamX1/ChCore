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

#include <sched/sched.h>
#include <arch/machine/registers.h>
#include <object/thread.h>
#include <common/vars.h>
#include <mm/kmalloc.h>
#include <lib/printk.h>

void arch_idle_ctx_init(struct thread_ctx *idle_ctx, void (*func)(void))
{
        /* Initialize to run the function `idle_thread_routine` */
        int i = 0;
        arch_exec_cont_t *ec = &(idle_ctx->ec);

        /* X0-X30 all zero */
        for (i = 0; i < REG_NUM; i++)
                ec->reg[i] = 0;
        /* SPSR_EL1 => Exit to EL1 */
        ec->reg[SPSR_EL1] = SPSR_EL1_KERNEL;
        /* ELR_EL1 => Next PC */
        ec->reg[ELR_EL1] = (u64)func;
}

inline void arch_switch_context(struct thread *target)
{
}
