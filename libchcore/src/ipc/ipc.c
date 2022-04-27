#include <chcore/ipc.h>
#include <chcore/thread.h>
#include <chcore/assert.h>
#include <chcore/internal/raw_syscall.h>
#include <string.h>
#include <chcore/memory.h>
#include <sync/spin.h>
#include <errno.h>
#include <malloc.h>
#include <stdio.h>

int client_ipc_num = 0; // Current clients number

/* Lab4: Register IPC server */
int ipc_register_server(server_handler server_handler)
{
        int ret = 0;
        struct ipc_vm_config vm_config;
        /* LAB 4 TODO BEGIN: fill vm_config */
        vm_config.buf_base_addr = SERVER_BUF_BASE;
        vm_config.buf_size = SERVER_BUF_SIZE;
        vm_config.stack_base_addr = SERVER_STACK_BASE;
        vm_config.stack_size = SERVER_STACK_SIZE;
        /* LAB 4 TODO END */
        ret = __chcore_sys_register_server(
                (u64)server_handler, MAX_CLIENT, (u64)&vm_config);
        chcore_bug_on(ret < 0);
        return 0;
}

/* Lab4: Register IPC client */
struct ipc_struct *ipc_register_client(int server_thread_cap)
{
        int conn_cap, retry_times = RETRY_UPPER_BOUND;
        struct ipc_struct *ipc_struct = malloc(sizeof(struct ipc_struct));
        // Assign a unique id for each client
        int client_id = __sync_fetch_and_add(&client_ipc_num, 1);
        struct ipc_vm_config vm_config;

        if (client_id >= MAX_CLIENT)
                return NULL;

        /* LAB 4 TODO BEGIN: fill vm_config according to client_id */
        vm_config.buf_base_addr = CLIENT_BUF_BASE + client_id * CLIENT_BUF_SIZE;
        vm_config.buf_size = CLIENT_BUF_SIZE;
        /* LAB 4 TODO END */
        while (retry_times) {
                conn_cap = __chcore_sys_register_client((u32)server_thread_cap,
                                                        (u64)&vm_config);
                if (conn_cap > 0) {
                        break;
                } else if (conn_cap == -EIPCRETRY) {
                        // Happens when server is not ready yet
                        retry_times--;
                        __chcore_sys_yield();
                        continue;
                } else if (conn_cap < 0) {
                        return 0;
                }
        }
        ipc_struct->shared_buf = vm_config.buf_base_addr;
        ipc_struct->shared_buf_len = vm_config.buf_size;
        ipc_struct->conn_cap = conn_cap;
        spinlock_init(&ipc_struct->ipc_lock);

        return ipc_struct;
}

/* IPC msg related stuff */
/*
 * ipc_msg is constructed on the shm pointed by icb->shared_buf.
 * A new ips_msg will override the old one.
 */
struct ipc_msg *ipc_create_msg(struct ipc_struct *icb, u64 data_len,
                               u64 cap_slot_number)
{
        ipc_msg_t *ipc_msg;
        int i;

        spinlock_lock(&icb->ipc_lock);
        ipc_msg = (ipc_msg_t *)icb->shared_buf;
        ipc_msg->data_len = data_len;
        ipc_msg->cap_slot_number = cap_slot_number;

        ipc_msg->data_offset = sizeof(*ipc_msg);
        ipc_msg->cap_slots_offset = ipc_msg->data_offset + data_len;
        memset(ipc_get_msg_data(ipc_msg), 0, data_len);
        for (i = 0; i < cap_slot_number; i++)
                ipc_set_msg_cap(ipc_msg, i, -1);

        return ipc_msg;
}

char *ipc_get_msg_data(struct ipc_msg *ipc_msg)
{
        return (char *)ipc_msg + ipc_msg->data_offset;
}

/* Each cap takes 8 bytes although its length is 4 bytes in fact */
static u64 *ipc_get_msg_cap_ptr(struct ipc_msg *ipc_msg, u64 cap_id)
{
        return (u64 *)((char *)ipc_msg + ipc_msg->cap_slots_offset) + cap_id;
}

u64 ipc_get_msg_cap(struct ipc_msg *ipc_msg, u64 cap_slot_index)
{
        if (cap_slot_index >= ipc_msg->cap_slot_number) {
                printf("%s failed due to overflow.\n", __func__);
                return -1;
        }
        return *ipc_get_msg_cap_ptr(ipc_msg, cap_slot_index);
}

int ipc_set_msg_data(struct ipc_msg *ipc_msg, void *data, u64 offset, u64 len)
{
        if ((offset + len < offset) || (offset + len > ipc_msg->data_len)) {
                printf("%s failed due to overflow.\n", __func__);
                return -1;
        }

        /* Lab4: memcpy the data to correct offset in ipc_msg */
        /* LAB 4 TODO BEGIN */
        memcpy(ipc_get_msg_data(ipc_msg) + offset, data, len);
        /* LAB 4 TODO END */
        return 0;
}

int ipc_set_msg_cap(struct ipc_msg *ipc_msg, u64 cap_slot_index, u32 cap)
{
        if (cap_slot_index >= ipc_msg->cap_slot_number) {
                printf("%s failed due to overflow.\n", __func__);
                return -1;
        }

        *ipc_get_msg_cap_ptr(ipc_msg, cap_slot_index) = cap;
        return 0;
}

/* Unlock the ipc_msg when destroying the message */
int ipc_destroy_msg(struct ipc_struct *icb, struct ipc_msg *ipc_msg)
{
        spinlock_unlock(&icb->ipc_lock);
        return 0;
}

/* IPC Call/return */
/* Client uses **ipc_call** to issue an IPC request */
s64 ipc_call(struct ipc_struct *icb, struct ipc_msg *ipc_msg)
{
        s64 ret;

        if (icb->conn_cap == 0) {
                ret = -EINVAL;
                goto out;
        }
        ret = __chcore_sys_ipc_call(icb->conn_cap,
                                    (void *)ipc_msg,
                                    ipc_msg ? ipc_msg->cap_slot_number : 0);
out:
        return ret;
}

/* Server uses **ipc_return** to finish an IPC request */
void ipc_return(struct ipc_msg *ipc_msg, int ret)
{
        /* update the ipc_msg to show that no cap will be transferred */
        if (ipc_msg)
                ipc_msg->cap_slot_number = 0;
        __chcore_sys_ipc_return((u64)ret, 0);
}

void ipc_return_with_cap(struct ipc_msg *ipc_msg, int ret)
{
        __chcore_sys_ipc_return((u64)ret, ipc_msg->cap_slot_number);
}
