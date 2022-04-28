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

#include <chcore/tmpfs.h>
#include <chcore/ipc.h>
#include <chcore/assert.h>
#include <chcore/internal/server_caps.h>
#include <chcore/fs/defs.h>
#include <string.h>

static struct ipc_struct *tmpfs_ipc_struct = NULL;

void connect_tmpfs_server(void)
{
        int tmpfs_cap = __chcore_get_tmpfs_cap();
        chcore_assert(tmpfs_cap >= 0);
        tmpfs_ipc_struct = ipc_register_client(tmpfs_cap);
        chcore_assert(tmpfs_ipc_struct);
}


int 
read_file_from_tfs(const char* path, char* buf) {

        int p = 0;
        if (!tmpfs_ipc_struct) {
                connect_tmpfs_server();
        }

        int file_fd = 1;

        {        //  Open test
           struct ipc_msg *ipc_msg = ipc_create_msg(
                   tmpfs_ipc_struct, sizeof(struct fs_request), 0);
           chcore_assert(ipc_msg);
           struct fs_request * fr =
                   (struct fs_request *)ipc_get_msg_data(ipc_msg);
           fr->req = FS_REQ_OPEN;
           strcpy(fr->open.pathname, path);
           fr->open.flags = O_RDONLY;
           fr->open.new_fd = file_fd;
           int ret = ipc_call(tmpfs_ipc_struct, ipc_msg);
           ipc_destroy_msg(tmpfs_ipc_struct, ipc_msg);
           if(ret < 0) {
                   return ret;
           }
        }

        {
	int cnt = 256;
        int ret = 0;
        do{
            struct ipc_msg *ipc_msg = ipc_create_msg(
                    tmpfs_ipc_struct, sizeof(struct fs_request) + cnt + 2, 0);
            chcore_assert(ipc_msg);
            struct fs_request * fr =
                    (struct fs_request *)ipc_get_msg_data(ipc_msg);
            fr->req = FS_REQ_READ;
            fr->read.fd = file_fd;
            fr->read.count = cnt;
            ret = ipc_call(tmpfs_ipc_struct, ipc_msg);
            if(ret > 0) {
                memcpy(buf + p, ipc_get_msg_data(ipc_msg), ret);
                p += ret;
            }
            ipc_destroy_msg(tmpfs_ipc_struct, ipc_msg);
        } while(ret > 0);
        // printf("read_file_from_tfs read %d bytes \n", ret);
        }

        {
                struct ipc_msg *ipc_msg = ipc_create_msg(
                        tmpfs_ipc_struct, sizeof(struct fs_request), 0);
                chcore_assert(ipc_msg);
                struct fs_request * fr = (struct fs_request *)ipc_get_msg_data(ipc_msg);
                fr->req = FS_REQ_CLOSE;
                fr->close.fd = file_fd;
                int ret = ipc_call(tmpfs_ipc_struct, ipc_msg);
                ipc_destroy_msg(tmpfs_ipc_struct, ipc_msg);
               if(ret < 0)
                       return ret;
        }
        return p;

}

int get_file_size_from_tfs(const char* path) {
        if (!tmpfs_ipc_struct) {
                connect_tmpfs_server();
        }
        struct ipc_msg *ipc_msg = ipc_create_msg(
                tmpfs_ipc_struct, sizeof(struct fs_request), 0);
        chcore_assert(ipc_msg);
        struct fs_request * fr =
                (struct fs_request *)ipc_get_msg_data(ipc_msg);

        fr->req = FS_REQ_GET_SIZE;
        strcpy(fr->getsize.pathname, path);

        int ret = ipc_call(tmpfs_ipc_struct, ipc_msg);
        ipc_destroy_msg(tmpfs_ipc_struct, ipc_msg);
        if(ret < 0) {
                return ret;
        }
        return ret;
}


