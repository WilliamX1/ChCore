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

#include "cpio.h"
#include <chcore/fs/defs.h>
#include <chcore/ipc.h>
#include <chcore/memory.h>
#include <chcore/assert.h>
#include <stdio.h>

#include "../fs_base/fs_wrapper_defs.h"
#include "../fs_base/fs_vnode.h"
#include "tmpfs.h"
#include "tmpfs_ops.h"

struct inode *tmpfs_root = NULL;
struct dentry *tmpfs_root_dent = NULL;
struct id_manager fidman;
struct fid_record fid_records[MAX_NR_FID_RECORDS];
struct server_entry *server_entrys[MAX_SERVER_ENTRY_NUM];
struct list_head fs_vnode_list;
bool mounted;

/* previous definition of some function */
struct dentry *tfs_lookup(struct inode *dir, const char *name, size_t len);

/*
 * Helper functions to calucate hash value of string
 */
static inline u64 hash_chars(const char *str, ssize_t len)
{
	u64 seed = 131;		/* 31 131 1313 13131 131313 etc.. */
	u64 hash = 0;
	int i;

	if (len < 0) {
		while (*str) {
			hash = (hash * seed) + *str;
			str++;
		}
	} else {
		for (i = 0; i < len; ++i)
			hash = (hash * seed) + str[i];
	}

	return hash;
}

/* BKDR hash */
static inline u64 hash_string(struct string *s)
{
	return (s->hash = hash_chars(s->str, s->len));
}

static inline int init_string(struct string *s, const char *name, size_t len)
{
	int i;

	s->str = malloc(len + 1);
	if (!s->str)
		return -ENOMEM;
	s->len = len;

	for (i = 0; i < len; ++i)
		s->str[i] = name[i];
	s->str[len] = '\0';

	hash_string(s);
	return 0;
}

/*
 *  Helper functions to create instances of key structures
 */
static inline struct inode *new_inode(void)
{
	struct inode *inode = malloc(sizeof(*inode));

	if (!inode)
		return ERR_PTR(-ENOMEM);

	inode->type = 0;
	inode->size = 0;

	return inode;
}

struct inode *new_dir(void)
{
	struct inode *inode;

	inode = new_inode();
	if (IS_ERR(inode))
		return inode;
	inode->type = FS_DIR;
	init_htable(&inode->dentries, 1024);

	return inode;
}

static struct inode *new_reg(void)
{
	struct inode *inode;

	inode = new_inode();
	if (IS_ERR(inode))
		return inode;
	inode->type = FS_REG;
	init_radix_w_deleter(&inode->data, free);

	return inode;
}

struct dentry *new_dent(struct inode *inode, const char *name,
			       size_t len)
{
	struct dentry *dent;
	int err;

	dent = malloc(sizeof(*dent));
	if (!dent)
		return ERR_PTR(-ENOMEM);
	err = init_string(&dent->name, name, len);
	if (err) {
		free(dent);
		return ERR_PTR(err);
	}
	dent->inode = inode;

	return dent;
}

// this function create a file (directory if `mkdir` == true, otherwise regular
// file) and its size is `len`. You should create an inode and corresponding 
// dentry, then add dentey to `dir`'s htable by `htable_add`.
// Assume that no separator ('/') in `name`.
static int tfs_mknod(struct inode *dir, const char *name, size_t len, int mkdir)
{
	struct inode *inode;
	struct dentry *dent;

	BUG_ON(!name);

	if (len == 0) {
		WARN("mknod with len of 0");
		return -ENOENT;
	}
	/* LAB 5 TODO BEGIN */
	if (tfs_lookup(dir, name, len) != NULL)
		return -EEXIST;
	/* init inode* */
	inode = mkdir ? new_dir() : new_reg();
	if (IS_ERR(inode))
		return -ENOMEM;
	/* init dent* using inode and other args */
	dent = new_dent(inode, name, len);
	if (IS_ERR(dent)) {
		free(inode);
		return -ENOMEM;
	};
	/* add to htable */
	init_hlist_node(&dent->node);
	htable_add(&(dir->dentries), dent->name.hash, &dent->node);

	/* LAB 5 TODO END */

	return 0;
}

int tfs_mkdir(struct inode *dir, const char *name, size_t len)
{
	return tfs_mknod(dir, name, len, 1 /* mkdir */ );
}

int tfs_creat(struct inode *dir, const char *name, size_t len)
{
	return tfs_mknod(dir, name, len, 0 /* mkdir */ );
}

// look up a file called `name` under the inode `dir` 
// and return the dentry of this file
struct dentry *tfs_lookup(struct inode *dir, const char *name,
				 size_t len)
{
	u64 hash = hash_chars(name, len);
	struct dentry *dent;
	struct hlist_head *head;

	head = htable_get_bucket(&dir->dentries, (u32) hash);

	for_each_in_hlist(dent, node, head) {
		if (dent->name.len == len && 0 == strcmp(dent->name.str, name))
			return dent;
	}
	return NULL;
}

// Walk the file system structure to locate a file with the pathname stored in `*name`
// and saves parent dir to `*dirat` and the filename to `*name`.
// If `mkdir_p` is true, you need to create intermediate directories when it missing.
// If the pathname `*name` starts with '/', then lookup starts from `tmpfs_root`, 
// else from `*dirat`.
// Note that when `*name` ends with '/', the inode of last component will be
// saved in `*dirat` regardless of its type (e.g., even when it's FS_REG) and
// `*name` will point to '\0'
int tfs_namex(struct inode **dirat, const char **name, int mkdir_p)
{
	BUG_ON(dirat == NULL);
	BUG_ON(name == NULL);
	BUG_ON(*name == NULL);

	char buff[MAX_FILENAME_LEN + 1];
	int i;
	struct dentry *dent;
	int err;

	if (**name == '/') {
		*dirat = tmpfs_root;
		// make sure `name` starts with actual name
		while (**name && **name == '/')
			++(*name);
	} else {
		BUG_ON(*dirat == NULL);
		BUG_ON((*dirat)->type != FS_DIR);
	}

	// make sure a child name exists
	if (!**name)
		return -EINVAL;

	// `tfs_lookup` and `tfs_mkdir` are useful here

	/* LAB 5 TODO BEGIN */
	/* extract first segment of name */
	for (i = 0; i < MAX_FILENAME_LEN; i++) {
		if (!(*name)[i] || (*name)[i] == '/') {
			buff[i] = '\0';
			break;
		};
		buff[i] = (*name)[i];
	};
	/* get dentry under parent dir "dirat" */
	dent = tfs_lookup(*dirat, buff, i);
	if (!(*name)[i]) { // at the and, *name = filename, we should check whether file exists finally
		return dent ? 0 : -ENOENT;
	};
	if (!dent) {	// not exist, create or quit
		if (mkdir_p) {
			err = tfs_mkdir(*dirat, buff, i);
			if (err < 0)
				return err;
			dent = tfs_lookup(*dirat, buff, i);
		} else {
			return -ENOENT;
		};
	};
	/* set name and recursion */
	(*name) += i;
	if (**name) { // name still not to the end
		*dirat = dent->inode;
		while ((**name) == '/') {
			(*name)++;
		};
		if (**name) { // if a child name exists, recursion
			return tfs_namex(dirat, name, mkdir_p);
		} else { // name == '\0', whichi means origin name endsup with '/'
			return 0;
		};
	};

	if (!**name)
		return -EINVAL;
	/* LAB 5 TODO END */

	/* we will never reach here? */
	return 0;
}

int tfs_remove(struct inode *dir, const char *name, size_t len)
{
	u64 hash = hash_chars(name, len);
	struct dentry *dent, *target = NULL;
	struct hlist_head *head;

	BUG_ON(!name);

	if (len == 0) {
		WARN("mknod with len of 0");
		return -ENOENT;
	}

	head = htable_get_bucket(&dir->dentries, (u32) hash);

	for_each_in_hlist(dent, node, head) {
		if (dent->name.len == len && 0 == strcmp(dent->name.str, name)) {
			target = dent;
			break;
		}
	}

	if (!target)
		return -ENOENT;

	BUG_ON(!target->inode);

	// remove only when file is closed by all processes
	if (target->inode->type == FS_REG) {
		// free radix tree
		radix_free(&target->inode->data);
		// free inode
		free(target->inode);
		// remove dentry from parent
		htable_del(&target->node);
		// free dentry
		free(target);
	} else if (target->inode->type == FS_DIR) {
		if (!htable_empty(&target->inode->dentries))
			return -ENOTEMPTY;

		// free htable
		htable_free(&target->inode->dentries);
		// free inode
		free(target->inode);
		// remove dentry from parent
		htable_del(&target->node);
		// free dentry
		free(target);
	} else {
		BUG("inode type that shall not exist");
	}

	return 0;
}

// write memory into `inode` at `offset` from `buf` for length is `size`
// it may resize the file
// `radix_get`, `radix_add` are used in this function
ssize_t tfs_file_write(struct inode * inode, off_t offset, const char *data,
		       size_t size)
{
	BUG_ON(inode->type != FS_REG);
	BUG_ON(offset > inode->size);

	u64 page_no, page_off;
	u64 cur_off = offset;
	size_t to_write;
	void *page;

	/* LAB 5 TODO BEGIN */
	u64 origin_total_page_num = ROUND_UP(inode->size, PAGE_SIZE) / PAGE_SIZE;
	size_t cur_to_write;
	for (to_write = size; to_write > 0; ) {
		page_no = ROUND_DOWN(cur_off, PAGE_SIZE) / PAGE_SIZE;
		if (origin_total_page_num == 0 || page_no >= origin_total_page_num) { // create a new page
			page = malloc(PAGE_SIZE);
			if (!page) 
				goto fail;
			if (radix_add(&inode->data, page_no * PAGE_SIZE, page) < 0) //radix tree key is offset(interger)
				goto fail;
		};
		page = radix_get(&inode->data, page_no * PAGE_SIZE);
		if (!page)
			goto fail;
		page_off = cur_off % PAGE_SIZE;
		cur_to_write = MIN(PAGE_SIZE - page_off, to_write);
		/* copy data to memory */
		memcpy((char*) page + page_off, data, cur_to_write);
		/* update variables in pne loop */
		to_write -= cur_to_write;
		cur_off += cur_to_write;
		data += cur_to_write;
	};
	if (inode->size < cur_off)
		inode->size = cur_off;
	/* LAB 5 TODO END */
fail:
	return cur_off - offset;
}

// read memory from `inode` at `offset` in to `buf` for length is `size`, do not
// exceed the file size
// `radix_get` is used in this function
// You can use memory functions defined in libc
ssize_t tfs_file_read(struct inode * inode, off_t offset, char *buff,
		      size_t size)
{
	BUG_ON(inode->type != FS_REG);
	BUG_ON(offset > inode->size);

	u64 page_no, page_off;
	u64 cur_off = offset;
	size_t to_read;
	void *page;

	/* LAB 5 TODO BEGIN */
	size_t cur_to_read;
	to_read = MIN(inode->size - offset, size); // caution: read out of the file
	while (to_read > 0) {
		page_no = ROUND_DOWN(offset, PAGE_SIZE) / PAGE_SIZE;
		page = radix_get(&inode->data, page_no * PAGE_SIZE);
		if (!page)
			goto fail;
		page_off = cur_off % PAGE_SIZE;
		cur_to_read = MIN(PAGE_SIZE - page_off, to_read);
		/* copy data to buff */
		memcpy(buff, (char*) page + page_off, cur_to_read);
		/* update variables in one loop */
		to_read -= cur_to_read;
		cur_off += cur_to_read;
		buff += cur_to_read;
	};
	/* LAB 5 TODO END */
fail:
	return cur_off - offset;
}

// load the cpio archive into tmpfs with the begin address as `start` in memory
// You need to create directories and files if necessary. You also need to write
// the data into the tmpfs.
int tfs_load_image(const char *start)
{
	struct cpio_file *f;
	struct inode *dirat;
	struct dentry *dent;
	const char *leaf;
	size_t len;
	int err;
	ssize_t write_count;

	BUG_ON(start == NULL);

	cpio_init_g_files();
	cpio_extract(start, "/");

	for (f = g_files.head.next; f; f = f->next) {
		/* LAB 5 TODO BEGIN */
		dirat = tmpfs_root;
		leaf = f->name;
		err = tfs_namex(&dirat, &leaf, 0);
		if (err < 0 && err != -ENOENT)
			return err;
		int f_type = f->header.c_mode & CPIO_FT_MASK;
		if (err == -ENOENT) {
			switch (f_type)
			{
			case CPIO_REG:
				err = tfs_creat(dirat, leaf, strlen(leaf));
				break;
			case CPIO_DIR:
				err = tfs_mkdir(dirat, leaf, strlen(leaf));
				break;
			default:
				err = -EPFNOSUPPORT;
				break;
			};
			if (err < 0)
				return err;
		};
		dent = tfs_lookup(dirat, leaf, strlen(leaf));
		if (f_type == CPIO_REG) {
			err = tfs_file_write(dent->inode, 0, f->data, f->header.c_filesize);
			BUG_ON(err != f->header.c_filesize);
			if (err < 0)
				return err;
		};
		/* LAB 5 TODO END */
	}

	return 0;
}

static int dirent_filler(void **dirpp, void *end, char *name, off_t off,
			 unsigned char type, ino_t ino)
{
	struct dirent *dirp = *(struct dirent **)dirpp;
	void *p = dirp;
	unsigned short len = strlen(name) + 1 +
	    sizeof(dirp->d_ino) +
	    sizeof(dirp->d_off) + sizeof(dirp->d_reclen) + sizeof(dirp->d_type);
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

/* path[0] must be '/' */
struct inode *tfs_open_path(const char *path)
{
	struct inode *dirat = NULL;
	const char *leaf = path;
	struct dentry *dent;
	int err;

	if (*path == '/' && !*(path + 1))
		return tmpfs_root;

	err = tfs_namex(&dirat, &leaf, 0);
	if (err)
		return NULL;

	dent = tfs_lookup(dirat, leaf, strlen(leaf));
	return dent ? dent->inode : NULL;
}

int del_inode(struct inode *inode)
{
	if (inode->type == FS_REG) {
		// free radix tree
		radix_free(&inode->data);
		// free inode
		free(inode);
	} else if (inode->type == FS_DIR) {
		if (!htable_empty(&inode->dentries))
			return -ENOTEMPTY;
		// free htable
		htable_free(&inode->dentries);
		// free inode
		free(inode);
	} else {
		BUG("inode type that shall not exist");
	}
	return 0;
}


int init_tmpfs(void)
{
	tmpfs_root = new_dir();
	tmpfs_root_dent = new_dent(tmpfs_root, "/", 1);

	init_id_manager(&fidman, MAX_NR_FID_RECORDS, DEFAULT_INIT_ID);
	/**
	 * Allocate the first id, which should be 0.
	 * No request should use 0 as the fid.
	 */
	chcore_assert(alloc_id(&fidman) == 0);

	init_fs_wrapper();

	mounted = true;

	return 0;
}

