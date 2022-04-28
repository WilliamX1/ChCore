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

#include <chcore/console.h>
#include <chcore/assert.h>
#include <chcore/internal/raw_syscall.h>

void chcore_console_putc(int ch)
{
        chcore_assert(ch >= 0 && ch < (char)-1);
        __chcore_sys_putc((char)ch);
}

int chcore_console_getc(void)
{
        int ch = -1;
        while (!(ch >= 0 && ch < (char)-1))
                ch = __chcore_sys_getc();
        return ch;
}

void chcore_console_puts(const char *str)
{
        while (*str) {
                chcore_console_putc(*str++);
        }
}
