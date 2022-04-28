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

#include <chcore/memory.h>
#include <chcore/capability.h>
#include <chcore/internal/mem_layout.h>
#include <chcore/internal/utils.h>

u64 chcore_vaddr_alloc(u64 size)
{
        static u64 current = MEM_AUTO_ALLOC_REGION;

        size = ROUND_UP(size, PAGE_SIZE);
        if (size == 0)
                return 0;

        u64 allocated = __sync_fetch_and_add(&current, size);
        if (allocated >= MEM_AUTO_ALLOC_REGION + MEM_AUTO_ALLOC_REGION_SIZE)
                return 0;
        return allocated;
}

void chcore_vaddr_free(u64 vaddr, u64 size)
{
        /* do nothing for now */
}

void *chcore_pmo_auto_map(int pmo_cap, u64 size, u64 perm)
{
        u64 vaddr = chcore_vaddr_alloc(size);
        if (vaddr == 0)
                return NULL;
        int ret = chcore_pmo_map(SELF_CAP, pmo_cap, vaddr, perm);
        if (ret != 0) {
                chcore_vaddr_free(vaddr, size);
                return NULL;
        }
        return (void *)vaddr;
}

void chcore_pmo_auto_unmap(int pmo_cap, u64 vaddr, u64 size)
{
        chcore_pmo_unmap(SELF_CAP, pmo_cap, vaddr);
        chcore_vaddr_free(vaddr, size);
}
