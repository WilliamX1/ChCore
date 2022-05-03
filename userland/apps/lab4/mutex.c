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

#include <chcore/internal/raw_syscall.h>
#include <sync/spin.h>
#include "mutex.h"
#include <stdio.h>

void lock_init(struct lock *lock)
{
        /* LAB 4 TODO BEGIN */
        lock->lock_sem = __chcore_sys_create_sem();
        __chcore_sys_signal_sem(lock->lock_sem);
        /* LAB 4 TODO END */
}

void lock(struct lock *lock)
{
        /* LAB 4 TODO BEGIN */
        __chcore_sys_wait_sem(lock->lock_sem, true);
        /* LAB 4 TODO END */
}

void unlock(struct lock *lock)
{
        /* LAB 4 TODO BEGIN */
        __chcore_sys_signal_sem(lock->lock_sem);
        /* LAB 4 TODO END */
}
