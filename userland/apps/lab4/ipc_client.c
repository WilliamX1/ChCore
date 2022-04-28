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

#define IPC_TEST_NUM 100
#define IPC_THD_NUM  20
volatile int finished = 0;

void *test_multiple_ipc_routine(void *arg)
{
        struct ipc_struct *client_ipc_struct;
        struct ipc_msg *ipc_msg;
        int ret, i;

        int server_cap = (int)(u64)arg;
        client_ipc_struct = ipc_register_client(server_cap);
        ipc_msg = ipc_create_msg(client_ipc_struct, 4, 0);
        chcore_assert(client_ipc_struct != 0);
        for (i = 0; i < IPC_TEST_NUM; i++) {
                ipc_set_msg_data(ipc_msg, (char *)&i, 0, 4);
                ret = ipc_call(client_ipc_struct, ipc_msg);
                chcore_assert(ret == 100 + i);
        }
        ipc_destroy_msg(client_ipc_struct, ipc_msg);
        __sync_fetch_and_add(&finished, 1);
        return 0;
}

void test_ipc_routine(int server_cap)
{
        struct ipc_struct *client_ipc_struct;
        struct ipc_msg *ipc_msg;
        int shared_page_pmo_cap, shared_msg;
        int ret, i;

        printf("IPC test: connect to server %d\n", server_cap);
        client_ipc_struct = ipc_register_client(server_cap);
        chcore_assert(client_ipc_struct != 0);

        /* IPC no data */
        printf("IPC no data test .... ");
        ipc_call(client_ipc_struct, 0);
        ipc_destroy_msg(client_ipc_struct, ipc_msg);
        printf("Passed!\n");

        /* IPC send data */
        printf("IPC transfer data test .... ");
        ipc_msg = ipc_create_msg(client_ipc_struct, 4, 0);
        for (i = 0; i < IPC_TEST_NUM; i++) {
                ipc_set_msg_data(ipc_msg, (char *)&i, 0, 4);
                ret = ipc_call(client_ipc_struct, ipc_msg);
                chcore_assert(ret == 100 + i);
        }
        ipc_destroy_msg(client_ipc_struct, ipc_msg);
        printf("Passed!\n");

        /* IPC send data */
        printf("IPC transfer cap test .... ");
        shared_page_pmo_cap = __chcore_sys_create_pmo(0x1000, PMO_DATA);
        shared_msg = 0xbeefbeef;
        __chcore_sys_write_pmo(
                shared_page_pmo_cap, 0, (u64)&shared_msg, sizeof(shared_msg));
        ipc_msg = ipc_create_msg(client_ipc_struct, 0, 1);
        ipc_set_msg_cap(ipc_msg, 0, shared_page_pmo_cap);
        ret = ipc_call(client_ipc_struct, ipc_msg);
        chcore_assert(ret == 0xabcd);
        chcore_assert(ipc_msg->cap_slot_number == 1);

        int ret_cap, read_val;
        ret_cap = ipc_get_msg_cap(ipc_msg, 0);
        chcore_assert(ret_cap >= 0);
        ret = __chcore_sys_read_pmo(
                ret_cap, 0, (u64)&read_val, sizeof(read_val));
        chcore_assert(ret >= 0);
        chcore_assert(read_val == 0x233);
        ipc_destroy_msg(client_ipc_struct, ipc_msg);
        printf("Passed!\n");

        printf("IPC transfer large data test .... ");
        ipc_msg = ipc_create_msg(client_ipc_struct, 4 * 100, 0);
        for (i = 0; i < IPC_TEST_NUM; i++) {
                ipc_set_msg_data(ipc_msg, (char *)&i, i * 4, 4);
        }
        ret = ipc_call(client_ipc_struct, ipc_msg);
        chcore_assert(ret == (0 + IPC_TEST_NUM - 1) * IPC_TEST_NUM / 2);
        printf("Passed!\n");
        ipc_destroy_msg(client_ipc_struct, ipc_msg);

        printf("%d threads concurrent IPC test .... ", IPC_THD_NUM);
        finished = 0;
        for (int i = 0; i < IPC_THD_NUM; i++)
                chcore_thread_create(
                        test_multiple_ipc_routine, server_cap, 0, TYPE_USER);
        while (finished != IPC_THD_NUM)
                ;
        printf("Passed!\n");
}

int main(int argc, char *argv[], char *envp[])
{
        printf("Hello from ipc_client.bin!\n");
        test_ipc_routine(__chcore_get_procm_cap());
        return 0;
}
