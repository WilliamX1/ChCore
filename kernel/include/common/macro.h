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

#ifndef __ASM__
#include <common/kprint.h>
#endif

#define ALIGN(n) __attribute__((__aligned__(n)))

#define ROUND_UP(x, n)     (((x) + (n)-1) & ~((n)-1))
#define ROUND_DOWN(x, n)   ((x) & ~((n)-1))
#define DIV_ROUND_UP(n, d) (((n) + (d)-1) / (d))

#define IS_ALIGNED(x, a) (((x) & ((typeof(x))(a)-1)) == 0)

#define BIT(x) (1ULL << (x))

#define offsetof(TYPE, MEMBER) ((u64) & ((TYPE *)0)->MEMBER)
#define container_of(ptr, type, field) \
        ((type *)((void *)(ptr) - (u64)(&(((type *)(0))->field))))

#define container_of_safe(ptr, type, field)                     \
        ({                                                      \
                typeof(ptr) __ptr = (ptr);                      \
                type *__obj = container_of(__ptr, type, field); \
                (__ptr ? __obj : NULL);                         \
        })

#define MAX(x, y) ((x) < (y) ? (y) : (x))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

#define BUG_ON(expr)                                        \
        do {                                                \
                if ((expr)) {                               \
                        printk("BUG: %s:%d on (expr) %s\n", \
                               __func__,                    \
                               __LINE__,                    \
                               #expr);                      \
                        for (;;) {                          \
                        }                                   \
                }                                           \
        } while (0)

#define BUG(str, ...)                          \
        do {                                   \
                printk("BUG: %s:%d " str "\n", \
                       __func__,               \
                       __LINE__,               \
                       ##__VA_ARGS__);         \
                for (;;) {                     \
                }                              \
        } while (0)

#define WARN(msg) printk("WARN: %s:%d %s\n", __func__, __LINE__, msg)

#define WARN_ON(cond, msg)                                      \
        do {                                                    \
                if ((cond)) {                                   \
                        printk("WARN: %s:%d %s on " #cond "\n", \
                               __func__,                        \
                               __LINE__,                        \
                               msg);                            \
                }                                               \
        } while (0)

#ifdef __GNUC__
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x)   (!!(x))
#define unlikely(x) (!!(x))
#endif // __GNUC__
