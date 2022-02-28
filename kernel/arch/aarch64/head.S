#include <common/asm.h>
#include <common/vars.h>

/* Args in x0 should be passed to main */
BEGIN_FUNC(start_kernel)
    /* Set the kernel stack at high vaddr */
    ldr     x2, =kernel_stack
    add     x2, x2, KERNEL_STACK_SIZE
    mov     sp, x2

    /* 
     * Code in bootloader specified only the primary 
     * cpu with MPIDR = 0 can be boot here. So we directly
     * set the TPIDR_EL1 to 0, which represent the logical
     * cpuid in the kernel 
     */
    mov     x3, #0
    msr     TPIDR_EL1, x3

    bl      main
END_FUNC(start_kernel)
