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
#include <ipc/connection.h>
#include <irq/timer.h>
#include <irq/irq.h>
#include <semaphore/semaphore.h>
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

/* Arch-specific declarations */
void arch_flush_cache(u64, s64, int);
u64 plat_get_current_tick(void);

/* Helper system calls for user-level drivers to use. */
int sys_cache_flush(u64 start, s64 len, int op_type)
{
        arch_flush_cache(start, len, op_type);
        return 0;
}

u64 sys_get_current_tick(void)
{
        return plat_get_current_tick();
}

/* Syscalls for perfromance benchmark */
void sys_perf_start(void)
{
        kdebug("Disable TIMER\n");
        plat_disable_timer();
}

void sys_perf_end(void)
{
        kdebug("Enable TIMER\n");
        plat_enable_timer();
}

/* Get cpu id */
u32 sys_get_cpu_id()
{
        u32 cpuid = 0;
        /* LAB 4 TODO BEGIN */
        cpuid = smp_get_cpu_id();
        /* LAB 4 TODO END */
        return cpuid;
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
        /* - schedule */
        [SYS_yield] = sys_yield,
        [SYS_set_affinity] = sys_set_affinity,
        [SYS_get_affinity] = sys_get_affinity,
        [SYS_get_cpu_id] = sys_get_cpu_id,

        /* IPC */
        /* - procedure call */
        [SYS_register_server] = sys_register_server,
        [SYS_register_client] = sys_register_client,
        [SYS_ipc_call] = sys_ipc_call,
        [SYS_ipc_return] = sys_ipc_return,

        /* Hardware Access (Privileged Instruction) */
        /* - cache */
        [SYS_cache_flush] = sys_cache_flush,
        /* - timer */
        [SYS_get_current_tick] = sys_get_current_tick,

        /* POSIX */
        /* - time */
        [SYS_clock_gettime] = sys_clock_gettime,
        /* - memory */
        [SYS_handle_brk] = sys_handle_brk,
        [SYS_handle_mmap] = sys_handle_mmap,
        [SYS_handle_munmap] = sys_handle_munmap,

        /* Debug */
        [SYS_top] = sys_top,
        [SYS_get_free_mem_size] = sys_get_free_mem_size,

        /* Performance Benchmark */
        [SYS_perf_start] = sys_perf_start,
        [SYS_perf_end] = sys_perf_end,

        /* lab4 semaphore */
        [SYS_create_sem] = sys_create_sem,
        [SYS_wait_sem] = sys_wait_sem,
        [SYS_signal_sem] = sys_signal_sem,
};
