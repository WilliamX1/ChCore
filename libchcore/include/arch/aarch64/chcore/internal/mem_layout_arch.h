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

/* Base address to place libc.so when launching dynamicaly linked apps. */
#define LIBC_SO_LOAD_BASE 0x400000000000UL

#define MAIN_THREAD_STACK_BASE 0x500000000000UL
#define MAIN_THREAD_STACK_SIZE 0x800000UL

#define CHILD_THREAD_STACK_BASE \
        (MAIN_THREAD_STACK_BASE + MAIN_THREAD_STACK_SIZE)
#define CHILD_THREAD_STACK_SIZE 0x800000UL

#define MEM_AUTO_ALLOC_REGION      0x300000000000UL
#define MEM_AUTO_ALLOC_REGION_SIZE 0x100000000000UL
