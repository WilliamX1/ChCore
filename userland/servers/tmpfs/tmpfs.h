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

#include <chcore/fs/defs.h>
#include <chcore/types.h>
#include <malloc.h>
#include <sys/types.h>
#include <chcore/fs/defs.h>
#include <chcore/internal/idman.h>
#include <chcore/assert.h>
#include <chcore/fs/defs.h>
#include <chcore/ipc.h>
#include <chcore/internal/raw_syscall.h>
#include <chcore/types.h>
#include <errno.h>
#include <malloc.h>
#include <stdio.h>
#include <sys/types.h>

#include "cpio.h"
#include "../fs_base/fs_vnode.h"

extern struct inode *tmpfs_root;
extern struct dentry *tmpfs_root_dent;
extern struct id_manager fidman;
extern struct fid_record fid_records[MAX_NR_FID_RECORDS];

int init_tmpfs(void);

int tfs_creat(struct inode *dir, const char *name, size_t len);
int tfs_mkdir(struct inode *dir, const char *name, size_t len);

int tfs_namex(struct inode **dirat, const char **name, int mkdir_p);
int tfs_remove(struct inode *dir, const char *name, size_t len);

ssize_t tfs_file_read(struct inode *inode, off_t offset, char *buff,
		      size_t size);
ssize_t tfs_file_write(struct inode *inode, off_t offset, const char *data,
		       size_t size);

int tfs_scan(struct inode *dir, unsigned int start, void *buf, void *end, int *readbytes);
struct inode *tfs_open_path(const char *path);

int tfs_load_image(const char *start);

int del_inode(struct inode *inode);

static inline struct inode *get_inode(struct inode *i)
{
	i->refcnt++;
	return i;
}
static inline int put_inode(struct inode *i) {
	i->refcnt--;
	chcore_assert(i->refcnt >= 0);
	if (!i->refcnt)
		return del_inode(i);
	return 0;
}



