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

#include <sys/types.h>

/* Spec from https://man.openbsd.org/FreeBSD-12.0/cpio.5 */

#define CPIO_FT_MASK 0170000
#define CPIO_REG 0100000
#define CPIO_DIR 0040000

struct cpio_newc_header {
        char c_magic[6];
        char c_ino[8];
        char c_mode[8];
        char c_uid[8];
        char c_gid[8];
        char c_nlink[8];
        char c_mtime[8];
        char c_filesize[8];
        char c_devmajor[8];
        char c_devminor[8];
        char c_rdevmajor[8];
        char c_rdevminor[8];
        char c_namesize[8];
        char c_check[8];
};

struct cpio_header {
        char c_magic[6];
        u64 c_ino;
        u64 c_mode;
        u64 c_uid;
        u64 c_gid;
        u64 c_nlink;
        u64 c_mtime;
        u64 c_filesize;
        u64 c_devmajor;
        u64 c_devminor;
        u64 c_rdevmajor;
        u64 c_rdevminor;
        u64 c_namesize;
        u64 c_check;
};

#define cpio_fatal(fmt, ...)                                                   \
        do {                                                                   \
                fprintf(stderr, "CPIO tool fatal error: " fmt, ##__VA_ARGS__); \
                exit(-1);                                                      \
        } while (0)

#define cpio_fatal_e(fmt, ...)                              \
        do {                                                \
                fprintf(stderr,                             \
                        "CPIO tool fatal error (%s): " fmt, \
                        strerror(errno),                    \
                        ##__VA_ARGS__);                     \
                exit(-1);                                   \
        } while (0)

#define ALIGN4_UP(x) ((((u64)x) & (~3llu)) + ((!!(((u64)x) & 3)) << 2))

struct cpio_file {
        struct cpio_header header;
        const char *name;
        const char *data;
        struct cpio_file *next;
};

struct {
        struct cpio_file head;
        struct cpio_file *tail;
} g_files;

void cpio_init_g_files(void);
int cpio_extract_file(const void *addr, const char *dirat);
void cpio_extract(const void *addr, const char *dirat);
int cpio_extract_single(const void *addr, const char *target,
                        int (*cpio_single_file_filler)(const void *start,
                                                       size_t size, void *data),
                        void *data);
