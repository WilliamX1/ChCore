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
#include <stdio.h>
#include <chcore/internal/raw_syscall.h>
#include <chcore/fs/defs.h>
#include "../fs_base/fs_wrapper_defs.h"
#include "../fs_base/fs_vnode.h"
#include "fakefs.h"

struct list_head fakefs_files;
struct spinlock fs_lock;
struct server_entry *server_entrys[MAX_SERVER_ENTRY_NUM];
struct list_head fs_vnode_list;
bool mounted;
bool using_page_cache;

void fakefs_test();
void init_fakefs_file_node(struct fakefs_file_node *n);

void fakefs_init() {
	init_list_head(&fakefs_files);
	spinlock_init(&fs_lock);
	struct fakefs_file_node *n = (struct fakefs_file_node *)malloc(sizeof(struct fakefs_file_node));
	init_fakefs_file_node(n);
	strcpy(n->path, "/");
	n->isdir = true;
	/* Insert node to fsm_server_entry_mapping */
	list_append(&n->node, &fakefs_files);
}


int main(int argc, const char *argv[]) 
{
	mounted = true;
	using_page_cache = false;
	init_fs_wrapper();
	fakefs_init();

    ipc_register_server(fs_server_dispatch);

     while (1) {
             __chcore_sys_yield();
     }

}