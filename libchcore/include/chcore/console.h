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

/*
 * ChCore APIs for debug console, i.e.
 * the UART device drived by the kernel.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void chcore_console_putc(int ch);
int chcore_console_getc(void);

void chcore_console_puts(const char *str);
int chcore_console_printf(const char *fmt, ...);

#ifdef __cplusplus
}
#endif
