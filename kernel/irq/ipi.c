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

#include <irq/ipi.h>
#include <machine.h>
#include <common/kprint.h>

/* Global IPI_data shared among all the CPUs */
struct ipi_data global_ipi_data[PLAT_CPU_NUM];

/* Invoked once during the kernel boot */
void init_ipi_data(void)
{
        int i;
        struct ipi_data *data;

        for (i = 0; i < PLAT_CPU_NUM; ++i) {
                data = &(global_ipi_data[i]);
                data->start = 0;
                data->finish = 0;
        }
}

/*
 * Interfaces for inter-cpu communication (named IPI_transaction).
 * IPI-based message sending.
 */

void prepare_ipi_tx(u32 target_cpu)
{
}

/* Set argments */
void set_ipi_tx_arg(u32 target_cpu, u32 arg_index, u64 val)
{
        struct ipi_data *data;

        data = &(global_ipi_data[target_cpu]);
        data->args[arg_index] = val;
}

/*
 * Start IPI-based transaction (tx).
 *
 * ipi_vector can be encoded into the physical interrupt (as IRQ number),
 * which can be used to implement some fast-path (simple) communication.
 *
 * Nevertheless, we support sending information with IPI.
 * So, actually, we can use one ipi_vector to distinguish different IPIs.
 */
void start_ipi_tx(u32 target_cpu, u32 ipi_vector)
{
        struct ipi_data *data;

        data = &(global_ipi_data[target_cpu]);
        /* Set ipi_vector */
        data->vector = ipi_vector;
        /* Mark the arguments are ready (set_ipi_tx_arg before) */
        data->start = 1;

        /* Send physical IPI to interrupt the target CPU */
        arch_send_ipi(target_cpu, ipi_vector);
}

/* Wait and unlock */
void wait_finish_ipi_tx(u32 target_cpu)
{
        /*
         * It is possible that core-A is waiting for core-B to finish one IPI
         * while core-B is also waiting for core-A to finish one IPI.
         * So, this function will polling on the IPI data of both the target
         * core and the local core, namely data_target and data_self.
         */
        struct ipi_data *data_target, *data_self;

        data_target = &(global_ipi_data[target_cpu]);
        data_self = &(global_ipi_data[smp_get_cpu_id()]);

        /* Wait untill finish */
        while (data_target->finish != 1) {
                if (data_self->start == 1) {
                        handle_ipi_request_to_local_cpu(data_self);
                        /*
                         * This function will poll on this varible.
                         * No data race since the IPI sender will also set this
                         * field to 0.
                         */
                        data_self->start = 0;
                }
        }

        /* Reset start/finish */
        data_target->start = 0;
        data_target->finish = 0;
}

/* Unlock only */
void finish_ipi_tx(u32 target_cpu)
{
}

/*
 * Receiver side interfaces.
 * Note that target_cpu is the receiver itself.
 */

/* Get argments */
u64 get_ipi_tx_arg(u32 target_cpu, u32 arg_index)
{
        struct ipi_data *data;

        data = &(global_ipi_data[target_cpu]);
        return data->args[arg_index];
}

/* Mark the receiver (i.e., target_cpu) has handled the tx */
void mark_finish_ipi_tx(u32 target_cpu)
{
        struct ipi_data *data;

        data = &(global_ipi_data[target_cpu]);
        data->finish = 1;
}
