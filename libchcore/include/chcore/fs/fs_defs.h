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

#ifdef __cplusplus
extern "C" {
#endif

/* This file will be used in both `fsm`, client, server */

#define AT_FDROOT (-101)
#define FS_REQ_PATH_BUF_LEN (256)
#define FS_REQ_PATH_LEN (255)
#define FS_BUF_SIZE (IPC_SHM_AVAILABLE - sizeof(struct fs_request))

/* IPC request type for fs */
enum fs_req_type {
	FS_REQ_UNDEFINED = 0,

	FS_REQ_OPEN,
	FS_REQ_CLOSE,
	FS_REQ_CREAT,
	FS_REQ_MKDIR,
	FS_REQ_RMDIR,
	FS_REQ_UNLINK,
	FS_REQ_READ,
	FS_REQ_WRITE,
	FS_REQ_GET_SIZE,
	FS_REQ_LSEEK,
	FS_REQ_GETDENTS64,
	FS_REQ_MOUNT,
	FS_REQ_UMOUNT,
	FS_REQ_GET_FS_CAP
};

// /* Client send fsm_req to FSM */
// enum fsm_req_type {
// 	FSM_REQ_UNDEFINED = 0,
// 
// 	FSM_REQ_PARSE_PATH,
// 	FSM_REQ_MOUNT,
// 	FSM_REQ_UMOUNT,
// 
// 	FSM_REQ_SYNC,
// 	/*
// 	 * Since procmgr is booted after fsm and fsm needs to send IPCs to procmgr,
// 	 * procmgr will issue the following IPC to connect itself with fsm.
// 	 */
// 	FSM_REQ_CONNECT_PROCMGR_AND_FSM,
// };

#define FS_READ_BUF_SIZE (IPC_SHM_AVAILABLE - (u64)(&(((fs_request *)(0))->read_buff_begin)))
#define FS_WRITE_BUF_SIZE (IPC_SHM_AVAILABLE - (u64)(&(((fs_request *)(0))->write->write_buff_begin)))

/* Clients send fs_request to fs_server */
struct fs_request {
	enum fs_req_type req;
	union {
		struct {
			int paritition;
			unsigned long offset;
			char fs_path[FS_REQ_PATH_BUF_LEN];
			char mount_path[FS_REQ_PATH_BUF_LEN];
		} mount;
		struct {
			char pathname[FS_REQ_PATH_BUF_LEN];
		} getfscap;
		struct {
			int new_fd;
			char pathname[FS_REQ_PATH_BUF_LEN];
			int flags;
			unsigned int mode;
			int fid;
		} open;
		struct {
			char pathname[FS_REQ_PATH_BUF_LEN];
			unsigned int mode;
		} creat;
		struct {
			int fd;
		} close;
		struct {
			int fd;
			size_t count;
		} read;
		struct {
			int fd;
			size_t count;
			char write_buff_begin;
		} write;
		struct {
			int fd;
			unsigned long offset;
			int whence;
		} lseek;
		struct {
			char pathname[FS_REQ_PATH_BUF_LEN];
			unsigned int mode;
		} mkdir;
		struct {
			char pathname[FS_REQ_PATH_BUF_LEN];
			int flags;
		} unlink;
		struct {
			char pathname[FS_REQ_PATH_BUF_LEN];
			int flags;
		} rmdir;	
		struct {
			int fd;
			size_t count;
		} getdents64;
		struct {
			char pathname[FS_REQ_PATH_BUF_LEN];
		} getsize;
	};
};

// struct fsm_request {
// 	/* Request Type */
// 	enum fsm_req_type req;
// 
// 	/* Arguments */
// 	// Means `path to parse` in normal cases. `device_name` for FSM_REQ_MOUNT/UMOUNT
// 	char path[FS_REQ_PATH_BUF_LEN];
// 	int path_len;
// 
// 	/* Arguments or Response */
// 	// As arguements when FSM_REQ_MOUNT/UMOUNT, as reponse when FSM_REQ_PARSE_PATH
// 	char mount_path[FS_REQ_PATH_BUF_LEN];
// 	int mount_path_len;
// 
// 	/* Response */
// 	int mount_id;
// 	int new_cap_flag;
// };

typedef unsigned long int ino_t;
typedef long int off_t;
typedef unsigned int mode_t;


#ifdef __cplusplus
}
#endif
