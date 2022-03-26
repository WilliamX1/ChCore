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

        /* LAB 3 TODO END */
}

static inline u32 __chcore_sys_getc(void)
{
        u32 ret = -1;
        /* LAB 3 TODO BEGIN */

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

        /* LAB 3 TODO END */
}

#ifdef __cplusplus
}
#endif
