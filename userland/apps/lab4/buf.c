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
#include "buf.h"

volatile int buffer_write_cnt = 0;
volatile int buffer_read_cnt = 0;
struct spinlock buffer_lock;
int buffer[BUF_SIZE];

void buffer_init(void)
{
        spinlock_init(&buffer_lock);
}

void buffer_add_safe(int msg)
{
        spinlock_lock(&buffer_lock);
        buffer[buffer_write_cnt] = msg;
        buffer_write_cnt = (buffer_write_cnt + 1) % BUF_SIZE;
        spinlock_unlock(&buffer_lock);
}

int buffer_remove_safe(void)
{
        spinlock_lock(&buffer_lock);
        int ret = buffer[buffer_read_cnt];
        buffer_read_cnt = (buffer_read_cnt + 1) % BUF_SIZE;
        spinlock_unlock(&buffer_lock);
        return ret;
}
