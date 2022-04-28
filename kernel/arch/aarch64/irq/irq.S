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
