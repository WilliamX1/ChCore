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

/* typedef void *DIR; */

struct dirent {
        ino_t d_ino; /* Inode number */
        off_t d_off; /* Not an offset; see below */
        unsigned short d_reclen; /* Length of this record */
        unsigned char d_type; /* Type of file; not supported
                                 by all filesystem types */
        char d_name[256]; /* Null-terminated filename */
};
