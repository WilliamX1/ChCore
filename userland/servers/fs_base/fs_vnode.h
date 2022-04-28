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

#include <chcore/assert.h>
#include <sys/types.h>
#include <chcore/fs/list.h>
#include <chcore/fs/hashtable.h>
#include <chcore/fs/defs.h>
#include <sync/spin.h>

#define MAX_FILE_PAGES 512
#define MAX_SERVER_ENTRY_NUM 1024

enum fs_vnode_type {
	FS_NODE_RESERVED = 0,
	FS_NODE_REG,
	FS_NODE_DIR
};

/*
 * per-inode
 */
#define PC_HASH_SIZE 512
struct fs_vnode {
	unsigned long vnode_id;				/* identifier */

	enum fs_vnode_type type;		/* regular or directory */
	int refcnt;				/* reference count */
	size_t size;				/* file size or directory entry number */
	struct page_cache_entity_of_inode *page_cache;
	int pmo_cap;				/* fmap fault is handled by this */
	void *private;

	struct list_head node;			/* for linked list */
};

/*
 * per-fd
 */
struct server_entry {
	/* `flags` and `offset` is assigned to each fd */
	u64 flags;
	unsigned long long offset;

	/* Different FS may use different struct to store path, normally `char*` */
	void *path;

	/* Each vnode is binding with a disk inode */
	struct fs_vnode *vnode;
};

extern struct server_entry *server_entrys[MAX_SERVER_ENTRY_NUM];

extern void free_entry(int entry_idx);
extern int alloc_entry();
extern void assign_entry(struct server_entry *e, u64 f, unsigned long long o, void *p, struct fs_vnode *n);

/* fs_vnode pool */
extern struct list_head fs_vnode_list;

extern void fs_vnode_init();
extern struct fs_vnode *alloc_fs_vnode(unsigned long id, enum fs_vnode_type type,
						size_t size, void *private);
extern void push_fs_vnode(struct fs_vnode *n);
extern void pop_free_fs_vnode(struct fs_vnode *n);
extern struct fs_vnode *get_fs_vnode_by_id(unsigned long vnode_id);

/* refcnt for vnode */
static inline void inc_ref_fs_vnode(struct fs_vnode *n)
{
	n->refcnt++;
}

static inline void dec_ref_fs_vnode(struct fs_vnode *n)
{
	n->refcnt--;
	chcore_assert(n->refcnt >= 0);
}
