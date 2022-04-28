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

#include <sched/sched.h>

struct thread_ctx *create_thread_ctx(u32 type);
void destroy_thread_ctx(struct thread *thread);
void init_thread_ctx(struct thread *thread, u64 stack, u64 func, u32 prio,
                     u32 type, s32 aff);

void arch_set_thread_stack(struct thread *thread, u64 stack);
void arch_set_thread_return(struct thread *thread, u64 ret);
u64 arch_get_thread_stack(struct thread *thread);
void arch_set_thread_next_ip(struct thread *thread, u64 ip);
u64 arch_get_thread_next_ip(struct thread *thread);
void arch_set_thread_info_page(struct thread *thread, u64 info_page_addr);
void arch_set_thread_arg0(struct thread *thread, u64 arg);
void arch_set_thread_arg1(struct thread *thread, u64 pid);
void set_thread_arch_spec_state(struct thread *thread);

void arch_enable_interrupt(struct thread *thread);
void arch_disable_interrupt(struct thread *thread);
void arch_mask_interrupts_for_thread(struct thread *thread);
