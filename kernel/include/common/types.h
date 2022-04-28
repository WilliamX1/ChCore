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

#include <common/macro.h>

typedef unsigned long long u64;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

typedef long long s64;
typedef int s32;
typedef short s16;
typedef signed char s8;

#include <posix/sys/types.h>

#define NULL ((void *)0)

typedef char bool;
#define true (1)
#define false (0)
typedef u64 paddr_t;
typedef u64 vaddr_t;

typedef u64 atomic_cnt;

/*
 * Different platform may have different cacheline size
 * and may have some features like prefetch.
 */
#define CACHELINE_SZ         64
#define pad_to_cache_line(n) (ROUND_UP(n, CACHELINE_SZ) - (n))
