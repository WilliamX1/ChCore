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

#include <object/object.h>
#include <common/list.h>
#include <common/types.h>
#include <common/bitops.h>
#include <common/kprint.h>
#include <common/macro.h>
#include <arch/sync.h>

struct object_slot {
        u64 slot_id;
        struct cap_group *cap_group;
        int isvalid;
        u64 rights;
        struct object *object;
        /* link copied slots pointing to the same object */
        struct list_head copies;
};

#define BASE_OBJECT_NUM BITS_PER_LONG
/* 1st cap is cap_group. 2nd cap is vmspace */
#define CAP_GROUP_OBJ_ID 0
#define VMSPACE_OBJ_ID   1

struct slot_table {
        unsigned int slots_size;
        struct object_slot **slots;
        /*
         * if a bit in full_slots_bmp is 1, corresponding
         * sizeof(unsigned long) bits in slots_bmp are all set
         */
        unsigned long *full_slots_bmp;
        unsigned long *slots_bmp;
};

#define MAX_GROUP_NAME_LEN 63

struct cap_group {
        struct slot_table slot_table;
        struct list_head thread_list;
        /* The number of threads */
        int thread_cnt;

        /*
         * Each process has a unique pid as a global identifier which
         * is set by the system server, procm.
         * Currently, pid is used as a client ID during IPC.
         */
        u64 pid;

        /* Now is used for debugging */
        char cap_group_name[MAX_GROUP_NAME_LEN + 1];
};

#define current_cap_group (current_thread->cap_group)

/*
 * ATTENTION: These interfaces are for capability internal use.
 * As a cap user, check object.h for interfaces for cap.
 */
int alloc_slot_id(struct cap_group *cap_group);

static inline void free_slot_id(struct cap_group *cap_group, int slot_id)
{
        struct slot_table *slot_table = &cap_group->slot_table;
        clear_bit(slot_id, slot_table->slots_bmp);
        clear_bit(slot_id / BITS_PER_LONG, slot_table->full_slots_bmp);
        slot_table->slots[slot_id] = NULL;
}

static inline struct object_slot *get_slot(struct cap_group *cap_group,
                                           int slot_id)
{
        if (slot_id < 0 || slot_id >= cap_group->slot_table.slots_size)
                return NULL;
        return cap_group->slot_table.slots[slot_id];
}

static inline void install_slot(struct cap_group *cap_group, int slot_id,
                                struct object_slot *slot)
{
        // BUG_ON(!is_write_locked(&cap_group->slot_table.table_guard));
        BUG_ON(!get_bit(slot_id, cap_group->slot_table.slots_bmp));
        cap_group->slot_table.slots[slot_id] = slot;
}

void *get_opaque(struct cap_group *cap_group, int slot_id, bool type_valid,
                 int type);

int __cap_free(struct cap_group *cap_group, int slot_id);

struct cap_group *create_root_cap_group(char *, size_t);

/* Fixed pid for root process and servers */
#define ROOT_PID (1)

/**
 * Fixed pcid for root process and servers,
 */
#define ROOT_PCID (1)

struct cap_group *root_cap_group;

/* Syscalls */
int sys_create_cap_group(u64 pid, u64 cap_group_name, u64 name_len, u64 pcid);
