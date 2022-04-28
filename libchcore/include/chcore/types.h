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

#include <chcore/types_arch.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __cplusplus
typedef int bool;
#define true (1)
#define false (0)
#endif

#ifdef __cplusplus
#define __CHCORE_NULL 0
#else
#define __CHCORE_NULL ((void *)0)
#endif

#ifndef NULL
#define NULL __CHCORE_NULL
#endif

#ifdef __cplusplus
}
#endif
