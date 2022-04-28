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
#include <common/list.h>

/*
 * Supported Order: [0, BUDDY_MAX_ORDER).
 * The max allocated size (continous physical memory size) is
 * 2^(BUDDY_MAX_ORDER - 1) * 4K, i.e., 16M.
 */
#define BUDDY_PAGE_SIZE (0x1000)
#define BUDDY_MAX_ORDER (14UL)

/* Forward declaration */
struct phys_mem_pool;

/* `struct page` is the metadata of one physical 4k page. */
struct page {
        /* Free list */
        struct list_head node;
        /* Whether the correspond physical page is free now. */
        int allocated;
        /* The order of the memory chunck that this page belongs to. */
        int order;
        /* Used for ChCore slab allocator. */
        void *slab;
        /* The physical memory pool this page belongs to */
        struct phys_mem_pool *pool;
};

struct free_list {
        struct list_head free_list;
        u64 nr_free;
};

/* Disjoint physical memory can be represented by several phys_mem_pool. */
struct phys_mem_pool {
        /*
         * The start virtual address (for used in kernel) of
         * this physical memory pool.
         */
        u64 pool_start_addr;
        u64 pool_mem_size;

        /*
         * This field is only used in ChCore unit test.
         * The number of (4k) physical pages in this physical memory pool.
         */
        u64 pool_phys_page_num;

        /*
         * The start virtual address (for used in kernel) of
         * the metadata area of this pool.
         */
        struct page *page_metadata;

        /* The free list of different free-memory-chunk orders. */
        struct free_list free_lists[BUDDY_MAX_ORDER];
};

extern struct phys_mem_pool global_mem[];
extern int physmem_map_num;

void init_buddy(struct phys_mem_pool *zone, struct page *start_page,
                vaddr_t start_addr, u64 page_num);

struct page *buddy_get_pages(struct phys_mem_pool *, u64 order);
void buddy_free_pages(struct phys_mem_pool *, struct page *page);

void *page_to_virt(struct page *page);
struct page *virt_to_page(void *ptr);
u64 get_free_mem_size_from_buddy(struct phys_mem_pool *);
