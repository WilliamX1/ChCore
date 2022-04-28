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
#include <common/vars.h>

    .macro  read_ctr, reg
        mrs     \reg, ctr_el0
        nop
    .endm

    .macro  dcache_line_size, reg, tmp
	read_ctr        \tmp
        /* cache line size encoding */
	ubfm            \tmp, \tmp, #16, #19
        /* bytes per word */
	mov             \reg, #4
        /* actual cache line size */
	lsl             \reg, \reg, \tmp
    .endm

    .macro dcache_by_line_op op, domain, kaddr, size, tmp1, tmp2
        dcache_line_size \tmp1, \tmp2
        add     \size, \kaddr, \size
        sub     \tmp2, \tmp1, #1
        bic     \kaddr, \kaddr, \tmp2
1:
        .if     (\op == cvau || \op == cvac)
                dc      civac, \kaddr
        .else
                dc      \op, \kaddr
        .endif
        add     \kaddr, \kaddr, \tmp1
        cmp     \kaddr, \size
        b.lo    1b
        dsb     \domain
    .endm

BEGIN_FUNC(flush_dcache_area)
	dcache_by_line_op civac, sy, x0, x1, x2, x3
	ret
END_FUNC(flush_dcache_area)

BEGIN_FUNC(put32)
    str w1,[x0]
    ret
END_FUNC(put32)

BEGIN_FUNC(get32)
    ldr w0,[x0]
    ret
END_FUNC(get32)

BEGIN_FUNC(put64)
    str x1,[x0]
    ret
END_FUNC(put64)

BEGIN_FUNC(get64)
    ldr x0,[x0]
    ret
END_FUNC(get64)
