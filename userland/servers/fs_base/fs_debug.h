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
#include <stdio.h>

/*
 * 0: print nothing
 */
#define FS_DEBUG 0

#define COLOR_RED       "\033[31m"
#define COLOR_GREEN     "\033[32m"
#define COLOR_YELLOW    "\033[33m"
#define COLOR_YELLOW_BG "\033[33;3m"
#define COLOR_DEFAULT   "\033[0m"

#define PRINT_HEX_PERLINE 32

#define UNUSED(x) ((void)x)

static inline void print_hex(char *buf, int size, int per_line)
{
	int i;
	for (i = 0; i < size; i++) {
		if (i % per_line == 0) {
			printf("\n");
		}
		printf("%02x ", buf[i]);
	}
	printf("\n");
}

#define colored_printf(COLOR, fmt, ...) printf(COLOR""fmt""COLOR_DEFAULT, ##__VA_ARGS__)
#define colored_printf_with_lineinfo(COLOR, fmt, ...) do { \
		colored_printf(COLOR, "<%s:%s:%d>: ", __FILE__, __func__, __LINE__);	\
		printf(fmt, ##__VA_ARGS__); \
	} while (0)

#if FS_DEBUG == 1
	#define fs_debug(fmt, ...) colored_printf_with_lineinfo(COLOR_YELLOW, fmt, ##__VA_ARGS__)
	#define fs_debug_newline() printf("\n")
	#define fs_debug_print_hex(buf, size) print_hex(buf, size, PRINT_HEX_PERLINE)
#else
	#define fs_debug(fmt, ...) do { } while (0)
	#define fs_debug_newline() do { } while (0)
	#define fs_debug_print_hex(...) do { } while (0)
#endif
 
// /* Tracing prints */
#if FS_DEBUG
	#define fs_debug_error(fmt, ...) colored_printf_with_lineinfo(COLOR_RED, fmt, ##__VA_ARGS__)
	#define fs_debug_warn(fmt, ...) colored_printf_with_lineinfo(COLOR_YELLOW_BG, fmt, ##__VA_ARGS__)
#else
	#define fs_debug_error(fmt, ...) do { } while (0)
	#define fs_debug_warn(fmt, ...) do { } while (0)
#endif

