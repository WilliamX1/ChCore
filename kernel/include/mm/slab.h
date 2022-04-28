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

#define SLAB_INIT_SIZE (2 * 1024 * 1024) // 2M

/* order range: [SLAB_MIN_ORDER, SLAB_MAX_ORDER] */
#define SLAB_MIN_ORDER (5)
#define SLAB_MAX_ORDER (11)

typedef struct slab_header slab_header_t;
struct slab_header {
        void *free_list_head;
        slab_header_t *next_slab;
        int order;
};

typedef struct slab_slot_list slab_slot_list_t;
struct slab_slot_list {
        void *next_free;
};

void init_slab(void);

void *alloc_in_slab(u64);
void free_in_slab(void *addr);

u64 get_free_mem_size_from_slab(void);
