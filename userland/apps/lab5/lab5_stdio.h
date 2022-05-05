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

#include <stdio.h>
#include <string.h>
#include <chcore/types.h>
#include <chcore/fsm.h>
#include <chcore/tmpfs.h>
#include <chcore/ipc.h>
#include <chcore/internal/raw_syscall.h>
#include <chcore/internal/server_caps.h>
#include <chcore/procm.h>
#include <chcore/fs/defs.h>

extern struct ipc_struct *tmpfs_ipc_struct;

typedef struct FILE {
	/* LAB 5 TODO BEGIN */
	int fd;
	unsigned int mode;
	char filename[FS_REQ_PATH_BUF_LEN];
	int offset;
	int refcnt;
	/* LAB 5 TODO END */
} FILE;

FILE *fopen(const char * filename, const char * mode);

size_t fwrite(const void * src, size_t size, size_t nmemb, FILE * f);

size_t fread(void * destv, size_t size, size_t nmemb, FILE * f);

int fclose(FILE *f);

int fscanf(FILE * f, const char * fmt, ...);

int fprintf(FILE * f, const char * fmt, ...);

typedef __builtin_va_list va_list;
#define va_start(v, l) __builtin_va_start(v, l)
#define va_end(v)      __builtin_va_end(v)
#define va_arg(v, l)   __builtin_va_arg(v, l)
#define va_copy(d, s)  __builtin_va_copy(d, s)
