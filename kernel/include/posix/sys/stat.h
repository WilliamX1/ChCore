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

#include <posix/sys/types.h>
#include <posix/time.h>

/* struct stat from https://pubs.opengroup.org/onlinepubs/9699919799/ */
struct stat {
        /* Device ID of device containing file. */
        dev_t st_dev;

        /* File serial number. */
        ino_t st_ino;

        /* Mode of file (see below). */
        mode_t st_mode;

        /* Number of hard links to the file. */
        nlink_t st_nlink;

        /* User ID of file. */
        uid_t st_uid;

        /* Group ID of file. */
        gid_t st_gid;

        /* Device ID (if file is character or block special). */
        dev_t st_rdev;

        /* For regular files, the file size in bytes.
         * For symbolic links, the length in bytes of the
         * pathname contained in the symbolic link.
         * For a shared memory object, the length in bytes.
         * For a typed memory object, the length in bytes.
         * For other file types, the use of this field is
         * unspecified.
         */
        off_t st_size;

        /* Last data access timestamp. */
        struct timespec st_atim;
        /* Last data modification timestamp. */
        struct timespec st_mtim;
        /* Last file status change timestamp. */
        struct timespec st_ctim;

        /* A file system-specific preferred I/O block size
         * for this object. In some file system types, this
         * may vary from file to file.
         */
        blksize_t st_blksize;

        /* Number of blocks allocated for this object. */
        blkcnt_t st_blocks;
};
