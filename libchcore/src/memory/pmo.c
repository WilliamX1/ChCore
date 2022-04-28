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

#include <chcore/memory.h>
#include <chcore/internal/raw_syscall.h>

int chcore_pmo_create(u64 size, u64 type)
{
        return __chcore_sys_create_pmo(size, type);
}

int chcore_pmo_map(u64 target_cap_group_cap, u64 pmo_cap, u64 addr, u64 perm)
{
        return __chcore_sys_map_pmo(
                target_cap_group_cap, pmo_cap, addr, perm, -1);
}

int chcore_pmo_unmap(u64 target_cap_group_cap, u64 pmo_cap, u64 addr)
{
        return __chcore_sys_unmap_pmo(target_cap_group_cap, pmo_cap, addr);
}

int chcore_pmo_write(u64 pmo_cap, u64 offset, void *buf, u64 len)
{
        return __chcore_sys_write_pmo(pmo_cap, offset, (u64)buf, len);
}

int chcore_pmo_read(u64 pmo_cap, u64 offset, void *buf, u64 len)
{
        return __chcore_sys_read_pmo(pmo_cap, offset, (u64)buf, len);
}

int chcore_pmo_create_multi(struct pmo_request *reqs, u64 nr_reqs)
{
        return __chcore_sys_create_pmos((u64)reqs, nr_reqs);
}

int chcore_pmo_map_multi(u64 target_cap_group_cap, struct pmo_map_request *reqs,
                         u64 nr_reqs)
{
        return __chcore_sys_map_pmos(target_cap_group_cap, (u64)reqs, nr_reqs);
}
