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

#define FALLOC_FL_KEEP_SIZE		0x01
#define FALLOC_FL_PUNCH_HOLE		0x02
#define FALLOC_FL_NO_HIDE_STALE		0x04

#define FALLOC_FL_COLLAPSE_RANGE	0x08
#define FALLOC_FL_ZERO_RANGE		0x10
#define FALLOC_FL_INSERT_RANGE		0x20
#define FALLOC_FL_UNSHARE_RANGE		0x40
