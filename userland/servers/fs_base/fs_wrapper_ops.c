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
#include <chcore/types.h>
#include "fs_wrapper_defs.h"
#include "fs_vnode.h"
#include <chcore/fs/defs.h>
#include <libc/sys/stat.h>
#include <chcore/fs/defs.h>
#include "fs_debug.h"

char *strdup(const char *s)
{
	size_t l = strlen(s);
	char *d = malloc(l+1);
	if (!d) return NULL;
	memcpy(d, s, l+1);
	return d;
}

/* Return true if fd is NOT valid */
static inline bool fd_type_invalid(int fd, bool isfile)
{
	if (fd < 0 || fd >= MAX_SERVER_ENTRY_NUM)
		return true;
	if (server_entrys[fd] == NULL)
		return true;
	if (isfile && (server_entrys[fd]->vnode->type != FS_NODE_REG))
		return true;
	if (!isfile && (server_entrys[fd]->vnode->type != FS_NODE_DIR))
		return true;
	return false;
}

/* Default server operation: do nothing, just print error info and return -1 */
int default_server_operation(struct ipc_msg *ipc_msg, struct fs_request *fr)
{
	printf("[fs server] operation %d is not defined\n", fr->req);
	return -1;
}

int fs_wrapper_open(u64 client_badge, struct ipc_msg *ipc_msg, struct fs_request *fr)
{
	int new_fd;
	char *path;
	int flags;
	int mode;
	int entry_id;
	int ret;

	unsigned long vnode_id;
	int vnode_type;
	size_t vnode_size;
	void *vnode_private;

	struct fs_vnode *vnode;

	unsigned long long entry_off;

	/* Prase arguments from fr */
	new_fd = fr->open.new_fd; /* Store fr->fd (newly generated client fd) to new_fd temporarly */
	path = fr->open.pathname;
	flags = fr->open.flags;
	mode = fr->open.mode;


	fr->open.new_fd = AT_FDROOT;
	ret = server_ops.open(path, flags, mode, &vnode_id, &vnode_size, &vnode_type, &vnode_private);
	if (ret != 0) {
		fs_debug_error("ret = %d\n", ret);
		return ret;
	}

	entry_id = alloc_entry();
	if (entry_id < 0) {
		server_ops.close(vnode_private, (vnode_type == FS_NODE_DIR));
		return -EMFILE;
	}

	vnode = get_fs_vnode_by_id(vnode_id);
	if (NULL != vnode) {
		/* Update vnode and entry */
		inc_ref_fs_vnode(vnode);
		assign_entry(server_entrys[entry_id], flags, entry_off, (void *)strdup(path), vnode);
	} else {
		vnode = alloc_fs_vnode(vnode_id, vnode_type, vnode_size, vnode_private);
		list_append(&vnode->node, &fs_vnode_list);
		assign_entry(server_entrys[entry_id], flags, entry_off, (void *)strdup(path), vnode);
	}
	/* After server handling the open request, mapping new_fd to fid */
	fs_wrapper_set_server_entry(client_badge, new_fd, entry_id);


	return new_fd;
}

int fs_wrapper_close(struct ipc_msg *ipc_msg, struct fs_request *fr)
{
	int fd;
	struct fs_vnode *vnode;
	int ret;

	/* Parsing and check arguments */
	fd = fr->close.fd;
	if (fd_type_invalid(fd, true) && fd_type_invalid(fd, false)) {
		// fs_debug_error("fd_type_invalid\n");
		return -ENOENT;
	}

	vnode = server_entrys[fd]->vnode;

	/* Do operator's close */
	ret = server_ops.close(vnode->private, (vnode->type == FS_NODE_DIR));
	if (ret) {
		return ret;
	}
	free_entry(fd);

	/* Deref vnode */
	dec_ref_fs_vnode(vnode);

	/* Revoke vnode, if refcnt == 0 */
	if (vnode->refcnt == 0) {
		pop_free_fs_vnode(vnode);
	}

	return 0;
}

int fs_wrapper_read(struct ipc_msg *ipc_msg, struct fs_request *fr)
{
	int fd;
	char *buf;
	unsigned long long offset;
	size_t size;
	void *operator;
	int ret;
	struct fs_vnode *vnode;
	char *page_buf;
	int fptr, page_idx, page_off, copy_size;

	ret = 0;
	fd = fr->read.fd;
	buf = (void *)fr;

	size = (size_t)fr->read.count;
	offset = (unsigned long long)server_entrys[fd]->offset;
	vnode = server_entrys[fd]->vnode;
	operator = server_entrys[fd]->vnode->private;


	/*
	* If offset is already outside the file,
	*      do nothing and return 0
	*/
	if (offset >= server_entrys[fd]->vnode->size) {
		goto out;
	}

	/*
	* If offset + size > file_size,
	* 	change size to (file_size - offset).
	*/
	if (offset + size > server_entrys[fd]->vnode->size) {
		size = server_entrys[fd]->vnode->size - offset;
	}

	/*
	* Server-side read operation should implement like:
	* - Base: read file from `offset` for `size` length,
	*      if it touch a file ending, return content from offset to end
	*      and return bytes read.
	*/
	ret = server_ops.read(operator, offset, size, buf);

	/* Update server_entry and vnode metadata */
	server_entrys[fd]->offset += ret;
out:
	return ret;
}

int fs_wrapper_write(struct ipc_msg *ipc_msg, struct fs_request *fr)
{
	int fd;
	char *buf;
	size_t size;
	unsigned long long offset;
	void *operator;
	int ret;
	struct fs_vnode *vnode;
	char *block_buf;
	int fptr, page_idx, block_idx, block_off, copy_size;

	ret = 0;
	fd = fr->write.fd;
	buf = (void *)fr + sizeof(struct fs_request); // fixed + 8 bug; original bug without +8

	size = (size_t)fr->write.count;
	offset = (unsigned long long)server_entrys[fd]->offset;
	vnode = server_entrys[fd]->vnode;
	operator = server_entrys[fd]->vnode->private;

	/*
	* If size == 0, do nothing and return 0
	* Even the offset is outside of the file, inode size is not changed!
	*/
	if (size == 0) {
		goto out;
	}

	/*
	* Server-side write operation should implement like:
	* - Base: write file and return bytes written
	* - If offset is outside the file (notice size=0 is handled)
	*      Filling '\0' until offset pos, then append file
	*/

	ret = server_ops.write(operator, offset, size, buf);

	/* Update server_entry and vnode metadata */
	server_entrys[fd]->offset += ret;
	if (server_entrys[fd]->offset > server_entrys[fd]->vnode->size) {
		server_entrys[fd]->vnode->size = server_entrys[fd]->offset;
	}
out:
	return ret;
}

int fs_wrapper_lseek(struct ipc_msg *ipc_msg, struct fs_request *fr)
{
	int fd;
	unsigned long long offset;
	int whence;
	unsigned long long target_off;

	fd = fr->lseek.fd;
	offset = fr->lseek.offset;
	whence = fr->lseek.whence;

	switch (whence) {
	case SEEK_SET: {
		target_off = offset;
		break;
	}
	case SEEK_CUR: {
		target_off = server_entrys[fd]->offset + offset;
		break;
	}
	case SEEK_END:
		target_off = server_entrys[fd]->vnode->size + offset;
		break;
	default: {
		printf("%s: %d Not impelemented yet\n", __func__, whence);
		target_off = -1;
		break;
	}
	}
	if (target_off < 0)
		return -EINVAL;

	server_entrys[fd]->offset = target_off;

	return target_off;
}

int fs_wrapper_unlink(struct ipc_msg *ipc_msg, struct fs_request *fr)
{
	char *path = fr->unlink.pathname;
	int flags = fr->unlink.flags;
	int ret;
	struct fs_vnode *vnode;

	ret = server_ops.unlink(path, flags);

	return ret;
}

int fs_wrapper_rmdir(struct ipc_msg *ipc_msg, struct fs_request *fr)
{
	char *path = fr->rmdir.pathname;
	int flags = fr->rmdir.flags;

	return server_ops.rmdir(path, flags);
}

int fs_wrapper_mkdir(struct ipc_msg *ipc_msg, struct fs_request *fr)
{
	int ret;

	const char *path = fr->mkdir.pathname;
	unsigned int mode = fr->mkdir.mode;

	ret = server_ops.mkdir(path, mode);
	return ret;
}

int fs_wrapper_creat(struct ipc_msg *ipc_msg, struct fs_request *fr)
{
	return server_ops.creat(ipc_msg, fr);
}

int fs_wrapper_getdents64(struct ipc_msg *ipc_msg, struct fs_request *fr)
{
	return server_ops.getdents64(ipc_msg, fr);
}

int fs_wrapper_get_size(struct ipc_msg *ipc_msg, struct fs_request *fr) {
	return server_ops.getsize(fr->getsize.pathname);
}
