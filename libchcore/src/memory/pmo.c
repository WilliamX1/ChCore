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
