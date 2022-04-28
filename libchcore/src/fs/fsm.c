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
#include <string.h>

static struct ipc_struct * fsm_ipc_struct = NULL;
static struct list_head fs_cap_infos;

struct fs_cap_info_node {
	int fs_cap;
	ipc_struct_t *fs_ipc_struct;
	struct list_head node;
};

struct fs_cap_info_node *set_fs_cap_info(int fs_cap)
{
        struct fs_cap_info_node *n;
        n = (struct fs_cap_info_node *)malloc(sizeof(*n));
        chcore_assert(n);
        n->fs_ipc_struct = ipc_register_client(fs_cap);
        chcore_assert(n->fs_ipc_struct);
        list_add(&n->node, &fs_cap_infos);
        return n;
}

/* Search for the fs whose capability is `fs_cap`.*/
struct fs_cap_info_node *get_fs_cap_info(int fs_cap)
{
        struct fs_cap_info_node *iter;
        struct fs_cap_info_node *matched_fs = NULL;
        for_each_in_list(iter, struct fs_cap_info_node, node, &fs_cap_infos) {
                if(iter->fs_cap == fs_cap) {
                        matched_fs = iter;
                        break;
                }	
        }
        if(!matched_fs) {
                return set_fs_cap_info(fs_cap);
        }
        return matched_fs;
}


static void connect_fsm_server(void)
{
	init_list_head(&fs_cap_infos);
        int fsm_cap = __chcore_get_fsm_cap();
        chcore_assert(fsm_cap >= 0);
        fsm_ipc_struct = ipc_register_client(fsm_cap);
        chcore_assert(fsm_ipc_struct);
}

int fsm_creat_file(char* path) 
{
        if (!fsm_ipc_struct) {
                connect_fsm_server();
        }
        struct ipc_msg *ipc_msg = ipc_create_msg(
                fsm_ipc_struct, sizeof(struct fs_request), 0);
        chcore_assert(ipc_msg);
        struct fs_request * fr =
                (struct fs_request *)ipc_get_msg_data(ipc_msg);
        fr->req = FS_REQ_CREAT;
        strcpy(fr->creat.pathname, path);
        int ret = ipc_call(fsm_ipc_struct, ipc_msg);
        ipc_destroy_msg(fsm_ipc_struct, ipc_msg);
        return ret;
}


int get_file_size_from_fsm(char* path) {
        if (!fsm_ipc_struct) {
                connect_fsm_server();
        }
        struct ipc_msg *ipc_msg = ipc_create_msg(
                fsm_ipc_struct, sizeof(struct fs_request), 0);
        chcore_assert(ipc_msg);
        struct fs_request * fr =
                (struct fs_request *)ipc_get_msg_data(ipc_msg);

        fr->req = FS_REQ_GET_SIZE;
        strcpy(fr->getsize.pathname, path);

        int ret = ipc_call(fsm_ipc_struct, ipc_msg);
        ipc_destroy_msg(fsm_ipc_struct, ipc_msg);
        return ret;
}

/* Write buf into the file at `path`. */
int fsm_write_file(const char* path, char* buf, unsigned long size) {
        if (!fsm_ipc_struct) {
                connect_fsm_server();
        }
        int ret = 0;

        /* LAB 5 TODO BEGIN */

        /* LAB 5 TODO END */

        return ret;
}

/* Read content from the file at `path`. */
int fsm_read_file(const char* path, char* buf, unsigned long size) {

        if (!fsm_ipc_struct) {
                connect_fsm_server();
        }
        int ret = 0;

        /* LAB 5 TODO BEGIN */

        /* LAB 5 TODO END */

        return ret;
}

void chcore_fsm_test()
{
        if (!fsm_ipc_struct) {
                connect_fsm_server();
        }
        char wbuf[257];
        char rbuf[257];
        memset(rbuf, 0, sizeof(rbuf));
        memset(wbuf, 'x', sizeof(wbuf));
        wbuf[256] = '\0';
        fsm_creat_file("/fakefs/fsmtest.txt");
        fsm_write_file("/fakefs/fsmtest.txt", wbuf, sizeof(wbuf));
        fsm_read_file("/fakefs/fsmtest.txt", rbuf, sizeof(rbuf));
        int res = memcmp(wbuf, rbuf, strlen(wbuf));
        if(res == 0) {
                printf("chcore fsm bypass test pass\n");
        }

}
