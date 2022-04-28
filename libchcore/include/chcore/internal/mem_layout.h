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

/*
 * Magic prefix for stack env page, the value is ASCII for "CHOS".
 * Keep it 4 bytes for compatibility with 32 bit architectures.
 */
#define ENV_MAGIC   0x43484f53
#define ENV_SIZE    0x1000
#define ENV_NO_CAPS (-1)

#include <chcore/internal/mem_layout_arch.h>
