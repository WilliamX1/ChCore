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

#include "lab5_stdio.h"


extern struct ipc_struct *tmpfs_ipc_struct;

/* You could add new functions or include headers here.*/
/* LAB 5 TODO BEGIN */

int alloc_id()
{
	static int fd = 0;
	return ++fd;
};



/* LAB 5 TODO END */


FILE *fopen(const char * filename, const char * mode) {

	/* LAB 5 TODO BEGIN */
	ipc_msg_t* ipc_msg;
	int ret;
	struct fs_request* fr_ptr;
	int fd = alloc_id();

begin:
	ipc_msg = ipc_create_msg(tmpfs_ipc_struct, sizeof(struct fs_request), 1);
	fr_ptr = (struct fs_request *) ipc_get_msg_data(ipc_msg);
	fr_ptr->req = FS_REQ_OPEN;
	fr_ptr->open.new_fd = fd;
	fr_ptr->open.mode = mode;

	if (strlen(filename) == 0)
		strcpy((void *) fr_ptr->open.pathname, "/");
	else if (*filename != '/') {
		fr_ptr->open.pathname[0] = '/';
		strcpy((void *) (fr_ptr->open.pathname + 1), filename);
	} else {
		strcpy((void *) fr_ptr->open.pathname, filename);
	};

	ret = ipc_call(tmpfs_ipc_struct, ipc_msg);
	ipc_destroy_msg(tmpfs_ipc_struct, ipc_msg);

	if (ret < 0)
		if (mode == 'r')
			goto error;
		else {
			ipc_msg = ipc_create_msg(tmpfs_ipc_struct, sizeof(struct fs_request), 1);
			fr_ptr = (struct fs_request *) ipc_get_msg_data(ipc_msg);
			fr_ptr->req = FS_REQ_CREAT;
			fr_ptr->creat.mode = mode;

			if (strlen(filename) == 0)
				strcpy((void *) fr_ptr->creat.pathname, "/");
			else if (*filename != '/') {
				fr_ptr->creat.pathname[0] = '/';
				strcpy((void *) (fr_ptr->creat.pathname + 1), filename);
			} else {
				strcpy((void *) fr_ptr->creat.pathname, filename);
			};

			ret = ipc_call(tmpfs_ipc_struct, ipc_msg);
			ipc_destroy_msg(tmpfs_ipc_struct, ipc_msg);
			if (ret < 0)
				goto error;
			goto begin;
		}
	FILE* file = malloc(sizeof(struct FILE));
	file->fd = fd;
	strcpy(file->filename, filename);
	file->mode = mode;
	file->offset = 0;
	// file->refcnt = 1;
	
	/* LAB 5 TODO END */
error:
    return file;
}

size_t fwrite(const void * src, size_t size, size_t nmemb, FILE * f) {

	/* LAB 5 TODO BEGIN */
	ipc_msg_t* ipc_msg;
	int ret;
	struct fs_request* fr_ptr;
	int fd = f->fd;

	ipc_msg = ipc_create_msg(tmpfs_ipc_struct, sizeof(struct fs_request), 1);
	fr_ptr = (struct fs_request *) ipc_get_msg_data(ipc_msg);
	fr_ptr->req = FS_REQ_WRITE;
	fr_ptr->write.count = nmemb;
	fr_ptr->write.fd = fd;

	memcpy((void *) fr_ptr + sizeof(struct fs_request), src, nmemb);

	ret = ipc_call(tmpfs_ipc_struct, ipc_msg);
	if (ret < 0)
		goto error;
	
	f->offset += ret;

	/* LAB 5 TODO END */
error:
	ipc_destroy_msg(tmpfs_ipc_struct, ipc_msg);
    return ret;
}

size_t fread(void * destv, size_t size, size_t nmemb, FILE * f) {

	/* LAB 5 TODO BEGIN */
	ipc_msg_t* ipc_msg;
	int ret;
	struct fs_request* fr_ptr;

	int fd = f->fd;
	ipc_msg = ipc_create_msg(tmpfs_ipc_struct, sizeof(struct fs_request), 1);
	fr_ptr = (struct fs_request *) ipc_get_msg_data(ipc_msg);
	fr_ptr->req = FS_REQ_READ;
	fr_ptr->read.fd = fd;
	fr_ptr->read.count = nmemb;

	ret = ipc_call(tmpfs_ipc_struct, ipc_msg);
	ipc_destroy_msg(tmpfs_ipc_struct, ipc_msg);
	if (ret <= 0)
		goto error;
	
	memcpy(destv, ipc_get_msg_data(ipc_msg), ret);

	f->offset += ret;
	
	/* LAB 5 TODO END */
error:
    return ret;
}

int fclose(FILE *f) {

	/* LAB 5 TODO BEGIN */
	free(f);
	/* LAB 5 TODO END */
    return 0;

}

/* Need to support %s and %d. */
int fscanf(FILE * f, const char * fmt, ...) {

	/* LAB 5 TODO BEGIN */

	/* LAB 5 TODO END */
    return 0;
}

/* Need to support %s and %d. */
int fprintf(FILE * f, const char * fmt, ...) {

	/* LAB 5 TODO BEGIN */

	/* LAB 5 TODO END */
    return 0;
}

