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
#include <errno.h>
#include <sync/spin.h>

typedef struct ipc_struct {
        u64 conn_cap;
        u64 shared_buf;
        u64 shared_buf_len;
        struct spinlock ipc_lock;
} ipc_struct_t;

typedef struct ipc_msg {
        u64 data_len;
        u64 cap_slot_number;
        u64 data_offset;
        u64 cap_slots_offset;
} ipc_msg_t;

struct ipc_vm_config {
        u64 stack_base_addr;
        u64 stack_size;
        u64 buf_base_addr;
        u64 buf_size;
};

/* Shadow thread configs */
#define SERVER_STACK_BASE 0x7000000
#define SERVER_STACK_SIZE 0x1000
#define SERVER_BUF_BASE   0x7400000
#define SERVER_BUF_SIZE   0x1000
#define CLIENT_BUF_BASE   0x7800000
#define CLIENT_BUF_SIZE   0x1000

#define MAX_CLIENT        32
#define RETRY_UPPER_BOUND 100

/*
 * server_handler is an IPC routine (can have two arguments):
 * first is ipc_msg and second is client_pid.
 */
typedef void (*server_handler)();

/* Registeration interfaces */
struct ipc_struct *ipc_register_client(int server_thread_cap);
int ipc_register_server(server_handler server_handler);

/* IPC message operating interfaces */
struct ipc_msg *ipc_create_msg(struct ipc_struct *icb, u64 data_len,
                               u64 cap_slot_number);
char *ipc_get_msg_data(struct ipc_msg *ipc_msg);
u64 ipc_get_msg_cap(struct ipc_msg *ipc_msg, u64 cap_id);
int ipc_set_msg_data(struct ipc_msg *ipc_msg, void *data, u64 offset, u64 len);
int ipc_set_msg_cap(struct ipc_msg *ipc_msg, u64 cap_slot_index, u32 cap);
int ipc_destroy_msg(struct ipc_struct *icb, struct ipc_msg *ipc_msg);

/* IPC issue/finish interfaces */
s64 ipc_call(struct ipc_struct *icb, struct ipc_msg *ipc_msg);
void ipc_return(struct ipc_msg *ipc_msg, int ret);
void ipc_return_with_cap(struct ipc_msg *ipc_msg, int ret);
