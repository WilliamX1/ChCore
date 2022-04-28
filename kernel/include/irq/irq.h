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

#include <common/types.h>
#include <object/thread.h>

#include <irq_num.h>

#define HANDLE_KERNEL 0
#define HANDLE_USER   1
extern u8 irq_handle_type[MAX_IRQ_NUM];

void eret_to_thread(u64 sp); /* in arch/xxx/irq/irq_entry.S */
void arch_interrupt_init(void); /* in arch/xxx/irq/irq_entry.c */
void arch_interrupt_init_per_cpu(void); /* in arch/xxx/irq/irq_entry.c */
void arch_enable_irqno(int irqno);
void arch_disable_irqno(int irqno);

void plat_handle_irq(void); /* in arch/xxx/plat/xxx/irq/irq.c */
void plat_interrupt_init(void); /* in arch/xxx/plat/xxx/irq/irq.c */
void plat_disable_timer(void);
void plat_enable_timer(void);
void plat_enable_irqno(int irq);
void plat_disable_irqno(int irq);

void plat_ack_irq(int irq);
