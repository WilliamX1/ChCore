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

#include <sys/types.h>

struct process_metadata {
        u64 phdr_addr;
        u64 phentsize;
        u64 phnum;
        u64 flags;
        u64 entry;
        u64 type;
};

struct user_elf_seg {
        u64 elf_pmo;
        size_t seg_sz;
        u64 p_vaddr;
        u64 flags;
};

#define ELF_PATH_LEN     256
#define ELF_MAX_LOAD_SEG 4

struct user_elf {
        struct user_elf_seg user_elf_seg[ELF_MAX_LOAD_SEG];
        char path[ELF_PATH_LEN];
        struct process_metadata elf_meta;
};

struct launch_process_args {
        struct user_elf *user_elf;
        int *child_process_cap;
        int *child_main_thread_cap;
        int *caps;
        int nr_caps;
        s32 cpuid;
        int argc;
        char **argv;
        int pid;
        u64 pcid;
};

int launch_process(struct launch_process_args *lp_args);
