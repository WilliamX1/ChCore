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

#define le16_to_cpu(x) (x)
#define le32_to_cpu(x) (x)
#define le64_to_cpu(x) (x)

#define be16_to_cpu(x) ((((x)&0xff) << 8) | (((x) >> 8) & 0xff))
#define be32_to_cpu(x) ((be16_to_cpu((x)) << 16) | (be16_to_cpu((x) >> 16)))
#define be64_to_cpu(x) ((be32_to_cpu((x)) << 32) | (be32_to_cpu((x) >> 32)))

#define be128ptr_to_cpu_hi(x) (be64_to_cpu(*(u64 *)(x)))
#define be128ptr_to_cpu_lo(x) (be64_to_cpu(*((u64 *)(x) + 1)))

#define be96ptr_to_cpu_hi(x) (be32_to_cpu(*(u32 *)(x)))
#define be96ptr_to_cpu_lo(x)                           \
        (((u64)(be32_to_cpu(*((u32 *)(x) + 1)))) << 32 \
         | (be32_to_cpu(*((u32 *)(x)) + 2)))

#define beXptr_to_cpu(bits, n)                                              \
        ({                                                                  \
                (bits == 32) ? be32_to_cpu(*(u32 *)(n)) :                   \
                               (bits == 64) ? be64_to_cpu(*(u64 *)(n)) : ({ \
                                       BUG("invalid X");                    \
                                       0;                                   \
                               });                                          \
        })

#define set_beXptr_to_cpu(bits, n, hi, lo)                  \
        do {                                                \
                if (bits > 64) {                            \
                        if (bits == 96) {                   \
                                lo = be96ptr_to_cpu_lo(n);  \
                                hi = be96ptr_to_cpu_hi(n);  \
                        } else if (bits == 128) {           \
                                lo = be128ptr_to_cpu_lo(n); \
                                hi = be128ptr_to_cpu_hi(n); \
                        } else {                            \
                                BUG("invalid X");           \
                        }                                   \
                } else {                                    \
                        lo = 0;                             \
                        hi = beXptr_to_cpu(bits, n);        \
                }                                           \
        } while (0)
