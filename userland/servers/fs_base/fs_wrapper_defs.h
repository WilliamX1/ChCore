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

#include <chcore/fs/list.h>
#include <chcore/fs/defs.h>
#include <chcore/ipc.h>
#include <sys/types.h>
#include <malloc.h>
#include <string.h>
#include <libc/sys/stat.h>

/* Indicates whether a certain fs has been mounted */
extern bool mounted;
extern bool using_page_cache;
extern struct fs_server_ops server_ops;

#define MAX_SERVER_ENTRY_PER_CLIENT     1024
/* (client_badge, fd) -> fid(server_entry) */
struct server_entry_node {
	u64 client_badge;
	int fd_to_fid[MAX_SERVER_ENTRY_PER_CLIENT];

	struct list_head node;
};

extern struct list_head server_entry_mapping;

void init_fs_wrapper(void);
int fs_wrapper_get_server_entry(u64 client_badge, int fd);
void fs_wrapper_set_server_entry(u64 client_badge, int fd, int fid);
void translate_fd_to_fid(u64 client_badge, struct fs_request *fr);

/**
 * FS Server Operation Vector
 *
 * NOTE:
 * Each fs server should implement its own operations,
 * 	and store in `server_ops` variable.
 * If there is any need to expand this structure,
 * 	do not forget add a default operation for every fs server impl.
 */
struct fs_server_ops {
	int (*open) (char *path, int flags, int mode, unsigned long *vnode_id, size_t *vnode_size, int *vnode_type, void **private);
	int (*read) (void *operator, unsigned long offset, size_t size, char *buf);
	int (*write) (void *operator, unsigned long offset, size_t size, const char *buf);
	int (*close) (void *operator, bool is_dir);

	int (*creat) (struct ipc_msg*ipc_msg, struct fs_request *fr);
	int (*unlink) (const char *path, int flags);
	int (*mkdir) (const char *path, mode_t mode);
	int (*rmdir) (const char *path, int flags);

	int (*getdents64) (struct ipc_msg*ipc_msg, struct fs_request *fr);
	int (*getsize) (char*);
};

int default_server_operation(struct ipc_msg*ipc_msg, struct fs_request *fr);
#define default_fmap_get_page_addr NULL
int fs_wrapper_open(u64 client_badge, struct ipc_msg*ipc_msg, struct fs_request *fr);
int fs_wrapper_close(struct ipc_msg*ipc_msg, struct fs_request *fr);
int fs_wrapper_read(struct ipc_msg*ipc_msg, struct fs_request *fr);
int fs_wrapper_write(struct ipc_msg*ipc_msg, struct fs_request *fr);
int fs_wrapper_lseek(struct ipc_msg*ipc_msg, struct fs_request *fr);
int fs_wrapper_unlink(struct ipc_msg*ipc_msg, struct fs_request *fr);
int fs_wrapper_rmdir(struct ipc_msg*ipc_msg, struct fs_request *fr);
int fs_wrapper_mkdir(struct ipc_msg*ipc_msg, struct fs_request *fr);
int fs_wrapper_creat(struct ipc_msg*ipc_msg, struct fs_request *fr);
int fs_wrapper_getdents64(struct ipc_msg*ipc_msg, struct fs_request *fr);
int fs_wrapper_get_size(struct ipc_msg *ipc_msg, struct fs_request *fr);

void fs_server_dispatch(struct ipc_msg*ipc_msg, u64 client_badge);
