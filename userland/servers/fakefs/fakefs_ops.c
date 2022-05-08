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

#include <chcore/fs/defs.h>
#include <chcore/ipc.h>
#include <chcore/memory.h>
#include <chcore/assert.h>
#include <stdio.h>
#include <string.h>
#include <libc/sys/stat.h>
#include <chcore/fs/defs.h>
#include <chcore/assert.h>
#include "fakefs.h"

extern struct list_head fakefs_files;
extern struct spinlock fs_lock;
extern struct server_entry *server_entrys[];
extern struct list_head fs_vnode_list;

void check_path(const char* path) {
	BUG_ON(!path);
	BUG_ON(*path != '/');
	BUG_ON(strlen(path) >= FAKEFS_MAX_PATH_LEN);

}
void init_fakefs_file_node(struct fakefs_file_node *n)
{
	memset(n->path, 0x0, sizeof(n->path));
	n->file = NULL;
	n->refcnt = 0;
	n->offset = 0;
	n->size = 0;
	n->isdir = false;
}

void print_all_file() {
	struct fakefs_file_node *private_iter;
	char* file = NULL;
	for_each_in_list(private_iter, struct fakefs_file_node, node, &fakefs_files) {
		file = private_iter->file ? private_iter->file : "null";
		printf("Path : %s File size %d File content: %s\n", private_iter->path, private_iter->size, file);
	}
}

struct fakefs_file_node * fakefs_lookup_by_path(const char* path) {
	struct fakefs_file_node *private_iter;
	for_each_in_list(private_iter, struct fakefs_file_node, node, &fakefs_files) {
		if (!strcmp(private_iter->path, path)) {
			return private_iter;
		}
	}
	return NULL;
}

int fakefs_creat(struct ipc_msg *ipc_msg, struct fs_request *fr)
{
	// printf("fakefs_ops get create request: path: %s\n", fr->creat.pathname);
	char* path = fr->creat.pathname;
	check_path(path);

	struct fakefs_file_node *file_node;
	file_node = fakefs_lookup_by_path(path);
	BUG_ON(file_node);

	struct fakefs_file_node *n = (struct fakefs_file_node *)malloc(sizeof(struct fakefs_file_node));
	init_fakefs_file_node(n);
	strcpy(n->path, path);

	/* Insert node to fsm_server_entry_mapping */
	list_append(&n->node, &fakefs_files);

	return 0;
}

int fakefs_open(char *path, int flags, int mode, unsigned long *vnode_id, size_t *vnode_size, int *vnode_type, void **vnode_private)
{
	// printf("fakefs_ops get open request: path: %s\n", path);
	check_path(path);

	struct fakefs_file_node *file_node;
	file_node = fakefs_lookup_by_path(path);
	if(!file_node) {
		return -1;
	}

	*vnode_id = (unsigned long)file_node;
	*vnode_type = file_node->isdir ? FS_NODE_DIR: FS_NODE_REG;
	*vnode_size = file_node->size;
	*vnode_private = file_node;
	if(file_node) {
		file_node->refcnt++;
	}

	return 0;
}

int fakefs_close(void *operator, bool is_dir)
{
	// printf("fakefs_ops get close request..\n");
	int ret = 0;
	struct fakefs_file_node *file_node = (struct fakefs_file_node *)operator;
	if(file_node) {
		file_node->refcnt--;
	}
	return 0;
}

int fakefs_unlink(const char *path, int flags)
{
	printf("[fs server] operation fakefs_unlink is not defined\n");
	return -1;
}

int fakefs_symlinkat(struct ipc_msg *ipc_msg, struct fs_request *fr)
{
	printf("[fs server] operation %d is not defined\n", fr->req);
	return -1;
}

int fakefs_read(void *operator, unsigned long offset, size_t size, char *buf)
{
	// printf("fakefs_ops get read request..\n");
	struct fakefs_file_node * file_node = (struct fakefs_file_node *)operator;
	BUG_ON(!file_node);

	/* Returns 0 according to man pages. */
	if (offset >= file_node->size)
		return 0;

	size = MIN(file_node->size - offset, size);
	memcpy(buf, file_node->file + offset, size);
	// printf("\tbuf: %s\n", buf);	
	return size;	
}

int fakefs_write(void *operator, unsigned long offset, size_t size, const char *buf)
{
	// printf("fakefs_ops get write request..\n");
	struct fakefs_file_node * file_node = (struct fakefs_file_node *)operator;
	BUG_ON(!file_node);
	int size_after_write = offset + size;
	if(size_after_write > file_node->size) {
		char* new_file = malloc(size_after_write);
		if(file_node->file != NULL) {
			memcpy(new_file, file_node->file, file_node->size);
			free(file_node->file);
		}
		file_node->file = new_file;
	}
	memcpy(file_node->file + offset, buf, size);	
	file_node->size = size_after_write;
	// printf("\tbuf: %s\n", buf);
	
	return size;
}

void del_file_node(struct fakefs_file_node *del_node) {
	if(del_node->file) {
		free(del_node->file);
	}
	free(del_node);
}

int fakefs_rmdir(const char *path, int flags)
{
	// printf("fakefs_ops get rmdir request, path: %s\n", path);
	struct list_head node_to_del;
	init_list_head(&node_to_del);
	
	int path_len = strlen(path);
	int list_size = 0;
	int i;
	
	struct fakefs_file_node *private_iter;
	for_each_in_list(private_iter, struct fakefs_file_node, node, &fakefs_files) {	
		list_size ++;
	}
	
	struct fakefs_file_node ** del_nodes = malloc(sizeof(*del_nodes) * list_size);
	int nr = 0;

	for_each_in_list(private_iter, struct fakefs_file_node, node, &fakefs_files) {	
		if(memcmp(private_iter->path, path, path_len) == 0
				&& (private_iter->path[path_len] == '/' || private_iter->path[path_len] == '\0')) {
			del_nodes[nr++] = private_iter;
		}
	}
	for(i = 0; i < nr; ++i) {
		list_del(&(del_nodes[i]->node));
		del_file_node(del_nodes[i]);
	}
	free(del_nodes);
	return 0;

}

int fakefs_mkdir(const char *path, mode_t mode)
{
	// printf("fakefs_ops get mkdir request, path: %s\n", path);
	int path_len = strlen(path);
	BUG_ON(path[path_len - 1] == '/');
	check_path(path);
	
	struct fakefs_file_node *file_node;
	file_node = fakefs_lookup_by_path(path);
	BUG_ON(file_node);
	if(file_node) {
	    return -1;
	}

	struct fakefs_file_node *n = (struct fakefs_file_node *)malloc(sizeof(struct fakefs_file_node));
	init_fakefs_file_node(n);
	strcpy(n->path, path);
	n->isdir = true;

	/* Insert node to fsm_server_entry_mapping */
	list_append(&n->node, &fakefs_files);
	// print_all_file();
	return 0;
}

int fakefs_rename(const char *oldpath, const char *newpath)
{
	printf("[fs server] operation fakefs_rename is not defined\n");
	return -1;
}

static int __dirent_filler(void **dirpp, void *end, char *name, unsigned long long off,
			 unsigned char type, unsigned long ino)
{
	struct dirent *dirp = *(struct dirent **)dirpp;
	void *p = dirp;
	unsigned short len = strlen(name) + 1 + sizeof(dirp->d_ino) +
			     sizeof(dirp->d_off) + sizeof(dirp->d_reclen) +
			     sizeof(dirp->d_type);
	p += len;
	if (p > end)
		return -EAGAIN;
	dirp->d_ino = ino;
	dirp->d_off = off;
	dirp->d_reclen = len;
	dirp->d_type = type;
	strcpy(dirp->d_name, name);
	*dirpp = p;
	return len;
}

char* findchar(char* s, char c) {
	while(*s && *s != c) {
		++s;
	}
	return s;
}


int fakefs_getdents(struct ipc_msg *ipc_msg, struct fs_request *fr)
{
	int ret = 0;
	char pathbuf[FAKEFS_MAX_PATH_LEN];
	char dirpathbuf[FAKEFS_MAX_PATH_LEN]; 
	int fd = fr->getdents64.fd;
	int count = fr->getdents64.count;
	struct fakefs_file_node *dir_node = server_entrys[fd]->vnode->private;
	char* dir_path = dir_node->path;
	int dir_path_len = strlen(dir_path);
	if(!dir_node->isdir) {
		return -1;
	}
	memset(dirpathbuf, 0x0, sizeof(dirpathbuf));
	if(dir_path[dir_path_len - 1] != '/') {
		strcpy(dirpathbuf, dir_path);
		dirpathbuf[dir_path_len] = '/';
		dir_path = dirpathbuf;
		dir_path_len += 1;
	}

	// printf("fakefs_ops get gendents request, path: %s\n", dirpathbuf);

	s64 cnt = 0;
	s64 start = server_entrys[fd]->offset;
	void * buf = ipc_get_msg_data(ipc_msg);
	void *p = buf;
	void *end = ipc_get_msg_data(ipc_msg) + count;


	struct fakefs_file_node *private_iter;
	for_each_in_list(private_iter, struct fakefs_file_node, node, &fakefs_files) {	
		char* finds = &(private_iter->path[dir_path_len]);
		char* s = findchar(finds, '/');
		if(memcmp(private_iter->path, dir_path, dir_path_len) == 0 && 
				private_iter != dir_node && *s == '\0') {
			if (cnt >= start) {
				memset(pathbuf, 0x0, sizeof(pathbuf)); 
				memcpy(pathbuf, finds, s - finds);
				ret = __dirent_filler(&p, end, pathbuf, cnt, 0, private_iter->size);
				// printf("Ret of __dirent_filler is %d\n", ret);
				if (ret <= 0) {
					break;
				}
			} 
			cnt ++;
		}
	}
	server_entrys[fd]->offset += cnt - start;
	return p - buf;

}

static struct ipc_struct *procm_ipc_struct = NULL;


void fakefs_test() {
	int procm_cap = __chcore_get_procm_cap();
	printf("procm_cap is %d\n", procm_cap);
	printf("[DEBUG] file %s : line %d : func %s\n", __FILE__, __LINE__, __func__);
	chcore_assert(procm_cap >= 0);
	procm_ipc_struct = ipc_register_client(procm_cap);
	{ // Create test
		struct ipc_msg *ipc_msg = ipc_create_msg(
			procm_ipc_struct, sizeof(struct fs_request) + 256, 0);
		chcore_assert(ipc_msg);
		struct fs_request * fr =
			(struct fs_request *)ipc_get_msg_data(ipc_msg);

		fr->req = FS_REQ_CREAT;
		strcpy(fr->creat.pathname, "/test.txt");
		
		fakefs_creat(ipc_msg, fr);
		
		ipc_destroy_msg(procm_ipc_struct, ipc_msg);
	}

}

int fakefs_getsize (char* path) {
	struct fakefs_file_node *file_node;
	file_node = fakefs_lookup_by_path(path);
	if(!file_node) {
		return -1;
	}
	return file_node->size;
}


struct fs_server_ops server_ops = {
	.open = fakefs_open,
	.read = fakefs_read,
	.write = fakefs_write,
	.close = fakefs_close,
	.creat = fakefs_creat,
	.unlink = fakefs_unlink,
	.mkdir = fakefs_mkdir,
	.rmdir = fakefs_rmdir,
	.getdents64 = fakefs_getdents,
	.getsize = fakefs_getsize,
};