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
