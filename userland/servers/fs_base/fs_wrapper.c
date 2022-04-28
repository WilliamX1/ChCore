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

#include <errno.h>
#include "fs_wrapper_defs.h"
#include "fs_vnode.h"

#include <libc/sys/stat.h>
#include <chcore/fs/defs.h>

/* fs server private data */
struct list_head server_entry_mapping;
struct spinlock fs_wrapper_meta_lock;

int real_file_reader(char *buffer, pidx_t file_page_idx, void *private)
{
	// page_cache_debug("read page %d block %d into %p\n", b, c, a);
	struct fs_vnode *vnode;
	size_t size;
	unsigned long long offset;

	vnode = (struct fs_vnode *)private;

	size = CACHED_PAGE_SIZE;
	offset = file_page_idx * CACHED_PAGE_SIZE;

	/* buffer size should always be PAGE_SIZE. */
	memset(buffer, 0, size);

	if (offset + size > vnode->size)
		size = vnode->size - offset;
	return server_ops.read(vnode->private, offset, size, buffer);
}

int real_file_writer(char *buffer, unsigned long file_page_idx, int page_block_idx, void *private)
{
	struct fs_vnode *vnode;
	unsigned long long offset;
	size_t size;


	vnode = (struct fs_vnode *)private;
	offset = file_page_idx * CACHED_PAGE_SIZE;
	if (page_block_idx == -1) {
		size = CACHED_PAGE_SIZE;
	} else {
		size = CACHED_BLOCK_SIZE;
		offset += page_block_idx * CACHED_BLOCK_SIZE;
	}

	if (offset + size > vnode->size)
		size = vnode->size - offset;
	return server_ops.write(vnode->private, offset, size, buffer);
}

void init_fs_wrapper(void)
{
	/* fs wrapper */
	init_list_head(&server_entry_mapping);
	fs_vnode_init();
	spinlock_init(&fs_wrapper_meta_lock);

}

/* Get (client_badge, fd) -> fid(server_entry) mapping */
int fs_wrapper_get_server_entry(u64 client_badge, int fd)
{
	struct server_entry_node *n;

	/* Stable fd number, need no translating */
	if (fd == AT_FDROOT)
		return AT_FDROOT;

	/* Validate fd */
	BUG_ON(fd < 0 || fd > MAX_SERVER_ENTRY_PER_CLIENT);

	for_each_in_list(n, struct server_entry_node, node, &server_entry_mapping)
		if (n->client_badge == client_badge)
			return n->fd_to_fid[fd];

	return -1;
}

/* Set (client_badge, fd) -> fid(server_entry) mapping */
void fs_wrapper_set_server_entry(u64 client_badge, int fd, int fid)
{

	struct server_entry_node *private_iter;

	/* Validate fd */
	BUG_ON(fd < 0 || fd > MAX_SERVER_ENTRY_PER_CLIENT);

	/* Check if client_badge already involved */
	for_each_in_list(private_iter, struct server_entry_node, node, &server_entry_mapping) {
		if (private_iter->client_badge == client_badge) {
			private_iter->fd_to_fid[fd] = fid;
			return;
		}
	}

	/* New server_entry_node */
	struct server_entry_node *n = (struct server_entry_node *)malloc(sizeof(*n));
	n->client_badge = client_badge;
	int i;
	for (i = 0; i < MAX_SERVER_ENTRY_PER_CLIENT; i++)
		n->fd_to_fid[i] = -1;

	n->fd_to_fid[fd] = fid;

	/* Insert node to server_entry_mapping */
	list_append(&n->node, &server_entry_mapping);
}

/* Translate xxxfd field to fid correspondingly */
void translate_fd_to_fid(u64 client_badge, struct fs_request *fr)
{
	/* Except FS_REQ_OPEN, fd should be translated */
	if (fr->req == FS_REQ_OPEN)
		return;

	switch (fr->req) {
	case FS_REQ_READ:
		fr->read.fd = fs_wrapper_get_server_entry(client_badge, fr->read.fd);
		break;
	case FS_REQ_WRITE:
		fr->write.fd = fs_wrapper_get_server_entry(client_badge, fr->write.fd);
		break;
	case FS_REQ_LSEEK:
		fr->lseek.fd = fs_wrapper_get_server_entry(client_badge, fr->lseek.fd);
		break;
	case FS_REQ_CLOSE:
		fr->close.fd = fs_wrapper_get_server_entry(client_badge, fr->close.fd);
		break;	
	case FS_REQ_GETDENTS64:
		fr->getdents64.fd = fs_wrapper_get_server_entry(client_badge, fr->getdents64.fd);
		break;
	default:
		break;
	}
}

void fs_server_dispatch(struct ipc_msg *ipc_msg, u64 client_badge)
{
	struct fs_request *fr;
	int ret;
	bool ret_with_cap = false;

	fr = (struct fs_request *)ipc_get_msg_data(ipc_msg);

	spinlock_lock(&fs_wrapper_meta_lock);

	/*
	 * Some FS Servers need to complete the initialization process when mounting
	 * eg. Connect with corresponding block device, Save partition offset, etc
	 * So, when the mounted flag is off, requests will be rejected except FS_REQ_MOUNT
	 */
	if (!mounted && (fr->req != FS_REQ_MOUNT)) {
		printf("[fs server] Not fully initialized, send FS_REQ_MOUNT first\n");
		ret = -EINVAL;
		goto out;
	}

	/*
	 * Now fr->fd stores the `Client Side FD Index',
	 * We need to translate fr->fd to fid here, except FS_REQ_OPEN
	 * FS_REQ_OPEN's fr->fd stores the newly generated `Client Side FD Index'
	 * and we should build mapping between fr->fd to fid when handle open request
	 */
	translate_fd_to_fid(client_badge, fr);

	/*
	 * FS Server Requests Handlers
	 */
	switch(fr->req) {

	case FS_REQ_OPEN:
		ret = fs_wrapper_open(client_badge, ipc_msg, fr);
		break;
	case FS_REQ_READ:
		ret = fs_wrapper_read(ipc_msg, fr);
		break;
	case FS_REQ_WRITE:
		ret = fs_wrapper_write(ipc_msg, fr);
		break;
	case FS_REQ_UNLINK:
		ret = fs_wrapper_unlink(ipc_msg, fr);
		break;
	case FS_REQ_RMDIR:
		ret = fs_wrapper_rmdir(ipc_msg, fr);
		break;
	case FS_REQ_MKDIR:
		ret = fs_wrapper_mkdir(ipc_msg, fr);
		break;
	case FS_REQ_CLOSE:
		ret = fs_wrapper_close(ipc_msg, fr);
		break;
	case FS_REQ_CREAT:
		ret = fs_wrapper_creat(ipc_msg, fr);
		break;	
	case FS_REQ_GET_SIZE:
		ret = fs_wrapper_get_size(ipc_msg, fr);
		break;
	case FS_REQ_GETDENTS64:
		ret = fs_wrapper_getdents64(ipc_msg, fr);
		break;
	case FS_REQ_LSEEK: /*LSEEK is handled in fs_wrapper_ops.*/
		ret = fs_wrapper_lseek(ipc_msg, fr);
		break;
	default:
		printf("[Error] Strange FS Server request number %d\n", fr->req);
		ret = -EINVAL;
		break;
	}

out:
	spinlock_unlock(&fs_wrapper_meta_lock);

	ipc_return(ipc_msg, ret);
}
