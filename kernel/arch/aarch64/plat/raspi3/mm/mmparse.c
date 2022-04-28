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

#include <arch/mmu.h>
#include <common/macro.h>
#include <common/types.h>
#include <mm/mm.h>
#include <mm/buddy.h>
#include <machine.h>

extern int physmem_map_num;
extern u64 physmem_map[][2];

/*
 * The usable physical memory: 0x0 - 0x3f000000.
 * The top 32M is reserved for gpu_mem, i.e., [0x3f000000 - 32M, 0x3f000000).
 * The bottom from 0x80000 is for kernel image.
 * So the real usable physical memory is [img_end, 0x3f000000 - 32M)
 */

extern char img_end;
#define USABLE_MEM_START (ROUND_UP((paddr_t)(&img_end), PAGE_SIZE))
#define USABLE_MEM_END   (0x3f000000)
#define RESERVED_FOR_GPU (32 << 20) /* 32M */

void parse_mem_map(void)
{
        physmem_map_num = 1;
        physmem_map[0][0] = USABLE_MEM_START; /* 4K-aligned */
        physmem_map[0][1] = USABLE_MEM_END - RESERVED_FOR_GPU; /* 4K-aligned */
        kinfo("physmem_map: [0x%lx, 0x%lx)\n",
              physmem_map[0][0],
              physmem_map[0][1]);
}
