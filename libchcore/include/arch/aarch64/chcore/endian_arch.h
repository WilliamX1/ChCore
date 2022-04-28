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

#define le16toh(x) (x)
#define le32toh(x) (x)
#define le64toh(x) (x)

#define be16toh(x) ((((x)&0xff) << 8) | (((x) >> 8) & 0xff))
#define be32toh(x) ((be16toh((x)) << 16) | (be16toh((x) >> 16)))
#define be64toh(x) ((be32toh((x)) << 32) | (be32toh((x) >> 32)))
