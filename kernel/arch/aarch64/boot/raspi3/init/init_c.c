#include "boot.h"
#include "image.h"
#include "consts.h"

#include <common/types.h>

char boot_cpu_stack[PLAT_CPU_NUMBER][INIT_STACK_SIZE] ALIGN(16);

/*
 * Initialize these varibles in order to make them not in .bss section.
 * So, they will have concrete initial value even on real machine.
 *
 * Non-primary CPUs will spin until they see the secondary_boot_flag becomes
 * non-zero which is set in kernel (see enable_smp_cores).
 *
 * The secondary_boot_flag is initilized as {NOT_BSS, 0, 0, ...}.
 */
#define NOT_BSS (0xBEEFUL)
long secondary_boot_flag[PLAT_CPU_NUMBER] = {NOT_BSS};
volatile u64 clear_bss_flag = NOT_BSS;

/* Uart */
void early_uart_init(void);
void uart_send_string(char *);

static void clear_bss(void)
{
        u64 bss_start_addr;
        u64 bss_end_addr;
        u64 i;

        bss_start_addr = (u64)&_bss_start;
        bss_end_addr = (u64)&_bss_end;

        for (i = bss_start_addr; i < bss_end_addr; ++i)
                *(char *)i = 0;

        clear_bss_flag = 0;
}

void init_c(void)
{
        /* Clear the bss area for the kernel image */
        clear_bss();

        /* Initialize UART before enabling MMU. */
        early_uart_init();
        uart_send_string("boot: init_c\r\n");

        /* Initialize Boot Page Table. */
        uart_send_string("[BOOT] Install boot page table\r\n");
        init_boot_pt();

        /* Enable MMU. */
        el1_mmu_activate();
        uart_send_string("[BOOT] Enable el1 MMU\r\n");

        /* Call Kernel Main. */
        uart_send_string("[BOOT] Jump to kernel main\r\n");
        start_kernel(secondary_boot_flag);

        /* Never reach here */
}
