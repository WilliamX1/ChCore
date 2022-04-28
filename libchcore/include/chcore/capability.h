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
