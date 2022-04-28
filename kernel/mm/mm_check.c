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

#include <mm/mm_check.h>
#include <mm/buddy.h>
#include <mm/slab.h>

/*
 * Note that this function does not return the exact number of free memory in
 * the system because some other CPUs may allocate memory concurrently.
 *
 * Nevertheless, we use it for debugging now.
 */
u64 get_free_mem_size(void)
{
        u64 size;
        int i;

        size = get_free_mem_size_from_slab();
        for (i = 0; i < physmem_map_num; ++i) {
                size += get_free_mem_size_from_buddy(&global_mem[i]);
        }
        return size;
}
