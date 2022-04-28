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

#include <string.h>

#include <chcore/types.h>

void memset(void *dest, int c, size_t n)
{
        for (size_t i = 0; i < n; i++) {
                ((u8 *)dest)[i] = (u8)c;
        }
}

void memcpy(void *dest, const void *src, size_t n)
{
        for (size_t i = 0; i < n; i++) {
                ((u8 *)dest)[i] = ((u8 *)src)[i];
        }
}

int memcmp(const void *s1, const void *s2, size_t n)
{
        for (size_t i = 0; i < n; i++) {
                if (((u8 *)s1)[i] != ((u8 *)s2)[i]) {
                        return ((u8 *)s1)[i] - ((u8 *)s2)[i];
                }
        }
        return 0;
}

char *strcpy(char *dest, const char *src)
{
        size_t i;
        for (i = 0; src[i] != '\0'; i++) {
                dest[i] = src[i];
        }
        dest[i] = '\0';
        return dest;
}

char *strncpy(char *dest, const char *src, size_t n)
{
        size_t i;
        for (i = 0; i < n && src[i] != '\0'; i++) {
                dest[i] = src[i];
        }
        for (; i < n; i++) {
                dest[i] = '\0';
        }
        return dest;
}

int strcmp(const char *s1, const char *s2)
{
        while (*s1 && *s1 == *s2) {
                s1++;
                s2++;
        }
        return (int)((u8)*s1 - (u8)*s2);
}

int strncmp(const char *s1, const char *s2, size_t n)
{
        while (n && *s1 && *s1 == *s2) {
                n--;
                s1++;
                s2++;
        }
        return n == 0 ? 0 : (int)((u8)*s1 - (u8)*s2);
}

size_t strlen(const char *s)
{
        size_t i;
        for (i = 0; *s; i++) {
                s++;
        }
        return i;
}

char *strstr(const char *haystack, const char *needle)
{
        size_t needle_len = strlen(needle);
        for (size_t i = 0; haystack[i] != '\0'; i++) {
                if (strncmp(haystack + i, needle, needle_len) == 0) {
                        return (char *)haystack + i;
                }
        }
        return NULL;
}

char *strcat(char *dest, const char *src)
{
        size_t dest_len = strlen(dest);
        size_t i;
        for (i = 0; src[i] != '\0'; i++) {
                dest[dest_len + i] = src[i];
        }
        dest[dest_len + i] = '\0';
        return dest;
}

char *strncat(char *dest, const char *src, size_t n)
{
        size_t dest_len = strlen(dest);
        size_t i;
        for (i = 0; i < n && src[i] != '\0'; i++) {
                dest[dest_len + i] = src[i];
        }
        dest[dest_len + i] = '\0';
        return dest;
}
