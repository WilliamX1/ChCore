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

#include <chcore/console.h>

#define chcore_bug(str)                                              \
        do {                                                         \
                chcore_console_printf(                               \
                        "BUG: %s:%d %s\n", __func__, __LINE__, str); \
                for (;;) {                                           \
                }                                                    \
        } while (0)

#define chcore_bug_on(expr)                                                    \
        do {                                                                   \
                if ((expr)) {                                                  \
                        chcore_console_printf(                                 \
                                "BUG: %s:%d %s\n", __func__, __LINE__, #expr); \
                        for (;;) {                                             \
                        }                                                      \
                }                                                              \
        } while (0)

#define chcore_warn(msg) \
        chcore_console_printf("WARN: %s:%d %s\n", __func__, __LINE__, msg)

#define chcore_warn_on(cond, msg)                                              \
        do {                                                                   \
                if ((cond)) {                                                  \
                        chcore_console_printf("WARN: %s:%d %s on " #cond "\n", \
                                              __func__,                        \
                                              __LINE__,                        \
                                              msg);                            \
                }                                                              \
        } while (0)

#define chcore_assert(expr)                \
        do {                               \
                if (!(expr)) {             \
                        chcore_bug(#expr); \
                }                          \
        } while (0)
