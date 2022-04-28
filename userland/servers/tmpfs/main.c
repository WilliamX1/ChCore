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

#include <stdio.h>
#include <chcore/ipc.h>
#include <chcore/memory.h>
#include <chcore/internal/raw_syscall.h>
#include <chcore/internal/server_caps.h>
#include <chcore/assert.h>
#include <chcore/thread.h>
#include <chcore/tmpfs.h>

int init_tmpfs(void);

int tfs_load_image(const char *start);

extern const char __binary_ramdisk_cpio_start;
extern u64 __binary_ramdisk_cpio_size;

void fs_server_dispatch(struct ipc_msg *ipc_msg, u64 client_badge);

#ifdef TMPFS_TEST
void tfs_test();
#endif

int main()
{

        init_tmpfs();

#ifdef TMPFS_TEST
        tfs_test();
#else 
        tfs_load_image(&__binary_ramdisk_cpio_start);
#endif

        ipc_register_server(fs_server_dispatch);

        while (1) {
                __chcore_sys_yield();
        }
        return 0;
}
