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

#include <irq/irq.h>
#include <common/types.h>
#include <arch/ipi.h>

void arch_send_ipi(u32 cpu, u32 ipi);
int handle_ipi(u32 ipi);

/* 7 u64 arg and 2 u32 (start/finish, vector) occupy one cacheline */
#define IPI_DATA_ARG_NUM (7)

struct ipi_data {
        /* start  <- 1: the ipi_data (argments) is ready */
        volatile u16 start;
        /* finish <- 1: the ipi_data (argments) is handled */
        volatile u16 finish;

        /* The IPI_vector */
        u32 vector;

        u64 args[IPI_DATA_ARG_NUM];
};

extern struct ipi_data global_ipi_data[];

void init_ipi_data(void);

/* IPI interfaces for achieving cross-core communication */

/* Sender side */
void prepare_ipi_tx(u32 target_cpu);
void set_ipi_tx_arg(u32 target_cpu, u32 arg_index, u64 val);
void start_ipi_tx(u32 target_cpu, u32 ipi_vector);
void wait_finish_ipi_tx(u32 target_cpu);

/* Receiver side */
u64 get_ipi_tx_arg(u32 target_cpu, u32 arg_index);
void mark_finish_ipi_tx(u32 target_cpu);
void handle_ipi_on_tlb_shootdown(void);

void handle_ipi_request_to_local_cpu(struct ipi_data *);
