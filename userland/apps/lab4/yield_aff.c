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

#include <chcore/internal/raw_syscall.h>
#include <chcore/thread.h>
#include <chcore/assert.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
        int ret = 0;
        int aff = 3;

        printf("Main thread on cpu %u\n", __chcore_sys_get_cpu_id());
        ret = __chcore_sys_set_affinity(-1, aff);
        chcore_assert(ret == 0);
        printf("Main thread set affinity %d\n", aff);
        __chcore_sys_yield();
        ret = __chcore_sys_get_affinity(-1);
        printf("Main thread affinity %d\n", ret);
        printf("Main thread exits on cpu_id: %u\n", __chcore_sys_get_cpu_id());
        return 0;
}
