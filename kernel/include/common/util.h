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

#include <common/types.h>

/*
 * memcpy does not handle: dst and src overlap.
 * memmove does.
 */
void memcpy(void *dst, const void *src, size_t size);
void memmove(void *dst, const void *src, size_t size);

void memset(void *dst, char ch, size_t size);

static inline int strcmp(const char *src, const char *dst)
{
        while (*src && *dst) {
                if (*src == *dst) {
                        src++;
                        dst++;
                        continue;
                }
                return *src - *dst;
        }
        if (!*src && !*dst)
                return 0;
        if (!*src)
                return -1;
        return 1;
}

static inline int strncmp(const char *src, const char *dst, size_t size)
{
        size_t i;

        for (i = 0; i < size; ++i) {
                if (src[i] == '\0' || src[i] != dst[i])
                        return src[i] - dst[i];
        }

        return 0;
}

static inline size_t strlen(const char *src)
{
        size_t i = 0;

        while (*src++)
                i++;

        return i;
}
