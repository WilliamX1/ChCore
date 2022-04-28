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

static void ipc_dispatch(struct ipc_msg *ipc_msg, u64 client_pid)
{
        int ret = 0;
        int mt_cap, pcid, pid;
        bool path_valid = false;
        struct procm_ipc_data *ipc_data;

        if (!ipc_msg)
                ipc_return(0, ret);

        ipc_data = (struct procm_ipc_data *)ipc_get_msg_data(ipc_msg);
        switch (ipc_data->request) {
        case PROCM_IPC_REQ_SPAWN:
                for (int i = 0; i < sizeof(ipc_data->spawn.args.path); i++) {
                        if (ipc_data->spawn.args.path[i] == '\0') {
                                path_valid = true;
                                break;
                        }
                }
                chcore_bug_on(!path_valid);
                pid = spawn(ipc_data->spawn.args.path, &mt_cap);
                if (pid > 0) {
                        ipc_data->spawn.returns.pid = pid;
                        ipc_msg->cap_slot_number = 1;
                        ipc_set_msg_cap(ipc_msg, 0, mt_cap);
                }
                break;
        default:
                ret = -1;
                break;
        }

        if (ret < 0) {
                ipc_return(ipc_msg, ret);
        } else {
                ipc_return_with_cap(ipc_msg, ret);
        }
}

void chcore_fakefs_test(int fakefs_cap);
static void *procm_main(void *arg)
{
        int ret, cap;

        /* wait until procm cap being set */
        while (__chcore_get_procm_cap() < 0)
                ;
        printf("Hello from ChCore Process Manager!\n");
        ipc_register_server(ipc_dispatch);

        /* start tmpfs */
        ret = spawn("/tmpfs.srv", &cap);
        chcore_bug_on(ret < 0);
        __chcore_set_tmpfs_cap(cap);

        /* start fsm */
        ret = spawn("/fsm.srv", &cap);
        chcore_bug_on(ret < 0);
        __chcore_set_fsm_cap(cap);
 
        int shell_cap;
        spawn("/shell.srv", &shell_cap);

        /* Server does not exit */
        while (1) {
                __chcore_sys_yield();
        }
}

int main(int argc, const char *argv[])
{
        int cap;

        /* Init procedure */
        cap = chcore_thread_create(procm_main, 0, 0, TYPE_USER);
        chcore_bug_on(cap < 0);
        __chcore_set_procm_cap(cap);
        return 0;
}
