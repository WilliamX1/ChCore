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

	/* try to open the file */
begin:
	ipc_msg = ipc_create_msg(tmpfs_ipc_struct, sizeof(struct fs_request), 1);
	fr_ptr = (struct fs_request *) ipc_get_msg_data(ipc_msg);
	fr_ptr->req = FS_REQ_OPEN;
	fr_ptr->open.new_fd = fd;
	fr_ptr->open.mode = (unsigned int) mode;

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
		if (*mode == 'r')
			goto error;
		else {
			/* create the target file */
			ipc_msg = ipc_create_msg(tmpfs_ipc_struct, sizeof(struct fs_request), 1);
			fr_ptr = (struct fs_request *) ipc_get_msg_data(ipc_msg);
			fr_ptr->req = FS_REQ_CREAT;
			fr_ptr->creat.mode = (unsigned int) mode;

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
	file->mode = (unsigned int) mode;
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

	/* set the offset */
	ipc_msg = ipc_create_msg(tmpfs_ipc_struct, sizeof(struct fs_request), 1);
	fr_ptr = (struct fs_request *) ipc_get_msg_data(ipc_msg);
	fr_ptr->req = FS_REQ_LSEEK;
	fr_ptr->lseek.fd = fd;
	fr_ptr->lseek.offset = f->offset;
	fr_ptr->lseek.whence = SEEK_SET;
	ret = ipc_call(tmpfs_ipc_struct, ipc_msg);
	ipc_destroy_msg(tmpfs_ipc_struct, ipc_msg);
	if (ret < 0)
		goto error;

	ipc_msg = ipc_create_msg(tmpfs_ipc_struct, sizeof(struct fs_request) + nmemb + 1, 1);
	fr_ptr = (struct fs_request *) ipc_get_msg_data(ipc_msg);
	fr_ptr->req = FS_REQ_WRITE;
	fr_ptr->write.count = nmemb;
	fr_ptr->write.fd = fd;	
	memcpy((void *) fr_ptr + sizeof(struct fs_request), src, nmemb);
	// memcpy((void *) fr_ptr + sizeof(struct fs_request) + 8, src, nmemb); // fixed + 8 bug

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

	/* set the offset */
	ipc_msg = ipc_create_msg(tmpfs_ipc_struct, sizeof(struct fs_request), 1);
	fr_ptr = (struct fs_request *) ipc_get_msg_data(ipc_msg);
	fr_ptr->req = FS_REQ_LSEEK;
	fr_ptr->lseek.fd = f->fd;
	fr_ptr->lseek.offset = f->offset;
	fr_ptr->lseek.whence = SEEK_SET;
	ret = ipc_call(tmpfs_ipc_struct, ipc_msg);
	ipc_destroy_msg(tmpfs_ipc_struct, ipc_msg);
	if (ret < 0)
		goto error;

	int fd = f->fd;
	ipc_msg = ipc_create_msg(tmpfs_ipc_struct, sizeof(struct fs_request), 1);
	fr_ptr = (struct fs_request *) ipc_get_msg_data(ipc_msg);
	fr_ptr->req = FS_REQ_READ;
	fr_ptr->read.fd = fd;
	fr_ptr->read.count = nmemb;

	ret = ipc_call(tmpfs_ipc_struct, ipc_msg);
	if (ret <= 0)
		goto error;
	
	memcpy(destv, ipc_get_msg_data(ipc_msg), ret);

	f->offset += ret;
	
	/* LAB 5 TODO END */
error:
	ipc_destroy_msg(tmpfs_ipc_struct, ipc_msg);
    return ret;
}

int fclose(FILE *f) {

	/* LAB 5 TODO BEGIN */
	ipc_msg_t* ipc_msg;
	int ret;
	struct fs_request* fr_ptr;

	int fd = f->fd;
	ipc_msg = ipc_create_msg(tmpfs_ipc_struct, sizeof(struct fs_request), 1);
	fr_ptr = (struct fs_request *) ipc_get_msg_data(ipc_msg);
	fr_ptr->req = FS_REQ_CLOSE;
	fr_ptr->close.fd = fd;

	ret = ipc_call(tmpfs_ipc_struct, ipc_msg);
	ipc_destroy_msg(tmpfs_ipc_struct, ipc_msg);
	if (ret < 0)
		goto error;
	
	if (--f->refcnt == 0) 
		free(f);
	
	/* LAB 5 TODO END */
error:
    return 0;
}

int double_buf(char* buf, int size) {
	char* new_buf = malloc(sizeof(char) * size);
	memcpy(new_buf, buf, size);
	buf = new_buf;
	size = size * 2;
	return size;
};

/* Need to support %s and %d. */
int fscanf(FILE * f, const char * fmt, ...) {

	/* LAB 5 TODO BEGIN */
	int size = 4096, len = 0, ret;
	char buf[size];

	len = fread(buf, sizeof(char), sizeof(buf), f);

	va_list ap; /* points to each unnamed arg in turn */
	char *p, *sval, *s_dst, *s_src;
	int* d_dst;
    int ival;
	int off = 0, copy_dst, copy_len, number;

	va_start(ap,fmt);   /* make ap point to 1st unnamed arg */

	for (int i = 0; ; i++) {
		const char* p = fmt + i;
		if (!*p) break;

		if (*p == '%') {
			switch (*++p)
			{
			case 'd':
				d_dst = va_arg(ap, int *);

				while (off < size && (buf[off] == ' ' || buf[off] == '\n' 
										|| buf[off] == '\0')) off++;
				if (buf[off] < '0' || buf[off] > '9')
					goto error;

				number = 0;

				while (off < size && '0' <= buf[off] 
						&& buf[off] <= '9') {
					number = (buf[off++] - '0') + number * 10;
				};

				*d_dst = number;
				i++;
				break;
			case 's':
				s_dst = va_arg(ap, char *);
				while (off < size && (buf[off] == ' ' || buf[off] == '\n' 
										|| buf[off] == '\0')) off++;
				
				copy_dst = off;
				while (copy_dst < size && buf[copy_dst] != ' ' 
					&& buf[copy_dst] != '\n' && buf[copy_dst] != '\0') {
					copy_dst++;
				};

				copy_len = copy_dst - off;
				if (copy_len == 0)
					goto error;
				
				s_src = malloc(copy_len * sizeof(char));
				memcpy(s_src, buf + off, copy_len);
				off += copy_len;

				strcpy(s_dst, s_src);
				i++;
				break;
			default:
				break;
			}
		};
	};
	va_end(ap);

	/* LAB 5 TODO END */
error:
    return 0;
}

/* Need to support %s and %d. */
int fprintf(FILE * f, const char * fmt, ...) {

	/* LAB 5 TODO BEGIN */
	int size = 4096, len = 0;
	char* buf = malloc(sizeof(char) * size);

	va_list ap; /* points to each unnamed arg in turn */
	char *p, *sval;
    int ival;

	va_start(ap,fmt);   /* make ap point to 1st unnamed arg */

	for (int i = 0; ; i++) {
		const char* p = fmt + i;
		if (!*p) break;

		if (*p == '%') {
			switch (*++p)
			{
			case 'd':
				ival = va_arg(ap, int);
				char tmp[64];
				int ival_len = 0;
				while (ival > 0) {
					tmp[ival_len++] = (ival % 10) + '0';
					ival /= 10;
				};
				
				if (strlen(tmp) + ival_len >= size)
					size = double_buf(buf, size);
				for (int i = ival_len - 1; i >= 0; i--)
					buf[len++] = tmp[i];
				i++;					
				break;
			case 's':
				for (sval = va_arg(ap, char *); *sval; sval++) {
					if (len >= size)
						size = double_buf(buf, size);
					buf[len++] = *sval;
				};
				i++;
				break;
			default:
				break;
			}
		} else {
			if (len >= size)
				size = double_buf(buf, size);
			buf[len++] = *p;
		};
	};
	va_end(ap);

	buf[len] = '\0';

	fwrite(buf, sizeof(char), len * sizeof(char), f);
	
	/* LAB 5 TODO END */
    return 0;
};