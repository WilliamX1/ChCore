//#include <ipc/connection.h>
#include <common/kprint.h>
#include <common/vars.h>
#include <common/macro.h>
#include <common/types.h>
#include <io/uart.h>
#include <machine.h>

ALIGN(STACK_ALIGNMENT)
char kernel_stack[PLAT_CPU_NUM][KERNEL_STACK_SIZE];

/*
 * @addr is boot flag addresses for smp;
 */
void main(void *addr)
{
        BUG("[FATAL] Should never be here!\n");
}
