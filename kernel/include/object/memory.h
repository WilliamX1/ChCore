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

#include <common/types.h>

/* Syscalls */
int sys_create_device_pmo(u64 paddr, u64 size);
int sys_create_pmo(u64 size, u64 type);
int sys_create_pmos(u64 user_buf, u64 cnt);
int sys_write_pmo(u64 pmo_cap, u64 offset, u64 user_ptr, u64 len);
int sys_read_pmo(u64 pmo_cap, u64 offset, u64 user_ptr, u64 len);
int sys_get_pmo_paddr(u64 pmo_cap, u64 user_buf);
int sys_get_phys_addr(u64 va, u64 *pa_buf);
int sys_map_pmo(u64 target_cap_group_cap, u64 pmo_cap, u64 addr, u64 perm,
                u64 len);
int sys_map_pmos(u64 target_cap_group_cap, u64 user_buf, u64 cnt);
int sys_unmap_pmo(u64 target_cap_group_cap, u64 pmo_cap, u64 addr);
u64 sys_handle_brk(u64 addr);
u64 sys_handle_mmap(u64 addr, size_t length, int prot, int flags, int fd,
                    u64 offset);
int sys_handle_munmap(u64 addr, size_t length);
u64 sys_get_free_mem_size(void);
