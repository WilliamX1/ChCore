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

#include <chcore/assert.h>
#include "fs_defs.h"
#include "fcntl.h"
#include "list.h"
#include "radix.h"
#include "hashtable.h"
#include <stdio.h>
#include <sys/types.h>

#define MAX_ELF_SIZE 128*128
#define MAX_FILENAME_LEN (255)
#define MAX_NR_FID_RECORDS 1024

#define FS_REG (1)
#define FS_DIR (2)

/* special errno branch number */
#define E_NAMEX_NOENT 201

#define PREFIX "[tmpfs]"
#define info(fmt, ...) printf(PREFIX " " fmt, ##__VA_ARGS__)
#if 0
#define debug(fmt, ...) \
	printf(PREFIX "<%s:%d>: " fmt, __func__, __LINE__, ##__VA_ARGS__)
#else
#define debug(fmt, ...) do { } while (0)
#endif
#define warn(fmt, ...) printf(PREFIX " " fmt, ##__VA_ARGS__)
#define error(fmt, ...) printf(PREFIX " " fmt, ##__VA_ARGS__)

#define ROUND_UP(x, n)		(((x) + (n) - 1) & ~((n) - 1))
#define ROUND_DOWN(x, n)	((x) & ~((n) - 1))

typedef unsigned int mode_t;
typedef unsigned long long vaddr_t;

struct string {
	char *str;
	size_t len;
	u64 hash;
};

struct dentry {
	struct string name;
	struct inode *inode;
	struct hlist_node node;
	int refcnt;
};

struct inode {
	int refcnt; /* Volatile reference. */
	int nlinks; /* Links */
	u64 type;
	size_t size; 
	unsigned int mode;
	union {
		struct htable dentries;
		struct radix data;
	};

	/* Address array, used by mmap. */
	struct {
		bool valid;
		u64 *array;
		size_t nr_used; /* Number of entries filled. */
		size_t size;    /* Total capacity. */
		int translated_pmo_cap;    /* PMO cap of translated array. */
	} aarray;
};

struct super_block {
	u64 s_magic; /* 0x79F5 */
	u64 s_size;
	u64 s_root;
};

struct component {
	enum {
		/* All components store dentries. */
		COMPONENT_NORMAL = 0,
		/**
		 * If we resolving symlinks, no matter how deep,
		 * the components should be tagged with
		 * COMPONENT_SYMLINK.
		 */
		COMPONENT_SYMLINK = 1,
	} c_type;
	struct dentry *c_dentry;
};

struct path {
	/* A list of components. */
	int nr_comps;
	int max_comps;
	struct component *comps;
	// int nr_symlink_resolving;
};

/* Opened fd entry */
struct fid_record {
	struct path path;
	struct inode *inode;
	u64 flags;
	u64 offset;
};

struct dirent {                                                                                                                                     
    unsigned long d_ino;                                                                                                                                    
    long d_off;                                                                                                                                    
    unsigned short d_reclen;                                                                                                                        
    unsigned char d_type;                                                                                                                           
    char d_name[256];                                                                                                                               
};  

#ifndef PAGE_SIZE
#define PAGE_SIZE 0x1000
#endif

#define CACHED_PAGE_SIZE	4096
#define CACHED_BLOCK_SIZE	512
#define BLOCK_PER_PAGE		(CACHED_PAGE_SIZE / CACHED_BLOCK_SIZE)

#define ACTIVE_LIST_MAX		(1 << 14)
#define INACTIVE_LIST_MAX	(1 << 14)
#define MAX_PINNED_PAGE		512
#define MAX_PAGE_CACHE_PAGE	(ACTIVE_LIST_MAX + INACTIVE_LIST_MAX + MAX_PINNED_PAGE)

#define WRITE_BACK_CYCLE	300

typedef unsigned long pidx_t;

/* The possibilities for the third argument to `fseek'.
   These values should not be changed.  */
#define SEEK_SET	0	/* Seek from beginning of file.  */
#define SEEK_CUR	1	/* Seek from current position.  */
#define SEEK_END	2	/* Seek from end of file.  */

