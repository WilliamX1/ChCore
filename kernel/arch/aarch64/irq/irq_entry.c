#include <irq/irq.h>
#include <common/types.h>
#include <common/kprint.h>
#include <common/util.h>
#include <sched/sched.h>
#include <arch/machine/smp.h>
#include <arch/machine/esr.h>
#include "irq_entry.h"

u8 irq_handle_type[MAX_IRQ_NUM];

void arch_enable_irqno(int irq)
{
        plat_enable_irqno(irq);
}

void arch_disable_irqno(int irq)
{
        plat_disable_irqno(irq);
}

void arch_interrupt_init_per_cpu(void)
{
        disable_irq();

        /* platform dependent init */
        set_exception_vector();
        plat_interrupt_init();
}

void arch_interrupt_init(void)
{
        arch_interrupt_init_per_cpu();
        memset(irq_handle_type, HANDLE_KERNEL, MAX_IRQ_NUM);
}

void handle_entry_c(int type, u64 esr, u64 address)
{
        /* Acquire the big kernel lock, if the exception is not from kernel */
        /* LAB 4 TODO BEGIN */
        if (type >= SYNC_EL0_64)
                lock_kernel();
        /* LAB 4 TODO END */

        /* ec: exception class */
        u32 esr_ec = GET_ESR_EL1_EC(esr);

        kdebug("Exception type: %d, ESR: 0x%lx, Fault address: 0x%lx, "
               "EC 0b%b\n",
               type,
               esr,
               address,
               esr_ec);

        /* Currently, ChCore only handles a part of IRQs */
        if (type < SYNC_EL0_64) {
                if (esr_ec != ESR_EL1_EC_DABT_CEL) {
                        kinfo("%s: irq type is %d\n", __func__, type);
                        BUG_ON(1);
                }
        }

        /* Dispatch exception according to EC */
        switch (esr_ec) {
        case ESR_EL1_EC_UNKNOWN:
                kdebug("Unknown\n");
                break;
        case ESR_EL1_EC_WFI_WFE:
                kdebug("Trapped WFI or WFE instruction execution\n");
                return;
        case ESR_EL1_EC_ENFP:
                kdebug("Access to SVE, Advanced SIMD, or floating-point functionality\n");
                break;
        case ESR_EL1_EC_ILLEGAL_EXEC:
                kdebug("Illegal Execution state\n");
                break;
        case ESR_EL1_EC_SVC_32:
                kdebug("SVC instruction execution in AArch32 state\n");
                break;
        case ESR_EL1_EC_SVC_64:
                kdebug("SVC instruction execution in AArch64 state\n");
                break;
        case ESR_EL1_EC_MRS_MSR_64:
                kdebug("Using MSR or MRS from a lower Exception level\n");
                break;
        case ESR_EL1_EC_IABT_LEL:
                kdebug("Instruction Abort from a lower Exception level\n");
                /* Page fault handler here:
                 * dynamic loading can trigger faults here.
                 */
                do_page_fault(esr, address);
                return;
        case ESR_EL1_EC_IABT_CEL:
                kinfo("Instruction Abort from current Exception level\n");
                break;
        case ESR_EL1_EC_PC_ALIGN:
                kdebug("PC alignment fault exception\n");
                break;
        case ESR_EL1_EC_DABT_LEL:
                kdebug("Data Abort from a lower Exception level\n");
                /* Handle faults caused by data access.
                 * We only consider page faults for now.
                 */
                do_page_fault(esr, address);
                return;
        case ESR_EL1_EC_DABT_CEL:
                kdebug("Data Abort from a current Exception level\n");
                do_page_fault(esr, address);
                return;
        case ESR_EL1_EC_SP_ALIGN:
                kdebug("SP alignment fault exception\n");
                break;
        case ESR_EL1_EC_FP_32:
                kdebug("Trapped floating-point exception taken from AArch32 state\n");
                break;
        case ESR_EL1_EC_FP_64:
                kdebug("Trapped floating-point exception taken from AArch64 state\n");
                break;
        case ESR_EL1_EC_SError:
                kdebug("SERROR\n");
                break;
        default:
                kdebug("Unsupported Exception ESR %lx\n", esr);
                break;
        }

        kinfo("Exception type: %d, ESR: 0x%lx, Fault address: 0x%lx, "
              "EC 0b%b\n",
              type,
              esr,
              address,
              esr_ec);

        BUG_ON(1);
}

/* Interrupt handler for interrupts happening when in EL0. */
void handle_irq(int type)
{
        /**
         * Lab4
         * Acquire the big kernel lock, if :
         *	The irq is not from the kernel
         * 	The thread being interrupted is an idle thread.
         */
        if (type >= SYNC_EL0_64
            || current_thread->thread_ctx->type == TYPE_IDLE) {
                /* LAB 4 TODO BEGIN */
                lock_kernel();
                /* LAB 4 TODO END */
        }

        plat_handle_irq();
        sched();
        eret_to_thread(switch_context());
}

void unexpected_handler(void)
{
        kinfo("[fatal error] %s is invoked\n", __func__);
        BUG_ON(1);
}

void __eret_to_thread(u64 sp);

void eret_to_thread(u64 sp)
{
        __eret_to_thread(sp);
}
