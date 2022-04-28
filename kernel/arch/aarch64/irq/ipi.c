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

void arch_send_ipi(u32 cpu, u32 ipi)
{
        plat_send_ipi(cpu, ipi);
}

int handle_ipi(u32 ipi)
{
        switch (ipi) {
        default:
                kwarn("Unknow IPI %d\n", ipi);
                return -1;
        }
}

void handle_ipi_request_to_local_cpu(struct ipi_data *data_self)
{
        /* Do nothing on AArch64 now. */
}
