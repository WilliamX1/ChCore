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

#include <common/types.h>
#include <common/errno.h>
#include <common/list.h>

struct object {
        u64 type;
        u64 size;
        /* Link all slots point to this object */
        struct list_head copies_head;
        /*
         * refcount is added when a slot points to it and when get_object is
         * called. Object is freed when it reaches 0.
         */
        u64 refcount;
        /*
         * opaque marks the end of this struct and the real object will be
         * stored here. Now its address will be 8-byte aligned.
         */
        u64 opaque[];
};

enum object_type {
        TYPE_CAP_GROUP = 0,
        TYPE_THREAD,
        TYPE_CONNECTION,
        TYPE_PMO,
        TYPE_VMSPACE,
        TYPE_SEMAPHORE, // <lab4>
        TYPE_NR,
};

struct cap_group;

typedef void (*obj_deinit_func)(void *);
extern const obj_deinit_func obj_deinit_tbl[TYPE_NR];

void *obj_get(struct cap_group *cap_group, int slot_id, int type);
void obj_put(void *obj);

void *obj_alloc(u64 type, u64 size);
void obj_free(void *obj);
int cap_alloc(struct cap_group *cap_group, void *obj, u64 rights);
int cap_free(struct cap_group *cap_group, int slot_id);
int cap_copy(struct cap_group *src_cap_group, struct cap_group *dest_cap_group,
             int src_slot_id);
int cap_move(struct cap_group *src_cap_group, struct cap_group *dest_cap_group,
             int src_slot_id);

int cap_free_all(struct cap_group *cap_group, int slot_id);

/* Syscalls */
int sys_cap_copy_to(u64 dest_cap_group_cap, u64 src_slot_id);
int sys_cap_copy_from(u64 src_cap_group_cap, u64 src_slot_id);
int sys_transfer_caps(u64 dest_group_cap, u64 src_caps_buf, int nr_caps,
                      u64 dst_caps_buf);
int sys_cap_move(u64 dest_cap_group_cap, u64 src_slot_id);
int sys_get_all_caps(u64 cap_group_cap);
