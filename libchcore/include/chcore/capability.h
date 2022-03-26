#pragma once

#include <chcore/types.h>

/* Capability of current cap_group */
#define SELF_CAP 0

#ifdef __cplusplus
extern "C" {
#endif

int chcore_cap_copy_to(u64 dest_cap_group_cap, u64 src_cap);
int chcore_cap_copy_from(u64 src_cap_group_cap, u64 src_cap);

int chcore_cap_transfer_multi(u64 dest_group_cap, int *src_caps, int nr_caps,
                              int *dest_caps);

#ifdef __cplusplus
}
#endif
