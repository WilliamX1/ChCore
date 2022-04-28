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

#include <chcore/assert.h>
#include <chcore/memory.h>
#include <chcore/internal/utils.h>
#include <chcore/internal/server_caps.h>
#include <chcore/internal/idman.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "spawn.h"
#include "elf.h"
#include "launch.h"

#define PFLAGS2VMRFLAGS(PF)                                     \
        (((PF)&PF_X ? VM_EXEC : 0) | ((PF)&PF_W ? VM_WRITE : 0) \
         | ((PF)&PF_R ? VM_READ : 0))

#define OFFSET_MASK 0xfff

static int parse_elf_from_binary(const char *binary, struct user_elf *user_elf)
{
        int ret;
        struct elf_file *elf;
        size_t seg_sz, seg_map_sz;
        u64 p_vaddr;
        int i;
        int j;
        u16 e_type;
        void *tmp_seg;

        u64 start;
        u64 size;

        elf = elf_parse_file(binary);

        if (elf->header.e_type != ET_EXEC) {
                chcore_warn("program type is not supported.\n");
                return -ESUPPORT;
        }
        /* init pmo, -1 indicates that this pmo is not used */
        for (i = 0; i < ELF_MAX_LOAD_SEG; ++i)
                user_elf->user_elf_seg[i].elf_pmo = -1;

        for (i = 0, j = 0; i < elf->header.e_phnum; ++i) {
                if (elf->p_headers[i].p_type != PT_LOAD)
                        continue;

                if (j >= ELF_MAX_LOAD_SEG) {
                        chcore_bug("FIXME: too many PT_LOAD segments");
                }

                seg_sz = elf->p_headers[i].p_memsz;
                p_vaddr = elf->p_headers[i].p_vaddr;
                chcore_bug_on(elf->p_headers[i].p_filesz > seg_sz);
                seg_map_sz = ROUND_UP(seg_sz + p_vaddr, PAGE_SIZE)
                             - ROUND_DOWN(p_vaddr, PAGE_SIZE);

                user_elf->user_elf_seg[j].elf_pmo =
                        chcore_pmo_create(seg_map_sz, PMO_DATA);
                chcore_bug_on(user_elf->user_elf_seg[j].elf_pmo < 0);

                tmp_seg = chcore_pmo_auto_map(user_elf->user_elf_seg[j].elf_pmo,
                                              seg_map_sz,
                                              VM_READ | VM_WRITE);
                chcore_bug_on(!tmp_seg);

                memset(tmp_seg, 0, seg_map_sz);
                /*
                 * OFFSET_MASK is for calculating the final offset for loading
                 * different segments from ELF.
                 * ELF segment can specify not aligned address.
                 *
                 */
                start = (u64)tmp_seg
                        + (elf->p_headers[i].p_vaddr & OFFSET_MASK);
                size = elf->p_headers[i].p_filesz;
                memcpy((void *)start,
                       (void *)(binary + elf->p_headers[i].p_offset),
                       size);

                user_elf->user_elf_seg[j].seg_sz = seg_sz;
                user_elf->user_elf_seg[j].p_vaddr = p_vaddr;
                user_elf->user_elf_seg[j].flags =
                        PFLAGS2VMRFLAGS(elf->p_headers[i].p_flags);

                chcore_pmo_auto_unmap(user_elf->user_elf_seg[j].elf_pmo,
                                      (u64)tmp_seg,
                                      seg_map_sz);
                j++;
        }

        user_elf->elf_meta.phdr_addr =
                elf->p_headers[0].p_vaddr + elf->header.e_phoff;
        user_elf->elf_meta.phentsize = elf->header.e_phentsize;
        user_elf->elf_meta.phnum = elf->header.e_phnum;
        user_elf->elf_meta.flags = elf->header.e_flags;
        user_elf->elf_meta.entry = elf->header.e_entry;
        user_elf->elf_meta.type = elf->header.e_type;

        e_type = elf->header.e_type;
        elf_free(elf);
        return e_type;
}

/* Symbols defined in asm code generated from incbin.tpl.S */
extern const char __binary_fsm_elf_start;
extern size_t __binary_fsm_elf_size;
extern const char __binary_tmpfs_elf_start;
extern size_t __binary_tmpfs_elf_size;
extern const char __binary_userproc_elf_start;
extern size_t __binary_userproc_elf_size;
extern const char __binary_ipc_client_elf_start;
extern size_t __binary_ipc_client_elf_size;
extern const char __binary_shell_elf_start;
extern size_t __binary_shell_elf_size;
extern const char __binary_fakefs_elf_start;
extern size_t __binary_fakefs_elf_size;



enum incbin_elf_id {
        INCBIN_ELF_FSM,
        INCBIN_ELF_TMPFS,
        INCBIN_ELF_USER, /* Lab4 specific */
        INCBIN_ELF_IPC_CLIENT, /* Lab4 specific */
        INCBIN_ELF_SHELL, /* Lab4 specific */
        INCBIN_ELF_FAKEFS, /* Lab4 specific */
        INCBIN_ELF_COUNT,
};

int readelf_from_incbin(enum incbin_elf_id elf_id, struct user_elf *user_elf)
{
        int ret;
        switch (elf_id) {
        case INCBIN_ELF_FSM:
                ret = parse_elf_from_binary(&__binary_fsm_elf_start, user_elf);
                break;
        case INCBIN_ELF_TMPFS:
                ret = parse_elf_from_binary(&__binary_tmpfs_elf_start,
                                            user_elf);
                break;
        /* Lab4 specific */
        case INCBIN_ELF_USER:
                ret = parse_elf_from_binary(&__binary_userproc_elf_start,
                                            user_elf);
                break;
        case INCBIN_ELF_IPC_CLIENT:
                ret = parse_elf_from_binary(&__binary_ipc_client_elf_start,
                                            user_elf);
                break;
        case INCBIN_ELF_SHELL:
                ret = parse_elf_from_binary(&__binary_shell_elf_start,
                                            user_elf);
                break;
        case INCBIN_ELF_FAKEFS:
                ret = parse_elf_from_binary(&__binary_fakefs_elf_start,
                                            user_elf);
                break;
 
        /* Lab4 End */
        default:
                chcore_warn("no such elf binary included");
                return -ENOENT;
        }
        chcore_assert(ret == ET_EXEC);
        return ret;
}

int read_file_from_tfs(const char* path, char* buf);
int get_file_size_from_tfs(const char* path);

int readelf_from_fs(const char *filename, struct user_elf *user_elf)
{
        int file_size = get_file_size_from_tfs(filename);
        char* buf = (char*)malloc(file_size);

        read_file_from_tfs (filename, buf);
        // read_file_from_fsm (filename, buf);
        int ret = parse_elf_from_binary(buf,user_elf);
        return ret;
}

static inline int alloc_pcid(void)
{
        static struct id_manager pcid_mgr;
#define PCID_MAX (1 << 16) /* aarch64 specific value */
#define PCID_MIN 10 /* reserved */
        if (!id_manager_initialized(&pcid_mgr)) {
                init_id_manager(&pcid_mgr, PCID_MAX, PCID_MIN);
        }
        return alloc_id(&pcid_mgr);
}

static inline int alloc_pid(void)
{
        static struct id_manager pid_mgr;
#define PID_MAX (1 << 20)
#define PID_MIN 10 /* reserved */
        if (!id_manager_initialized(&pid_mgr)) {
                init_id_manager(&pid_mgr, PID_MAX, PID_MIN);
        }
        return alloc_id(&pid_mgr);
}

int spawn(const char *filename, int *new_thread_cap)
{
        struct user_elf user_elf;
        int ret;
        size_t len;
        char *argv[1];
        /* List system server here */
        int system_server_caps[] = {
                __chcore_get_procm_cap(),
                __chcore_get_fsm_cap(),
                __chcore_get_tmpfs_cap(),
        };

        struct launch_process_args lp_args;

        /* Lab 4 specific code here */
        if (strcmp(filename, "/user.bin") == 0) {
                ret = readelf_from_incbin(INCBIN_ELF_USER, &user_elf);
        } else if (strcmp(filename, "/ipc_client.bin") == 0) {
                ret = readelf_from_incbin(INCBIN_ELF_IPC_CLIENT, &user_elf);
                /* Lab 4 specific code ends */
        } else if (strcmp(filename, "/fsm.srv") == 0) {
                ret = readelf_from_incbin(INCBIN_ELF_FSM, &user_elf);
        } else if (strcmp(filename, "/tmpfs.srv") == 0) {
                ret = readelf_from_incbin(INCBIN_ELF_TMPFS, &user_elf);
        } else if (strcmp(filename, "/shell.srv") == 0) {
                ret = readelf_from_incbin(INCBIN_ELF_SHELL, &user_elf);
        } else if (strcmp(filename, "/fakefs.srv") == 0) {
                ret = readelf_from_incbin(INCBIN_ELF_FAKEFS, &user_elf);
        } else {
                ret = readelf_from_fs(filename, &user_elf);
        }

        if (ret < 0) {
                chcore_warn("failed to read elf\n");
                return ret;
        }

        strcpy(user_elf.path, filename);
        argv[0] = user_elf.path;

        lp_args.user_elf = &user_elf;
        lp_args.child_process_cap = NULL;
        lp_args.child_main_thread_cap = new_thread_cap;
        lp_args.caps = system_server_caps;
        lp_args.nr_caps = sizeof(system_server_caps) / sizeof(int);
        lp_args.cpuid = 0;
        lp_args.argc = 1;
        lp_args.argv = argv;
        lp_args.pcid = alloc_pcid();
        lp_args.pid = alloc_pid();

        ret = launch_process(&lp_args);
        return !ret ? lp_args.pid : ret;
}
