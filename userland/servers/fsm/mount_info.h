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

#include <chcore/fs/fs_defs.h>
#include <chcore/fs/list.h>
#include <string.h>
#include <malloc.h>
#include "defs.h"

/* ------------------------------------------------------------------------ */

/*
 * Mount Point Informations
 */
struct mount_point_info_node {
	int fs_cap;
	char path[MAX_MOUNT_POINT_LEN + 1];
	int path_len;
	ipc_struct_t *_fs_ipc_struct;
	int refcnt;

	struct list_head node;
};


#define MAX_SERVER_ENTRY_PER_CLIENT     1024
struct client_fd_info_node {
	u64 client_badge;
	int fd;
	ipc_struct_t *_fs_ipc_struct;
	struct mount_point_info_node* mount_point_info[MAX_SERVER_ENTRY_PER_CLIENT];

	struct list_head node;
};

extern int fs_num;

extern struct list_head mount_point_infos;
// extern pthread_rwlock_t mount_point_infos_rwlock;

struct mount_point_info_node *set_mount_point(const char *path, int path_len, int fs_cap);
struct mount_point_info_node *get_mount_point(const char *path, int path_len);
int remove_mount_point(const char *path);
