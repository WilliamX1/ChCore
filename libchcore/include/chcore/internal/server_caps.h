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

#ifdef __cplusplus
extern "C" {
#endif

int __chcore_get_procm_cap(void);
int __chcore_get_fsm_cap(void);
int __chcore_get_tmpfs_cap(void);

void __chcore_set_procm_cap(int cap);
void __chcore_set_fsm_cap(int cap);
void __chcore_set_tmpfs_cap(int cap);

#ifdef __cplusplus
}
#endif
