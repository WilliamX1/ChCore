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

#include <chcore/fsm.h>
#include <chcore/ipc.h>
#include <chcore/assert.h>
#include <chcore/internal/server_caps.h>
#include <chcore/fs/defs.h>

static struct ipc_struct *fakefs_ipc_struct = NULL;

static void connect_fakefs_server(int fakefs_cap)
{
        fakefs_ipc_struct = ipc_register_client(fakefs_cap);
        chcore_assert(fakefs_ipc_struct);
}

int chcore_fakefs_test(int fakefs_cap) {
    if (!fakefs_ipc_struct) {
        connect_fakefs_server(fakefs_cap);
    }

    {   //  Creat test
        struct ipc_msg *ipc_msg = ipc_create_msg(
                fakefs_ipc_struct, sizeof(struct fs_request), 0);
        chcore_assert(ipc_msg);
        struct fs_request * fr =
                (struct fs_request *)ipc_get_msg_data(ipc_msg);
        fr->req = FS_REQ_CREAT;
        strcpy(fr->creat.pathname, "/test.txt");
        int ret = ipc_call(fakefs_ipc_struct, ipc_msg);
        ipc_destroy_msg(fakefs_ipc_struct, ipc_msg);
        if(ret < 0) {
                return ret;
        }
    }

    {   //  Creat test
        struct ipc_msg *ipc_msg = ipc_create_msg(
                fakefs_ipc_struct, sizeof(struct fs_request), 0);
        chcore_assert(ipc_msg);
        struct fs_request * fr =
                (struct fs_request *)ipc_get_msg_data(ipc_msg);
        fr->req = FS_REQ_CREAT;
        strcpy(fr->creat.pathname, "/hello.txt");
        int ret = ipc_call(fakefs_ipc_struct, ipc_msg);
        ipc_destroy_msg(fakefs_ipc_struct, ipc_msg);
        if(ret < 0) {
                return ret;
        }
    }

    {   //  Creat test
        struct ipc_msg *ipc_msg = ipc_create_msg(
                fakefs_ipc_struct, sizeof(struct fs_request), 0);
        chcore_assert(ipc_msg);
        struct fs_request * fr =
                (struct fs_request *)ipc_get_msg_data(ipc_msg);
        fr->req = FS_REQ_CREAT;
        strcpy(fr->creat.pathname, "/world.txt");
        int ret = ipc_call(fakefs_ipc_struct, ipc_msg);
        ipc_destroy_msg(fakefs_ipc_struct, ipc_msg);
        if(ret < 0) {
                return ret;
        }
    }

    int file_fd = 3;
    { // Open file
        struct ipc_msg *ipc_msg = ipc_create_msg(
                fakefs_ipc_struct, sizeof(struct fs_request), 0);
        chcore_assert(ipc_msg);
        struct fs_request * fr =
                (struct fs_request *)ipc_get_msg_data(ipc_msg);
        fr->req = FS_REQ_OPEN;
        strcpy(fr->open.pathname, "/test.txt");
        fr->open.flags = O_WRONLY;
        fr->open.new_fd = file_fd;
        int ret = ipc_call(fakefs_ipc_struct, ipc_msg);
        ipc_destroy_msg(fakefs_ipc_struct, ipc_msg);
        if(ret < 0) {
                return ret;
        }

    }

    { // Write file
        char* buf = "test file!";
        int cnt = strlen(buf) + 1;
        struct ipc_msg *ipc_msg = ipc_create_msg(
                fakefs_ipc_struct, sizeof(struct fs_request) + cnt + 1, 0);
        chcore_assert(ipc_msg);
        struct fs_request * fr =
                (struct fs_request *)ipc_get_msg_data(ipc_msg);

        fr->req = FS_REQ_WRITE;
        fr->write.fd = file_fd;
        fr->write.count = cnt;
        ipc_set_msg_data(ipc_msg, buf, sizeof(struct fs_request), cnt + 1);

        int ret = ipc_call(fakefs_ipc_struct, ipc_msg);

        ipc_destroy_msg(fakefs_ipc_struct, ipc_msg);

        if(ret < 0) {
            return ret;
        }
    }


    {   //  Open test
        struct ipc_msg *ipc_msg = ipc_create_msg(
                fakefs_ipc_struct, sizeof(struct fs_request), 0);
        chcore_assert(ipc_msg);
        struct fs_request * fr =
                (struct fs_request *)ipc_get_msg_data(ipc_msg);
        fr->req = FS_REQ_LSEEK;
        fr->lseek.fd = file_fd;
        fr->lseek.offset = 0;
        fr->lseek.whence = 0;
        int ret = ipc_call(fakefs_ipc_struct, ipc_msg);
        ipc_destroy_msg(fakefs_ipc_struct, ipc_msg);
        if(ret < 0) {
                return ret;
        }
    }

    {
        char buf [258];
	    int cnt = 256;
        int ret = 0;
        int p = 0;
        do{
                struct ipc_msg *ipc_msg = ipc_create_msg(
                    fakefs_ipc_struct, sizeof(struct fs_request) + cnt + 2, 0);
                chcore_assert(ipc_msg);
                struct fs_request * fr =
                    (struct fs_request *)ipc_get_msg_data(ipc_msg);
                fr->req = FS_REQ_READ;
                fr->read.fd = file_fd;
                fr->read.count = cnt;

                ret = ipc_call(fakefs_ipc_struct, ipc_msg);
                if(ret > 0) {
	                memcpy(buf + p, ipc_get_msg_data(ipc_msg), ret);
                        p += ret;
                }
                ipc_destroy_msg(fakefs_ipc_struct, ipc_msg);
        } while(ret > 0);
    }

    {   //  Creat test
        struct ipc_msg *ipc_msg = ipc_create_msg(
                fakefs_ipc_struct, sizeof(struct fs_request), 0);
        chcore_assert(ipc_msg);
        struct fs_request * fr =
                (struct fs_request *)ipc_get_msg_data(ipc_msg);
        fr->req = FS_REQ_MKDIR;
        strcpy(fr->mkdir.pathname, "/testdir");
        int ret = ipc_call(fakefs_ipc_struct, ipc_msg);
        ipc_destroy_msg(fakefs_ipc_struct, ipc_msg);
        if(ret < 0) {
                return ret;
        }
    }

    {   //  mkdir test
        struct ipc_msg *ipc_msg = ipc_create_msg(
                fakefs_ipc_struct, sizeof(struct fs_request), 0);
        chcore_assert(ipc_msg);
        struct fs_request * fr =
                (struct fs_request *)ipc_get_msg_data(ipc_msg);
        fr->req = FS_REQ_MKDIR;
        strcpy(fr->mkdir.pathname, "/testdir2");
        int ret = ipc_call(fakefs_ipc_struct, ipc_msg);
        ipc_destroy_msg(fakefs_ipc_struct, ipc_msg);
        if(ret < 0) {
                return ret;
        }
    }

    {   //  Creat test
        struct ipc_msg *ipc_msg = ipc_create_msg(
                fakefs_ipc_struct, sizeof(struct fs_request), 0);
        chcore_assert(ipc_msg);
        struct fs_request * fr =
                (struct fs_request *)ipc_get_msg_data(ipc_msg);
        fr->req = FS_REQ_CREAT;
        strcpy(fr->creat.pathname, "/testdir/hello.txt");
        int ret = ipc_call(fakefs_ipc_struct, ipc_msg);
        ipc_destroy_msg(fakefs_ipc_struct, ipc_msg);
        if(ret < 0) {
                return ret;
        }
    }

    {   //  Creat test
        struct ipc_msg *ipc_msg = ipc_create_msg(
                fakefs_ipc_struct, sizeof(struct fs_request), 0);
        chcore_assert(ipc_msg);
        struct fs_request * fr =
                (struct fs_request *)ipc_get_msg_data(ipc_msg);
        fr->req = FS_REQ_CREAT;
        strcpy(fr->creat.pathname, "/testdir/world.txt");
        int ret = ipc_call(fakefs_ipc_struct, ipc_msg);
        ipc_destroy_msg(fakefs_ipc_struct, ipc_msg);
        if(ret < 0) {
                return ret;
        }
    }

    { // Get size
        struct ipc_msg *ipc_msg = ipc_create_msg(
                fakefs_ipc_struct, sizeof(struct fs_request), 0);
        chcore_assert(ipc_msg);
        struct fs_request * fr =
                (struct fs_request *)ipc_get_msg_data(ipc_msg);
        fr->req = FS_REQ_GET_SIZE;
        strcpy(fr->getsize.pathname, "/test.txt");
        int ret = ipc_call(fakefs_ipc_struct, ipc_msg);
        ipc_destroy_msg(fakefs_ipc_struct, ipc_msg);
        if(ret < 0) {
                return ret;
        }

    }

    return 0;
}