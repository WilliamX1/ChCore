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

#include "fs_vnode.h"
#include "fs_debug.h"

void free_entry(int entry_idx)
{
	free(server_entrys[entry_idx]->path);
	free(server_entrys[entry_idx]);
	server_entrys[entry_idx] = NULL;
}

int alloc_entry()
{
	int i;

	for (i = 0; i < MAX_SERVER_ENTRY_NUM; i++) {
		if (server_entrys[i] == NULL) {
			server_entrys[i] =
				(struct server_entry *)malloc(sizeof(struct server_entry));
			if (server_entrys[i] == NULL)
				return -1;
			return i;
		}
	}
	return -1;
}

void assign_entry(struct server_entry *e, u64 f, unsigned long long o, void *p, struct fs_vnode *n)
{
	e->flags = f;
	e->offset = o;
	e->path = p;
	e->vnode = n;
}

void fs_vnode_init()
{
	init_list_head(&fs_vnode_list);
}

struct fs_vnode *alloc_fs_vnode(unsigned long id, enum fs_vnode_type type,
					size_t size, void *private)
{
	struct fs_vnode *ret = (struct fs_vnode *)malloc(sizeof(*ret));

	/* Filling Initial State */
	ret->vnode_id = id;
	ret->type = type;
	ret->size = size;
	ret->private = private;

	/* Ref Count start as 1 */
	ret->refcnt = 1;

	/**
	 * NOTE: PMO_FILE is not created immediately at allocation time.
	 * When a vnode first mmaped,
	 * 	trigger a create_pmo for this vnode lazily.
	 */
	ret->pmo_cap = -1;

	/* Create a page cache entity for vnode */
	extern bool using_page_cache;
	init_list_head(&ret->node);

	return ret;
}

void push_fs_vnode(struct fs_vnode *n)
{
	list_append(&n->node, &fs_vnode_list);
}

void pop_free_fs_vnode(struct fs_vnode *n)
{
	list_del(&n->node);
	free(n);
}

struct fs_vnode *get_fs_vnode_by_id(unsigned long vnode_id)
{
	struct fs_vnode *n;
	for_each_in_list(n, struct fs_vnode, node, &fs_vnode_list) {
		if (n->vnode_id == vnode_id)
			return n;
	}
	return NULL;
}
