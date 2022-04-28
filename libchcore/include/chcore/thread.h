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

#define MAX_PRIO 255

#ifdef __cplusplus
extern "C" {
#endif

struct thread_args {
        u64 cap_group_cap;
        u64 stack;
        u64 pc;
        u64 arg;
        u32 prio;
        /* 0: TYPE_USER; 1: TYPE_SHADOW */
        u32 type;
};

enum thread_type {
        TYPE_USER = 0,
        TYPE_SHADOW = 1, /* SHADOW thread is used to achieve migrate IPC */
};

int chcore_thread_create(void *(*func)(void *), u64 arg, u32 prio, u32 type);

#ifdef __cplusplus
}
#endif
