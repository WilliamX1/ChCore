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

#include <lib/printk.h>

#define WARNING 0
#define INFO    1
#define DEBUG   2

/* LOG_LEVEL is INFO by default */

#if LOG_LEVEL >= WARNING
#define kwarn(fmt, ...) printk("[WARN] file:%s " fmt, __FILE__, ##__VA_ARGS__)
#else
#define kwarn(fmt, ...)
#endif

#if LOG_LEVEL >= INFO
#define kinfo(fmt, ...) printk("[INFO] " fmt, ##__VA_ARGS__)
#else
#define kinfo(fmt, ...)
#endif

#if LOG_LEVEL >= DEBUG
#define kdebug(fmt, ...) printk("[DEBUG] " fmt, ##__VA_ARGS__)
#else
#define kdebug(fmt, ...)
#endif
