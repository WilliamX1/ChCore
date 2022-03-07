#include <common/asm.h>
#include <common/vars.h>

/* Args in x0 (boot_flag) should be passed to main */
BEGIN_FUNC(start_kernel)

    /* Set the kernel stack at high vaddr */
    ldr     x2, =kernel_stack
    add     x2, x2, KERNEL_STACK_SIZE
    mov     sp, x2

    /* Save x0 */
    str     x0, [sp, #-8]!

    /*
     * Make sure that we don't use low address in the kernel,
     * until we have our first user-level process.
     */
    mov     x2, #0
    msr     ttbr0_el1, x2
    isb

    /*
     * Call flush_tlb_all here to flush all the cached TLBs for
     * the boot time TTBR0_EL1.
     */
    bl flush_tlb_all

    /*
     * Code in bootloader specified only the primary 
     * cpu with MPIDR = 0 can be boot here. So we directly
     * set the TPIDR_EL1 to 0, which represent the logical
     * cpuid in the kernel 
     */
    mov     x3, #0
    msr     TPIDR_EL1, x3

    /* Restore x0 */
    ldr     x0, [sp], #8

    bl      main
END_FUNC(start_kernel)
