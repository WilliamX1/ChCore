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

#include <sync/spin.h>
#include <chcore/internal/raw_syscall.h>

void spinlock_init(struct spinlock *lock)
{
        lock->val = 0;
}

void spinlock_lock(struct spinlock *lock)
{
        while (__sync_lock_test_and_set(&lock->val, 1)) {
                while (lock->val != 0)
                        __chcore_sys_yield();
        }
}

void spinlock_unlock(struct spinlock *lock)
{
        __sync_lock_release(&lock->val);
}