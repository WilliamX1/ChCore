#include <common/asm.h>
#include "irq_entry.h"

/* Functions of IRQ */

BEGIN_FUNC(set_exception_vector)
	adr	x0, el1_vector
	msr	vbar_el1, x0
	ret
END_FUNC(set_exception_vector)

/* 
 * DAIF 4 bits
 * D: debug exceptions
 * A: asynchronous aborts SError
 * I: IRQs
 * F: FIQs
 */
BEGIN_FUNC(enable_irq)
	msr	daifclr, #2
	ret
END_FUNC(enable_irq)

BEGIN_FUNC(disable_irq)
        msr     daifset, #2
        ret
END_FUNC(disable_irq)
