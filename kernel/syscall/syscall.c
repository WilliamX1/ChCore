#include <common/types.h>
#include <io/uart.h>
#include <mm/uaccess.h>
#include <mm/kmalloc.h>
#include <mm/mm.h>
#include <common/kprint.h>
#include <object/memory.h>
#include <object/thread.h>
#include <object/cap_group.h>
#include <object/object.h>
#include <sched/sched.h>
#include <irq/irq.h>
#include <arch/machine/smp.h>

#include "syscall_num.h"

/* Placeholder for system calls that are not implemented */
void sys_null_placeholder(long arg)
{
        BUG("Invoke non-implemented syscall\n");
}

void sys_putc(char ch)
{
        /* LAB 3 TODO BEGIN */
        uart_send(ch);
        /* LAB 3 TODO END */
}

u32 sys_getc(void)
{
        /* LAB 3 TODO BEGIN */
        return uart_recv();
        /* LAB 3 TODO END */
}

const void *syscall_table[NR_SYSCALL] = {
        [0 ... NR_SYSCALL - 1] = sys_null_placeholder,

        /* Character */
        [SYS_putc] = sys_putc,
        [SYS_getc] = sys_getc,

        /* PMO */
        /* - single */
        [SYS_create_pmo] = sys_create_pmo,
        [SYS_create_device_pmo] = sys_create_device_pmo,
        [SYS_map_pmo] = sys_map_pmo,
        [SYS_unmap_pmo] = sys_unmap_pmo,
        [SYS_write_pmo] = sys_write_pmo,
        [SYS_read_pmo] = sys_read_pmo,
        /* - batch */
        [SYS_create_pmos] = sys_create_pmos,
        [SYS_map_pmos] = sys_map_pmos,
        /* - address translation */
        [SYS_get_pmo_paddr] = sys_get_pmo_paddr,
        [SYS_get_phys_addr] = sys_get_phys_addr,

        /* Capability */
        [SYS_cap_copy_to] = sys_cap_copy_to,
        [SYS_cap_copy_from] = sys_cap_copy_from,
        [SYS_transfer_caps] = sys_transfer_caps,

        /* Multitask */
        /* - create & exit */
        [SYS_create_cap_group] = sys_create_cap_group,
        [SYS_create_thread] = sys_create_thread,
        [SYS_thread_exit] = sys_thread_exit,

        /* POSIX */
        /* - memory */
        [SYS_handle_brk] = sys_handle_brk,
        [SYS_handle_mmap] = sys_handle_mmap,
        [SYS_handle_munmap] = sys_handle_munmap,
};
