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

#pragma once

#include <chcore/types.h>
#include <chcore/internal/syscall_arch.h>
#include <chcore/internal/syscall_num.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Character */

static inline void __chcore_sys_putc(char ch)
{
        /* LAB 3 TODO BEGIN */
        __chcore_syscall1(__CHCORE_SYS_putc, ch);
        /* LAB 3 TODO END */
}

static inline u32 __chcore_sys_getc(void)
{
        u32 ret = -1;
        /* LAB 3 TODO BEGIN */
        ret = (u32) __chcore_syscall0(__CHCORE_SYS_getc);
        /* LAB 3 TODO END */
        return ret;
}

/* PMO */

/* - single */

static inline int __chcore_sys_create_pmo(u64 size, u64 type)
{
        return __chcore_syscall2(__CHCORE_SYS_create_pmo, size, type);
}

static inline int __chcore_sys_create_device_pmo(u64 paddr, u64 size)
{
        return __chcore_syscall2(__CHCORE_SYS_create_device_pmo, paddr, size);
}

static inline int __chcore_sys_map_pmo(u64 target_cap_group_cap, u64 pmo_cap,
                                       u64 addr, u64 perm, u64 len)
{
        return __chcore_syscall5(__CHCORE_SYS_map_pmo,
                                 target_cap_group_cap,
                                 pmo_cap,
                                 addr,
                                 perm,
                                 len);
}

static inline int __chcore_sys_unmap_pmo(u64 target_cap_group_cap, u64 pmo_cap,
                                         u64 addr)
{
        return __chcore_syscall3(
                __CHCORE_SYS_unmap_pmo, target_cap_group_cap, pmo_cap, addr);
}

static inline int __chcore_sys_write_pmo(u64 pmo_cap, u64 offset, u64 user_ptr,
                                         u64 len)
{
        return __chcore_syscall4(
                __CHCORE_SYS_write_pmo, pmo_cap, offset, user_ptr, len);
}

static inline int __chcore_sys_read_pmo(u64 pmo_cap, u64 offset, u64 user_ptr,
                                        u64 len)
{
        return __chcore_syscall4(
                __CHCORE_SYS_read_pmo, pmo_cap, offset, user_ptr, len);
}

/* - batch */

static inline int __chcore_sys_create_pmos(u64 user_buf, u64 cnt)
{
        return __chcore_syscall2(__CHCORE_SYS_create_pmos, user_buf, cnt);
}

static inline int __chcore_sys_map_pmos(u64 target_cap_group_cap, u64 user_buf,
                                        u64 cnt)
{
        return __chcore_syscall3(
                __CHCORE_SYS_map_pmos, target_cap_group_cap, user_buf, cnt);
}

/* - address translation */

static inline int __chcore_sys_get_pmo_paddr(u64 pmo_cap, u64 user_buf)
{
        return __chcore_syscall2(__CHCORE_SYS_get_pmo_paddr, pmo_cap, user_buf);
}

static inline int __chcore_sys_get_phys_addr(u64 va, u64 *pa_buf)
{
        return __chcore_syscall2(__CHCORE_SYS_get_phys_addr, va, (long)pa_buf);
}

/* Capability */

static inline int __chcore_sys_cap_copy_to(u64 dest_cap_group_cap,
                                           u64 src_slot_id)
{
        return __chcore_syscall2(
                __CHCORE_SYS_cap_copy_to, dest_cap_group_cap, src_slot_id);
}

static inline int __chcore_sys_cap_copy_from(u64 src_cap_group_cap,
                                             u64 src_slot_id)
{
        return __chcore_syscall2(
                __CHCORE_SYS_cap_copy_from, src_cap_group_cap, src_slot_id);
}

static inline int __chcore_sys_transfer_caps(u64 dest_group_cap,
                                             u64 src_caps_buf, int nr_caps,
                                             u64 dst_caps_buf)
{
        return __chcore_syscall4(__CHCORE_SYS_transfer_caps,
                                 dest_group_cap,
                                 src_caps_buf,
                                 nr_caps,
                                 dst_caps_buf);
}

/* Multitask */

/* - create & exit */

static inline int __chcore_sys_create_cap_group(u64 pid, u64 cap_group_name,
                                                u64 name_len, u64 pcid)
{
        return __chcore_syscall4(__CHCORE_SYS_create_cap_group,
                                 pid,
                                 cap_group_name,
                                 name_len,
                                 pcid);
}

static inline int __chcore_sys_create_thread(u64 thread_args_p)
{
        return __chcore_syscall1(__CHCORE_SYS_create_thread, thread_args_p);
}

static inline void __chcore_sys_thread_exit(void)
{
        /* LAB 3 TODO BEGIN */
        __chcore_syscall0(__CHCORE_SYS_thread_exit);
        /* LAB 3 TODO END */
}

/* - schedule */

static inline void __chcore_sys_yield(void)
{
        __chcore_syscall0(__CHCORE_SYS_yield);
}

static inline int __chcore_sys_set_affinity(u64 thread_cap, s32 aff)
{
        return __chcore_syscall2(__CHCORE_SYS_set_affinity, thread_cap, aff);
}

static inline s32 __chcore_sys_get_affinity(u64 thread_cap)
{
        return __chcore_syscall1(__CHCORE_SYS_get_affinity, thread_cap);
}

static inline u32 __chcore_sys_get_cpu_id()
{
        return __chcore_syscall0(__CHCORE_SYS_get_cpu_id);
}

/* IPC */

/* - procedure call */

static inline u64 __chcore_sys_register_server(u64 ipc_rountine, u64 max_client,
                                               u64 vm_config_ptr)
{
        return __chcore_syscall3(__CHCORE_SYS_register_server,
                                 ipc_rountine,
                                 max_client,
                                 vm_config_ptr);
}

static inline u32 __chcore_sys_register_client(u32 server_cap,
                                               u64 vm_config_ptr)
{
        return __chcore_syscall2(
                __CHCORE_SYS_register_client, server_cap, vm_config_ptr);
}

static inline u64 __chcore_sys_ipc_call(u32 conn_cap, void *ipc_msg_in_client,
                                        u64 cap_num)
{
        return __chcore_syscall3(__CHCORE_SYS_ipc_call,
                                 conn_cap,
                                 (long)ipc_msg_in_client,
                                 cap_num);
}

static inline void __chcore_sys_ipc_return(u64 ret, u64 cap_num)
{
        __chcore_syscall2(__CHCORE_SYS_ipc_return, ret, cap_num);
}

/* Hardware Access (Privileged Instruction) */

/* - cache */

static inline int __chcore_sys_cache_flush(u64 start, s64 len, int op_type)
{
        return __chcore_syscall3(__CHCORE_SYS_cache_flush, start, len, op_type);
}

/* - timer */

static inline u64 __chcore_sys_get_current_tick(void)
{
        return __chcore_syscall0(__CHCORE_SYS_get_current_tick);
}

/* Debug */

static inline void __chcore_sys_top(void)
{
        __chcore_syscall0(__CHCORE_SYS_top);
}

static inline u64 __chcore_sys_get_free_mem_size(void)
{
        return __chcore_syscall0(__CHCORE_SYS_get_free_mem_size);
}

/* Performance Benchmark */

static inline void __chcore_sys_perf_start(void)
{
        __chcore_syscall0(__CHCORE_SYS_perf_start);
}

static inline void __chcore_sys_perf_end(void)
{
        __chcore_syscall0(__CHCORE_SYS_perf_end);
}

/* lab4 semaphore */
static inline s32 __chcore_sys_create_sem(void)
{
        return __chcore_syscall0(__CHCORE_SYS_create_sem);
}

static inline s32 __chcore_sys_wait_sem(u32 sem_cap, bool is_block)
{
        return __chcore_syscall2(__CHCORE_SYS_wait_sem, sem_cap, is_block);
}

static inline s32 __chcore_sys_signal_sem(u32 sem_cap)
{
        return __chcore_syscall1(__CHCORE_SYS_signal_sem, sem_cap);
}

#ifdef __cplusplus
}
#endif
