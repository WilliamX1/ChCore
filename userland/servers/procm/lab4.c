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

/* Only for lab4 use, not a part of procm */
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <chcore/thread.h>
#include <chcore/ipc.h>
#include <chcore/internal/raw_syscall.h>
#include <chcore/assert.h>
#include <chcore/procm.h>
#include <chcore/internal/idman.h>
#include <chcore/internal/server_caps.h>
#include <chcore/fsm.h>
#include <chcore/memory.h>

#include "elf.h"
#include "spawn.h"

static void lab4_test_ipc_dispatch(struct ipc_msg *ipc_msg, u64 client_pid)
{
        int ret = 0;
        int ret_cap, write_val;
        bool ret_with_cap = false;

        if (!ipc_msg)
                ipc_return(0, ret);

        if (ipc_msg->cap_slot_number == 1) { /* transfer pmo cap test */
                int cap;
                int shared_msg;

                cap = ipc_get_msg_cap(ipc_msg, 0);
                chcore_assert(cap >= 0);
                ret = __chcore_sys_read_pmo(
                        cap, 0, (u64)&shared_msg, sizeof(shared_msg));
                chcore_assert(ret >= 0);
                /* read from shared memory should be MAGIC_NUM */
                chcore_assert(shared_msg == 0xbeefbeef);
                /* return a pmo cap */
                ret_cap = __chcore_sys_create_pmo(0x1000, PMO_DATA);
                chcore_assert(ret >= 0);
                ipc_msg->cap_slot_number = 1;
                ipc_set_msg_cap(ipc_msg, 0, ret_cap);
                ret_with_cap = true;
                write_val = 0x233;
                ret = __chcore_sys_write_pmo(
                        ret_cap, 0, (u64)&write_val, sizeof(write_val));
                chcore_assert(ret >= 0);
                ret = 0xabcd; /* ipc return 0 */
        } else if (ipc_msg->data_len == 4) { /* transfer message test */
                ret = ((int *)ipc_get_msg_data(ipc_msg))[0] + 100;
        } else {
                ret = 0;
                int i = 0;
                int len = ipc_msg->data_len;
                while ((i * 4) < len) {
                        ret += ((int *)ipc_get_msg_data(ipc_msg))[i];
                        i++;
                }
        }

        if (ret_with_cap)
                ipc_return_with_cap(ipc_msg, ret);
        else
                ipc_return(ipc_msg, ret);
}

static void *routine(void *arg)
{
        int ret, cap;

        /* wait until procm cap being set */
        while (__chcore_get_procm_cap() < 0)
                ;
        printf("Hello from ChCore Process Manager!\n");

        /* Lab4 specific, remove in lab5 */
        spawn("/user.bin", &cap); /* Test spawn function */
        printf("before reg server!\n");
        ipc_register_server(lab4_test_ipc_dispatch);
        printf("reg server return!\n");
        spawn("/ipc_client.bin", &cap); /* Test IPC */
        /* End Lab4 specific */

        /* Server does not exit */
        while (1) {
                __chcore_sys_yield();
        }
}

int main(int argc, const char *argv[])
{
        int cap;

        cap = chcore_thread_create(routine, 0, 0, TYPE_USER);
        chcore_bug_on(cap < 0);
        __chcore_set_procm_cap(cap);
        return 0;
}
