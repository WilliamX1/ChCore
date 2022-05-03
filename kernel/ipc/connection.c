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

#include <ipc/connection.h>
#include <mm/kmalloc.h>
#include <mm/uaccess.h>
#include <mm/mm.h>
#include <sched/context.h>
#include <irq/irq.h>

/* Impl in memory.c */
int pmo_init(struct pmobject *pmo, pmo_type_t type, size_t len, paddr_t paddr);

void connection_deinit(void *conn)
{
        /* For now, no de-initialization is required */
}

/**
 * Helper function called when an ipc_connection is created
 */
static struct thread *create_server_thread(struct thread *src)
{
        struct thread *new;
        struct server_ipc_config *new_config;
        struct server_ipc_config *src_config;

        new = kmalloc(sizeof(struct thread));
        BUG_ON(new == NULL);

        new->vmspace = obj_get(src->cap_group, VMSPACE_OBJ_ID, TYPE_VMSPACE);
        BUG_ON(!new->vmspace);

        // Init the thread ctx
        new->thread_ctx = create_thread_ctx(TYPE_SHADOW);
        if (!new->thread_ctx)
                goto out_fail;
        memcpy((char *)&(new->thread_ctx->ec),
               (const char *)&(src->thread_ctx->ec),
               sizeof(arch_exec_cont_t));
        new->thread_ctx->prio = MAX_PRIO - 1;
        new->thread_ctx->state = TS_INIT;
        new->thread_ctx->affinity = NO_AFF;
        new->thread_ctx->type = TYPE_SHADOW;

        src_config = (struct server_ipc_config *)src->general_ipc_config;
        new_config = kzalloc(sizeof(struct server_ipc_config));
        BUG_ON(new_config == 0);
        new_config->callback = src_config->callback;
        new_config->vm_config = src_config->vm_config;
        new->general_ipc_config = new_config;
        new->cap_group = src->cap_group;
        obj_put(new->vmspace);
        return new;
out_fail:
        obj_put(new->vmspace);
        kfree(new);
        return NULL;
}

/**
 * Helper function to create an ipc_connection by the client thread
 */
static int create_connection(struct thread *source, struct thread *target,
                             struct ipc_vm_config *client_vm_config)
{
        struct ipc_connection *conn = NULL;
        int ret = 0;
        int conn_cap = 0, server_conn_cap = 0;
        struct pmobject *stack_pmo, *buf_pmo;
        int conn_idx;
        struct server_ipc_config *server_ipc_config;
        struct ipc_vm_config *vm_config;
        u64 server_stack_base, server_buf_base, client_buf_base;
        u64 stack_size, buf_size;

        BUG_ON(source == NULL);
        BUG_ON(target == NULL);

        // Get the ipc_connection
        conn = obj_alloc(TYPE_CONNECTION, sizeof(*conn));
        if (!conn) {
                ret = -ENOMEM;
                goto out_fail;
        }
        conn->target = create_server_thread(target);
        if (!conn->target) {
                ret = -ENOMEM;
                goto out_fail;
        }
        // Get the server's ipc config
        server_ipc_config = target->general_ipc_config;
        vm_config = &server_ipc_config->vm_config;
        conn_idx = find_next_zero_bit(
                server_ipc_config->conn_bmp, server_ipc_config->max_client, 0);
        set_bit(conn_idx, server_ipc_config->conn_bmp);

        // Create the server thread's stack
        /* Lab4: set server_stack_base */
        /* LAB 4 TODO BEGIN */
        server_stack_base = vm_config->stack_base_addr + conn_idx * vm_config->stack_size;
        /* LAB 4 TODO END */
        stack_size = vm_config->stack_size;
        stack_pmo = kmalloc(sizeof(struct pmobject));
        if (!stack_pmo) {
                ret = -ENOMEM;
                goto out_free_obj;
        }
        pmo_init(stack_pmo, PMO_DATA, stack_size, 0);
        vmspace_map_range(target->vmspace,
                          server_stack_base,
                          stack_size,
                          VMR_READ | VMR_WRITE,
                          stack_pmo);

        conn->server_stack_top = server_stack_base + stack_size;

        // Create and map the shared buffer for client and server
        /* LAB 4: set server_buf_base and client_buf_base */
        /* LAB 4 TODO BEGIN */
        server_buf_base = vm_config->buf_base_addr + conn_idx * vm_config->buf_size;
        client_buf_base = client_vm_config->buf_base_addr;
        /* LAB 4 TODO END */
        buf_size = MIN(vm_config->buf_size, client_vm_config->buf_size);
        client_vm_config->buf_size = buf_size;

        buf_pmo = kmalloc(sizeof(struct pmobject));
        if (!buf_pmo) {
                ret = -ENOMEM;
                goto out_free_stack_pmo;
        }
        pmo_init(buf_pmo, PMO_DATA, buf_size, 0);

        /* LAB 4: map shared ipc buf to vmspace of server and client */
        /* LAB 4 TODO BEGIN */
        vmspace_map_range(current_thread->vmspace, client_buf_base, buf_size, VMR_READ | VMR_WRITE, buf_pmo);
        vmspace_map_range(target->vmspace, server_buf_base, buf_size, VMR_READ | VMR_WRITE, buf_pmo);
        /* LAB 4 TODO END */

        conn->buf.client_user_addr = client_buf_base;
        conn->buf.server_user_addr = server_buf_base;

        conn_cap = cap_alloc(current_cap_group, conn, 0);
        if (conn_cap < 0) {
                ret = conn_cap;
                goto out_free_obj;
        }

        server_conn_cap =
                cap_copy(current_cap_group, target->cap_group, conn_cap);
        if (server_conn_cap < 0) {
                ret = server_conn_cap;
                goto out_free_obj;
        }
        conn->server_conn_cap = server_conn_cap;

        return conn_cap;
out_free_stack_pmo:
        kfree(stack_pmo);
out_free_obj:
        obj_free(conn);
out_fail:
        return ret;
}

/**
 * Lab4
 * Helper function
 * Client thread calls this function and then return to server thread
 * This function should never return
 *
 * Replace the place_holder to correct value!
 */
static u64 thread_migrate_to_server(struct ipc_connection *conn, u64 arg)
{
        struct thread *target = conn->target;
        struct server_ipc_config *target_ipc_config =
                (struct server_ipc_config *)(target->general_ipc_config);
        u64 callback = target_ipc_config->callback;

        conn->source = current_thread;
        current_thread->thread_ctx->state = TS_WAITING;
        target->active_conn = conn;
        obj_put(conn);

        /**
         * Lab4
         * This command set the sp register, read the file to find which field
         * of the ipc_connection stores the stack of the server thread?
         * */
        /* LAB 4 TODO BEGIN: use arch_set_thread_stack*/
        arch_set_thread_stack(target, conn->server_stack_top);
        /* LAB 4 TODO END */

        /**
         * Lab4
         * This command set the ip register, read the file to find which field
         * of the ipc_connection stores the instruction to be called when switch
         * to the server?
         * */
        /* LAB 4 TODO BEGIN: use arch_set_thread_next_ip */
        arch_set_thread_next_ip(target, callback);
        /* LAB 4 TODO END */

        /**
         * Lab4
         * The argument set by sys_ipc_call;
         * first arg: arg, second arg: pid
         */

        /* LAB 4 TODO BEGIN: use arch_set_thread_arg0/1 */
        arch_set_thread_arg0(target, arg);
        arch_set_thread_arg1(target, current_thread->cap_group->pid);
        /* LAB 4 TODO END */

        /**
         * Passing the scheduling context of the current thread to thread of
         * connection
         */
        target->thread_ctx->sc = current_thread->thread_ctx->sc;

        /**
         * Switch to the server
         */
        switch_to_thread(target);
        eret_to_thread(switch_context());

        /* Function never return */
        BUG_ON(1);
        return 0;
}

/**
 * Lab4
 * Helper function
 * Server thread calls this function and then return to client thread
 * This function should never return
 *
 * Replace the place_holder to correct value!
 */
static int thread_migrate_to_client(struct ipc_connection *conn, u64 ret_value)
{
        struct thread *source = conn->source;
        current_thread->active_conn = NULL;

        /**
         * Lab4
         * The return value returned by server thread;
         */
        /* LAB 4 TODO BEGIN: use arch_set_thread_return */
        arch_set_thread_return(source, ret_value);
        /* LAB 4 TODO END */

        /**
         * Switch to the client
         */
        switch_to_thread(source);
        eret_to_thread(switch_context());

        /* Function never return */
        BUG_ON(1);
        return 0;
}

/**
 * lab4: Transfer cap to server
 */
int ipc_send_cap(struct ipc_connection *conn)
{
        int i, r;
        u64 cap_slot_number;
        u64 cap_slots_offset;
        u64 *cap_buf;
        ipc_msg_t *ipc_msg = conn->ipc_msg;

        r = copy_from_user((char *)&cap_slot_number,
                           (char *)&ipc_msg->cap_slot_number,
                           sizeof(cap_slot_number));
        if (r < 0)
                goto out;
        if (likely(cap_slot_number == 0)) {
                r = 0;
                goto out;
        } else if (cap_slot_number >= MAX_CAP_TRANSFER) {
                r = -EINVAL;
                goto out;
        }

        r = copy_from_user((char *)&cap_slots_offset,
                           (char *)&ipc_msg->cap_slots_offset,
                           sizeof(cap_slots_offset));
        if (r < 0)
                goto out;

        cap_buf = kmalloc(cap_slot_number * sizeof(*cap_buf));
        if (!cap_buf) {
                r = -ENOMEM;
                goto out;
        }

        r = copy_from_user((char *)cap_buf,
                           (char *)ipc_msg + cap_slots_offset,
                           sizeof(*cap_buf) * cap_slot_number);
        if (r < 0)
                goto out;

        for (i = 0; i < cap_slot_number; i++) {
                u64 dest_cap;

                /* Lab4: copy the cap to server and update the cap_buf */
                /* BLANK END */
                /* LAB 4 TODO END */
                dest_cap = cap_copy(
                        current_cap_group, conn->target->cap_group, cap_buf[i]);
                if (dest_cap < 0)
                        goto out_free_cap;
                cap_buf[i] = dest_cap;
                /* BLANK END */
                /* LAB 4 TODO END */
        }

        r = copy_to_user((char *)ipc_msg + cap_slots_offset,
                         (char *)cap_buf,
                         sizeof(*cap_buf) * cap_slot_number);
        if (r < 0)
                goto out_free_cap;

        kfree(cap_buf);
        return 0;

out_free_cap:
        for (--i; i >= 0; i--)
                cap_free(conn->target->cap_group, cap_buf[i]);
        kfree(cap_buf);
out:
        return r;
}

/**
 * lab4: Transfer cap back to client
 */
static void ipc_send_cap_to_client(struct ipc_connection *conn, u64 cap_num)
{
        int r, i;
        u64 ret_cap;
        struct ipc_msg *server_ipc_msg;
        struct ipc_msg *ipc_msg = conn->ipc_msg;

        if (cap_num == 0)
                return;

        server_ipc_msg =
                (struct ipc_msg *)((u64)ipc_msg - conn->buf.client_user_addr
                                   + conn->buf.server_user_addr);

        for (i = 0; i < cap_num; ++i) {
                r = copy_from_user((char *)&ret_cap,
                                   (char *)server_ipc_msg
                                           + server_ipc_msg->cap_slots_offset
                                           + sizeof(ret_cap) * i,
                                   sizeof(ret_cap));
                BUG_ON(r < 0);

                ret_cap = cap_copy(
                        current_cap_group, conn->source->cap_group, ret_cap);
                BUG_ON(ret_cap < 0);
                r = copy_to_user((char *)server_ipc_msg
                                         + server_ipc_msg->cap_slots_offset
                                         + sizeof(ret_cap) * i,
                                 (char *)&ret_cap,
                                 sizeof(ret_cap));
                BUG_ON(r < 0);
        }
}

/* IPC related system calls */

/* Lab4: Register server */
u64 sys_register_server(u64 callback, u64 max_client, u64 vm_config_ptr)
{
        struct thread *server = current_thread;
        struct server_ipc_config *server_ipc_config;
        struct ipc_vm_config *vm_config;
        int r;
        BUG_ON(server == NULL);

        // Create the server ipc_config
        server_ipc_config = kmalloc(sizeof(struct server_ipc_config));
        if (!server_ipc_config) {
                r = -ENOMEM;
                goto out_fail;
        }
        // Init the server ipc_config
        server_ipc_config->callback = callback;
        if (max_client > IPC_MAX_CONN_PER_SERVER) {
                r = -EINVAL;
                goto out_free_server_ipc_config;
        }
        server_ipc_config->max_client = max_client;
        server_ipc_config->conn_bmp =
                kzalloc(BITS_TO_LONGS(max_client) * sizeof(long));
        if (!server_ipc_config->conn_bmp) {
                r = -ENOMEM;
                goto out_free_server_ipc_config;
        }
        // Get and check the parameter vm_config
        vm_config = &server_ipc_config->vm_config;
        r = copy_from_user(
                (char *)vm_config, (char *)vm_config_ptr, sizeof(*vm_config));
        if (r < 0)
                goto out_free_conn_bmp;
        if (!is_user_addr_range(vm_config->stack_base_addr,
                                vm_config->stack_size)
            || !is_user_addr_range(vm_config->buf_base_addr,
                                   vm_config->buf_size)
            || !IS_ALIGNED(vm_config->stack_base_addr, PAGE_SIZE)
            || !IS_ALIGNED(vm_config->stack_size, PAGE_SIZE)
            || !IS_ALIGNED(vm_config->buf_base_addr, PAGE_SIZE)
            || !IS_ALIGNED(vm_config->buf_size, PAGE_SIZE)) {
                r = -EINVAL;
                goto out_free_conn_bmp;
        }
        // Set general_ipc_config means succ register server
        server->general_ipc_config = server_ipc_config;
        return r;

out_free_conn_bmp:
        kfree(server_ipc_config->conn_bmp);
out_free_server_ipc_config:
        kfree(server_ipc_config);
out_fail:
        return r;
}

/* Lab4: Register client */
u32 sys_register_client(u32 server_cap, u64 vm_config_ptr)
{
        struct thread *client = current_thread;
        struct thread *server = NULL;
        struct ipc_connection *conn;
        struct ipc_vm_config vm_config = {0};
        u64 client_buf_size;
        int conn_cap = 0;
        int r = 0;

        r = copy_from_user(
                (char *)&vm_config, (char *)vm_config_ptr, sizeof(vm_config));
        if (r < 0)
                goto out_fail;
        if (!is_user_addr_range(vm_config.buf_base_addr, vm_config.buf_size)
            || !IS_ALIGNED(vm_config.buf_base_addr, PAGE_SIZE)
            || !IS_ALIGNED(vm_config.buf_size, PAGE_SIZE)) {
                r = -EINVAL;
                goto out_fail;
        }

        server = obj_get(current_cap_group, server_cap, TYPE_THREAD);
        if (!server) {
                r = -ECAPBILITY;
                goto out_fail;
        }
        if (!server->general_ipc_config) {
                /* Not ready */
                r = -EIPCRETRY;
                goto out_fail;
        }

        client_buf_size = vm_config.buf_size;
        conn_cap = create_connection(client, server, &vm_config);
        if (conn_cap < 0) {
                r = conn_cap;
                goto out_obj_put_thread;
        }

        conn = obj_get(current_cap_group, conn_cap, TYPE_CONNECTION);

        if (client_buf_size != vm_config.buf_size) {
                r = copy_to_user((char *)vm_config_ptr,
                                 (char *)&vm_config,
                                 sizeof(vm_config));
                if (r < 0)
                        goto out_obj_put_conn;
        }

        r = conn_cap;

out_obj_put_conn:
        obj_put(conn);
out_obj_put_thread:
        obj_put(server);
out_fail:
        return r;
}

/**
 * lab4: Return from a ipc handler thread
 * The thread of ipc_connection calls sys_ipc_return
 * you should migrate to the client now.
 * This function should never return!
 */
void sys_ipc_return(u64 ret, u64 cap_num)
{
        struct ipc_connection *conn = current_thread->active_conn;

        if (conn == NULL) {
                WARN("An inactive thread calls ipc_return\n");
                goto out;
        }

        if (cap_num != 0) {
                ipc_send_cap_to_client(conn, cap_num);
        }

        /* Lab4: update the thread's state and sc */
        /* LAB 4 TODO BEGIN */
        conn->source->thread_ctx->state = TS_RUNNING;
        conn->source->thread_ctx->sc = current_thread->thread_ctx->sc;
        /* LAB 4 TODO END */

        thread_migrate_to_client(conn, ret);
        BUG("This function should never\n");
out:
        return;
}

/*
 * lab4: Issue an IPC request
 * 1. Get the conection structure from the cap.
 * 2. If IPC msg contains cap transfer, transfer it to server.
 * 3. IPC-msg is based on shared memory, calculate the correct offset.
 * 4. Migrate to server and set the correct thread states.
 */
u64 sys_ipc_call(u32 conn_cap, struct ipc_msg *ipc_msg, u64 cap_num)
{
        struct ipc_connection *conn = NULL;
        u64 arg;
        int r = 0;

        conn = obj_get(current_thread->cap_group, conn_cap, TYPE_CONNECTION);
        if (!conn) {
                r = -ECAPBILITY;
                goto out_fail;
        }
        conn->ipc_msg = ipc_msg;

        /**
         * Lab4
         * Here, you need to transfer all the capbiliies of client thread to
         * capbilities in server thread in the ipc_msg if cap_num > 0
         */
        /* LAB 4 TODO BEGIN: use ipc_send_cap */
        if (cap_num > 0) {
                r = ipc_send_cap(conn);
                if (r < 0)
                        goto out_obj_put;
        }
        /* LAB 4 TODO END */

        if (ipc_msg == 0)
                thread_migrate_to_server(conn, 0);

        /**
         * Lab4
         * The arg is actually the 64-bit arg for ipc_dispatcher
         * Then what value should the arg be?
         * */
        /* LAB 4 TODO BEGIN: set arg */
        arg = conn->buf.server_user_addr;
        /* LAB 4 TODO END */

        thread_migrate_to_server(conn, arg);

        BUG("This function should never\n");
out_obj_put:
        obj_put(conn);
out_fail:
        return r;
}
