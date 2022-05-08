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
#include <chcore/types.h>
#include <chcore/fsm.h>
#include <chcore/tmpfs.h>
#include <chcore/ipc.h>
#include <chcore/internal/raw_syscall.h>
#include <chcore/internal/server_caps.h>
#include <chcore/procm.h>
#include <chcore/fs/defs.h>
#include <string.h>

#include "lab5_stdio.h"

#define TEST_FUNC(name) \
    do { \
        if (name() == 0) { \
            printf(#name" pass!\n"); \
        } else { \
            printf(#name" fail!\n"); \
        } \
    } while (0)


struct ipc_struct *tmpfs_ipc_struct = NULL;

void connect_tmpfs_server(void)
{
        int tmpfs_cap = __chcore_get_tmpfs_cap();
        chcore_assert(tmpfs_cap >= 0);
        tmpfs_ipc_struct = ipc_register_client(tmpfs_cap);
        chcore_assert(tmpfs_ipc_struct);
}

char wbuf[256];
char rbuf[32];
char rbuf2[32];

int lab5_stdio_file_read_write () {
    memset(wbuf, 0x0, sizeof(wbuf));
    memset(rbuf, 0x0, sizeof(rbuf));
    for(int i = 0; i < sizeof(wbuf); ++i) {
        wbuf[i] = (char) i;
    }
    FILE * pFile;
    pFile = fopen("/myfile.txt", "w");
    fwrite(wbuf, sizeof(char) , sizeof(wbuf), pFile);
    fclose(pFile);

    pFile = fopen("/myfile.txt", "r");
    int cnt;
    do {
        cnt = fread(rbuf, sizeof(char), sizeof(rbuf), pFile);
    } while(cnt > 0);
    fclose(pFile);
    return memcmp(rbuf, (char*)wbuf + sizeof(wbuf) - sizeof(rbuf), sizeof(rbuf));
}

int lab5_stdio_file_printf_scanf () {
    memset(rbuf, 0x0, sizeof(rbuf));
    memset(rbuf2, 0x0, sizeof(rbuf2));
    void* ptr = malloc(sizeof(char) * 2);
    int data = *(int*)&ptr;

    FILE * pFile;
    pFile = fopen("/myfile2.txt", "w");
    fprintf(pFile, "fprintf %s %d\n", __func__, data);
    fclose(pFile);

    int outdata;
    pFile = fopen("/myfile2.txt", "r");
    fscanf(pFile, "%s %s %d", rbuf, rbuf2, &outdata);
    free(ptr);
    return strcmp(rbuf, "fprintf") != 0 || strcmp(rbuf2, __func__) != 0 || outdata != data;
}

int main() {
    connect_tmpfs_server();
    TEST_FUNC(lab5_stdio_file_read_write);
    TEST_FUNC(lab5_stdio_file_printf_scanf);
}
