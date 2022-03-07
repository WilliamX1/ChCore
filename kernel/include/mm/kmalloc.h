#pragma once

#include <common/types.h>
#include <common/util.h>

u64 size_to_page_order(size_t size);
void *kmalloc(size_t size);
/* zero the allocated are */
void *kzalloc(size_t size);
void kfree(void *ptr);

/* return vaddr of (1 << order) continous free physical pages */
void *get_pages(int order);
void free_pages(void *addr);
