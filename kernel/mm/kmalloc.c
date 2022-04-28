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

#include <common/types.h>
#include <common/macro.h>
#include <common/errno.h>
#include <common/util.h>
#include <common/kprint.h>
#include <mm/buddy.h>
#include <mm/slab.h>

#define _SIZE (1UL << SLAB_MAX_ORDER)

/* Declaration */
void *get_pages(int order);

u64 size_to_page_order(u64 size)
{
        u64 order;
        u64 pg_num;
        u64 tmp;

        order = 0;
        pg_num = ROUND_UP(size, BUDDY_PAGE_SIZE) / BUDDY_PAGE_SIZE;
        tmp = pg_num;

        while (tmp > 1) {
                tmp >>= 1;
                order += 1;
        }

        if (pg_num > (1 << order))
                order += 1;

        return order;
}

void *kmalloc(size_t size)
{
        u64 order;

        if (size <= _SIZE) {
                return alloc_in_slab(size);
        }

        if (size <= BUDDY_PAGE_SIZE)
                order = 0;
        else
                order = size_to_page_order(size);

        return get_pages(order);
}

void *kzalloc(size_t size)
{
        void *ptr;

        ptr = kmalloc(size);

        /* lack of memory */
        if (ptr == NULL)
                return NULL;

        memset(ptr, 0, size);
        return ptr;
}

void kfree(void *ptr)
{
        struct page *p_page;

        p_page = virt_to_page(ptr);
        if (p_page && p_page->slab) {
                free_in_slab(ptr);
        } else {
                buddy_free_pages(p_page->pool, p_page);
        }
}

void *get_pages(int order)
{
        struct page *p_page = NULL;
        int i;

        for (i = 0; i < physmem_map_num; ++i) {
                p_page = buddy_get_pages(&global_mem[i], order);
                if (p_page) {
                        break;
                }
        }

        if (!p_page) {
                kwarn("[OOM] Cannot get page from any memory pool!\n");
                return NULL;
        }
        return page_to_virt(p_page);
}

void free_pages(void *addr)
{
        struct page *p_page;
        p_page = virt_to_page(addr);
        buddy_free_pages(p_page->pool, p_page);
}

#ifdef CHCORE_KERNEL_TEST
#include <lab.h>
void lab2_test_kmalloc(void)
{
        /* This can test both boot page table and buddy system. */
        bool ok = true;
        {
                void *p = kmalloc(10);
                BUG_ON(p == NULL);
                kfree(p);
        }
        {
                int *p = (int *)kmalloc(100 * sizeof(int));
                BUG_ON(p == NULL);
                for (int i = 0; i < 100; i++) {
                        p[i] = i;
                }
                for (int i = 0; i < 100; i++) {
                        lab_assert(p[i] == i);
                }
                kfree(p);
        }
        {
                u8 *p = (u8 *)kzalloc(0x2000);
                BUG_ON(p == NULL);
                for (int i = 0; i < 0x2000; i++) {
                        lab_assert(p[i] == 0);
                }
                kfree(p);
        }
        lab_check(ok, "kmalloc");
}
#endif /* CHCORE_KERNEL_TEST */
