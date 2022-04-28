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

#include "tmpfs.h"
#include "tmpfs_ops.h"

#define TEST_FUNC(name) \
    do { \
        if (name() == 0) { \
            printf(#name" pass!\n"); \
        } else { \
            printf(#name" fail!\n"); \
        } \
    } while (0)

extern const char __binary_ramdisk_cpio_start;
extern u64 __binary_ramdisk_cpio_size;

/* Retrieve the entry name from one dirent */
static void get_dent_name(struct dirent *p, char name[])
{
	int len;
	len = p->d_reclen - sizeof(p->d_ino) - sizeof(p->d_off)
	      - sizeof(p->d_reclen) - sizeof(p->d_type);
	memcpy(name, p->d_name, len);
	name[len - 1] = '\0';
}


static void list_dir(struct inode * inode)  {
    char buf[4096];
    char name[4096];
    int readbytes = 0;
    int offset = 0;
	struct dirent *p;
    char* scan_buf = buf;
    tfs_scan(inode, 0, buf, (char*)buf + sizeof(buf), &readbytes);
    for (offset = 0; offset < readbytes; offset += p->d_reclen) {
		p = (struct dirent *)(scan_buf + offset);
		get_dent_name(p, name);
		printf("Testdir-%s ", name);
	}
}

int test_mkdir() {
    int err;
    err = tmpfs_mkdir("/test_dir", 0);
    if(err) {
        return err;
    }
    err = tmpfs_mkdir("/test_dir/test_dir2", 0);
    if(err) {
        return err;
    }
    err = tmpfs_mkdir("/test_dir/test_dir3", 0);
    struct inode * inode = tfs_open_path("/test_dir");
    if(!inode) {
        return -1;
    }
    list_dir(inode);
 
    return err;
}

#define FILE_LEN (PAGE_SIZE * 2)

int test_read_write() {
    char *buf[2];
    struct inode * inode = tfs_open_path("/test_dir/tmp.txt");
    if(inode < 0) {
        return -1;
    }
	buf[0] = malloc(FILE_LEN);
    memset(buf[0], 'x', FILE_LEN);
	buf[1] = malloc(FILE_LEN);
	int ret = tfs_file_write(inode, 0, buf[0], FILE_LEN);
	if (ret != FILE_LEN) {
		printf("write ret=%x len=%x\n", ret, FILE_LEN);
		return -1;
	}
	ret = tfs_file_read(inode, 0, buf[1], FILE_LEN);
	if (ret != FILE_LEN) {
		printf("read ret=%x len=%x\n", ret, FILE_LEN);
		return -1;
	}
    ret = memcmp(buf[0], buf[1], FILE_LEN); 
    free(buf[0]);
    free(buf[1]);
	return ret;
}

int test_create() {
    int err = fs_creat("/test_dir/tmp.txt");
    printf("test_create err is %d\n", err);
    struct inode * inode = tfs_open_path("/test_dir/tmp.txt");
    if(!inode) {
        return -1;
    }
    return err;
}

int test_unlink() {
    tmpfs_unlink("/test_dir/tmp.txt", 0);
    if(tfs_open_path("/test_dir/tmp.txt")) {
        return -1;
    }
    tmpfs_rmdir("/test_dir/test_dir2", 0);
    tmpfs_rmdir("/test_dir/test_dir3", 0);
    tmpfs_rmdir("/test_dir", 0);
    if(tfs_open_path("/test_dir") || tfs_open_path("/test_dir/test_dir2") 
        || tfs_open_path("/test_dir/test_dir3")) {
            return -1;
        }

    return 0;
}

int test_tfs_load_image() {
    tfs_load_image(&__binary_ramdisk_cpio_start);
    struct inode * inode1 = tfs_open_path("/lab5.bin");
    struct inode * inode2 = tfs_open_path("/helloworld.bin");
    struct inode * inode3 = tfs_open_path("/test.txt");
    if(!inode1 || !inode2 || !inode3 || (inode3 && inode3->size != 11)) {
        return -1;
    }
    return 0;
}

void tfs_test() {
    TEST_FUNC(test_tfs_load_image);
    TEST_FUNC(test_mkdir);
    TEST_FUNC(test_create);
    TEST_FUNC(test_read_write);
    TEST_FUNC(test_unlink); 
}


