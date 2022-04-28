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

#include <chcore/types.h>

/* PMO types */
#define PMO_ANONYM 0
#define PMO_DATA   1
#define PMO_FORBID 10 /* Forbidden area: avoid overflow */

/* virtual memory permission flags */
#define VM_READ   (1 << 0)
#define VM_WRITE  (1 << 1)
#define VM_EXEC   (1 << 2)
#define VM_FORBID (0)

#define PAGE_SIZE 0x1000

#ifdef __cplusplus
extern "C" {
#endif

int chcore_pmo_create(u64 size, u64 type);
int chcore_pmo_map(u64 target_cap_group_cap, u64 pmo_cap, u64 addr, u64 perm);
int chcore_pmo_unmap(u64 target_cap_group_cap, u64 pmo_cap, u64 addr);
int chcore_pmo_write(u64 pmo_cap, u64 offset, void *buf, u64 len);
int chcore_pmo_read(u64 pmo_cap, u64 offset, void *buf, u64 len);

struct pmo_request {
        /* input: args */
        u64 size;
        u64 type;

        /* output: return value */
        u64 ret_cap;
};

int chcore_pmo_create_multi(struct pmo_request *reqs, u64 nr_reqs);

struct pmo_map_request {
        /* input: args */
        u64 pmo_cap;
        u64 addr;
        u64 perm;
        /*
         * If you want to free the pmo cap in current cap goup
         * after the pmo was mapped to the vmsapce of another
         * process, please set free_cap to 1.
         */
        u64 free_cap;

        /* output: return value */
        u64 ret;
};

int chcore_pmo_map_multi(u64 target_cap_group_cap, struct pmo_map_request *reqs,
                         u64 nr_reqs);

/*
 * Allocate and free virtual address region.
 * The size of allocated vaddr block is aligned to 4K.
 */
u64 chcore_vaddr_alloc(u64 size);
void chcore_vaddr_free(u64 vaddr, u64 size);

/*
 * Automatically map and unmap PMO to available virtual address.
 */
void *chcore_pmo_auto_map(int pmo_cap, u64 size, u64 perm);
void chcore_pmo_auto_unmap(int pmo_cap, u64 vaddr, u64 size);

#ifdef __cplusplus
}
#endif
