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

#include "fsm.h"
#include <string.h>
#include <chcore/fs/defs.h>
#include <chcore/procm.h>
#include <chcore/internal/raw_syscall.h>
#include <chcore/internal/server_caps.h>
#include <chcore/fakefs.h>
#include "mount_info.h"

int fs_num = 0;

int rootfs_cap = -1;

struct list_head fsm_mount_info_mapping;


struct spinlock fsmlock;
/* Initialize when fsm start */
static inline void init_utils()
{
	init_list_head(&mount_point_infos);
	init_list_head(&fsm_mount_info_mapping);
}

int init_fsm(void)
{
	u64 size;
	int ret;

	/* Initialize */
	init_utils();

	spinlock_init(&fsmlock);

	ret = fsm_mount_fs("/tmpfs.srv", "/");
	if (ret < 0) {
		error("failed to mount tmpfs, ret %d\n", ret);
		__chcore_sys_thread_exit();
		
	}

	return 0;
}

int rootfs_mkdir(const char* mount_point) 
{
		struct mount_point_info_node *mpinfo;
		mpinfo = get_mount_point("/", 1);
		BUG_ON(!mpinfo);
		BUG_ON(!mpinfo->_fs_ipc_struct);
		struct ipc_msg * ipc_msg = ipc_create_msg(mpinfo->_fs_ipc_struct, sizeof(struct fs_request), 0);
		chcore_assert(ipc_msg);
		struct fs_request * fr =
			(struct fs_request *)ipc_get_msg_data(ipc_msg);
		fr->req = FS_REQ_MKDIR;
		strcpy(fr->mkdir.pathname, mount_point);
		int ret = ipc_call(mpinfo->_fs_ipc_struct, ipc_msg);
		ipc_destroy_msg(mpinfo->_fs_ipc_struct, ipc_msg);
		return ret;
}


int fsm_mount_fs(const char *path, const char *mount_point)
{
	int fs_cap;
	int ret = -1;
	struct mount_point_info_node *mp_node;

	ret = -1;
	if (fs_num == MAX_FS_NUM) {
		error("maximal number of FSs is reached: %d\n", fs_num);
		goto out;
	}

	if (strlen(mount_point) > MAX_MOUNT_POINT_LEN) {
		error("mount point too long: > %d\n", MAX_MOUNT_POINT_LEN);
		goto out;
	}

	if (mount_point[0] != '/') {
		error("mount point should start with '/'\n");
		goto out;
	}

	if (strcmp(path, "/tmpfs.srv") == 0) {
		int fs_cap = __chcore_get_tmpfs_cap();
		chcore_assert(fs_cap > 0);
		mp_node = set_mount_point("/", 1, fs_cap);
		ret = fs_cap;
		rootfs_cap = fs_cap;
	} else if(strcmp(path, "/fakefs.srv") == 0) {
		int fs_cap;
		chcore_procm_spawn("/fakefs.srv", &fs_cap);
		chcore_assert(fs_cap > 0);
		mp_node = set_mount_point(mount_point, strlen(mount_point), fs_cap);
		ret = fs_cap;
		chcore_fakefs_test(fs_cap);
		BUG_ON(rootfs_cap == -1);
		rootfs_mkdir(mount_point);
	} else {
		chcore_assert(0);
	}

	/* Connect to the FS that we mount now. */
	mp_node->_fs_ipc_struct = ipc_register_client(mp_node->fs_cap);

	if (mp_node->_fs_ipc_struct == NULL) {
		info("ipc_register_client failed\n");
		BUG_ON(remove_mount_point(mp_node->path) != 0);
		goto out;
	}

	strncpy(mp_node->path, mount_point, sizeof(mp_node->path));

	fs_num++;

out:
	return ret;
}

/*
 * @args: 'path' is device name, like 'sda1'...
 * send FS_REQ_UMOUNT to corresponding fs_server
 */
int fsm_umount_fs(const char *path)
{
	return remove_mount_point(path);
}
