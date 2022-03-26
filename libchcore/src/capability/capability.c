#include <chcore/capability.h>
#include <chcore/internal/raw_syscall.h>

int chcore_cap_copy_to(u64 dest_cap_group_cap, u64 src_cap)
{
        return __chcore_sys_cap_copy_to(dest_cap_group_cap, src_cap);
}

int chcore_cap_copy_from(u64 src_cap_group_cap, u64 src_cap)
{
        return __chcore_sys_cap_copy_from(src_cap_group_cap, src_cap);
}

int chcore_cap_transfer_multi(u64 dest_group_cap, int *src_caps, int nr_caps,
                              int *dest_caps)
{
        return __chcore_sys_transfer_caps(
                dest_group_cap, (u64)src_caps, nr_caps, (u64)dest_caps);
}
