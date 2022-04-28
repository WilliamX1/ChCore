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

#include "cpio.h"
#include <chcore/fs/defs.h>
#include <chcore/ipc.h>
#include <chcore/memory.h>
#include <chcore/assert.h>
#include <stdio.h>
#include <string.h>
#include <libc/sys/stat.h>
#include "../fs_base/fs_wrapper_defs.h"
#include "../fs_base/fs_vnode.h"

int tmpfs_open(char *path, int flags, int mode, unsigned long *vnode_id, size_t *vnode_size, int *vnode_type, void **vnode_private);

int tmpfs_read(void *foperator, unsigned long offset, size_t size, char *buf);

int tmpfs_write(void *foperator, unsigned long offset, size_t size, const char *buf);

int tmpfs_close(void *foperator, bool is_dir);

int fs_creat(const char *path);

int tmpfs_creat(struct ipc_msg *ipc_msg, struct fs_request *fr);

int tmpfs_unlink(const char *path, int flags);

int tmpfs_rmdir(const char *path, int flags);


int tmpfs_mkdir(const char *path, mode_t mode);

int tfs_scan(struct inode *dir, unsigned int start, void *buf, void *end,
		    int *read_bytes);

int tmpfs_getdents(struct ipc_msg *ipc_msg, struct fs_request *fr);

int tmpfs_get_size(char* path);
