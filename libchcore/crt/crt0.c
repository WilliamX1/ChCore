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

#include <chcore/types.h>
#include <chcore/internal/raw_syscall.h>
#include <chcore/internal/crt0_arch.h>
#include <chcore/assert.h>

int main();

void _start_c(long *p)
{
        int argc = (int)p[0];
        char **argv = (void *)(p + 1);
        main(argc, argv);
        __chcore_sys_thread_exit(); /* Should recycle process in real system */
        chcore_bug("Should not reach here!\n");
}
