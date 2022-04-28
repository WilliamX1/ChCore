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

#include <common/asm.h>
#include <arch/machine/registers.h>
#include <arch/machine/esr.h>

#include "irq_entry.h"

.extern syscall_table
.extern lock_kernel
.extern unlock_kernel

.macro	exception_entry	label
	/* Each entry should be 0x80 aligned */
	.align	7
	b	\label
.endm

/* See more details a1bout the bias in registers.h */
.macro	exception_enter
	/* LAB 3 TODO BEGIN */

	/* LAB 3 TODO END */
	mrs	x21, sp_el0
	mrs	x22, elr_el1
	mrs	x23, spsr_el1
	/* LAB 3 TODO BEGIN */

	/* LAB 3 TODO END */
.endm

.macro	exception_exit
	/* LAB 3 TODO BEGIN */

	/* LAB 3 TODO END */
	msr	sp_el0, x21
	msr	elr_el1, x22
	msr	spsr_el1, x23
	/* LAB 3 TODO BEGIN */

	/* LAB 3 TODO END */
	eret
.endm

/*
 * Vector table offsets from vector table base address from ARMv8 Manual
 *	Address		|	Exception Type		| 	Description
 * ============================================================================
 *	VBAR_Eln+0x000	|	 Synchronous		|	 SPSel=0
 * 		+0x080	|	  IRQ/vIRQ		|	Current EL
 *		+0x100	|	  FIQ/vFIQ		|   with Stack Pointer
 * 		+0x180	|	SError/vSError		|    shared with EL0
 * ============================================================================
 *	VBAR_Eln+0x200	|	 Synchronous		|	 SPSel=1
 * 		+0x280	|	  IRQ/vIRQ		|	Current EL
 *		+0x300	|	  FIQ/vFIQ		|   with dedicated
 * 		+0x380	|	SError/vSError		|    Stack Pointer
 * ============================================================================
 *	VBAR_Eln+0x400	|	 Synchronous		|
 * 		+0x480	|	  IRQ/vIRQ		|	Lower EL
 *		+0x500	|	  FIQ/vFIQ		|    using AArch64
 * 		+0x580	|	SError/vSError		|
 * ============================================================================
 *	VBAR_Eln+0x600	|	 Synchronous		|
 * 		+0x680	|	  IRQ/vIRQ		|     	Lower EL
 *		+0x700	|	  FIQ/vFIQ		|    using AArch32
 * 		+0x780	|	SError/vSError		|
 * ============================================================================
 */

.align	11
EXPORT(el1_vector)
/* LAB 3 TODO BEGIN */

/* LAB 3 TODO END */
/*
 * The selected stack pointer can be indicated by a suffix to the Exception Level:
 *  - t: SP_EL0 is used
 *  - h: SP_ELx is used
 *
 * ChCore does not enable or handle irq_el1t, fiq_xxx, and error_xxx.
 * The SPSR_EL1 of idle threads is set to 0b0101, which means interrupt
 * are enabled during the their execution and SP_EL1 is selected (h).
 * Thus, irq_el1h is enabled and handled.
 *
 * Similarly, sync_el1t is also not enabled while we simply reuse the handler for
 * sync_el0 to handle sync_el1h (e.g., page fault during copy_to_user and fpu).
 */

irq_el1h:
    /* Simply reusing exception_enter/exit is OK. */
	exception_enter
	mov	x0, IRQ_EL1h
	bl	handle_irq
	exception_exit /* Lab4: Do not unlock! */

irq_el1t:
fiq_el1t:
fiq_el1h:
error_el1t:
error_el1h:
sync_el1t:
	/* LAB 3 TODO BEGIN */

	/* LAB 3 TODO END */

sync_el1h:
	exception_enter
	mov	x0, #SYNC_EL1h
	mrs	x1, esr_el1
	mrs	x2, elr_el1
	/* LAB 3 TODO BEGIN */

	/* LAB 3 TODO END */
	exception_exit /* Lab4: Do not unlock! */

sync_el0_64:
	/* Since we cannot touch x0-x7, we need some extra work here */
	exception_enter
	mrs	x25, esr_el1
	lsr	x24, x25, #ESR_EL1_EC_SHIFT
	cmp	x24, #ESR_EL1_EC_SVC_64
	b.eq	el0_syscall
	/* Not supported exception */
	mov	x0, SYNC_EL0_64 
	mrs	x1, esr_el1
	mrs	x2, elr_el1
	bl	handle_entry_c

	exception_exit

el0_syscall:

	sub	sp, sp, #16 * 8
	stp	x0, x1, [sp, #16 * 0]
	stp	x2, x3, [sp, #16 * 1]
	stp	x4, x5, [sp, #16 * 2]
	stp	x6, x7, [sp, #16 * 3]
	stp	x8, x9, [sp, #16 * 4]
	stp	x10, x11, [sp, #16 * 5]
	stp	x12, x13, [sp, #16 * 6]
	stp	x14, x15, [sp, #16 * 7]
	
	/* LAB 4 TODO BEGIN */

	/* LAB 4 TODO END */

	ldp	x0, x1, [sp, #16 * 0]
	ldp	x2, x3, [sp, #16 * 1]
	ldp	x4, x5, [sp, #16 * 2]
	ldp	x6, x7, [sp, #16 * 3]
	ldp	x8, x9, [sp, #16 * 4]
	ldp	x10, x11, [sp, #16 * 5]
	ldp	x12, x13, [sp, #16 * 6]
	ldp	x14, x15, [sp, #16 * 7]
	add	sp, sp, #16 * 8

	adr	x27, syscall_table		// syscall table in x27
	uxtw	x16, w8				// syscall number in x16
	ldr	x16, [x27, x16, lsl #3]		// find the syscall entry
	blr	x16

	/* Ret from syscall */
	str	x0, [sp]

	exception_exit


irq_el0_64:
	exception_enter
	mov	x0, IRQ_EL0_64
	bl	handle_irq
	/* should nerver reach here */
	b .

error_el0_64:
fiq_el0_64:
sync_el0_32:
irq_el0_32:
fiq_el0_32:
error_el0_32:
	bl unexpected_handler

/* void eret_to_thread(u64 sp) */
BEGIN_FUNC(__eret_to_thread)
	mov	sp, x0
	dmb ish

	exception_exit
END_FUNC(__eret_to_thread)
