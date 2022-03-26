#pragma once

#define SYNC_EL1t  0
#define IRQ_EL1t   1
#define FIQ_EL1t   2
#define ERROR_EL1t 3

#define SYNC_EL1h  4
#define IRQ_EL1h   5
#define FIQ_EL1h   6
#define ERROR_EL1h 7

#define SYNC_EL0_64  8
#define IRQ_EL0_64   9
#define FIQ_EL0_64   10
#define ERROR_EL0_64 11

#define SYNC_EL0_32  12
#define IRQ_EL0_32   13
#define FIQ_EL0_32   14
#define ERROR_EL0_32 15

#ifndef __ASM__
/* assembly helper functions */
void set_exception_vector(void);
void enable_irq(void);
void disable_irq(void);
/* fault handlers */
void do_page_fault(u64 esr, u64 fault_addr);
#endif /* __ASM__ */