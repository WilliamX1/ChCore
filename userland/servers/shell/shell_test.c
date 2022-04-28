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

#include "shell.h"

#include <stdio.h>
#include <string.h>
#include <chcore/internal/raw_syscall.h>
#include <chcore/ipc.h>
#include <chcore/assert.h>
#include <chcore/procm.h>
#include <chcore/fsm.h>
#include <chcore/thread.h>
#include <chcore/internal/server_caps.h>


#define TEST_FUNC(name) \
  do { \
    if (name() == 0) { \
      printf(#name" pass!\n"); \
    } else { \
      printf(#name" fail!\n"); \
    } \
  } while (0)

extern struct ipc_struct *fs_ipc_struct_for_shell;


static void test_readline()
{
	char* buf = readline("1>");
	printf("%s\n", buf);
	readline("2>");
}

static void test_echo()
{
	builtin_cmd("echo abc123XYZ");
}

static void test_ls() {
	builtin_cmd("ls /");
}

static void test_cat() {
	builtin_cmd("cat /test.txt");
}

static void test_top() {
	do_top();
}

static void test_run() {
	run_cmd("/helloworld.bin");
}

void shell_test() {
	test_echo();
	printf("\nSHELL ");
	test_ls();
	printf("\nSHELL ");
	test_cat();
	printf("\n");
	test_readline();
	test_top();
	test_run();
	for (int i = 0; i < 10000; i++) {
        __chcore_sys_yield(); 
	}	
	run_cmd("/lab5.bin");
	for (int i = 0; i < 10000 * 5; i++) {
    	__chcore_sys_yield(); 
	}

}

void fsm_test() {

	int fsm_cap = __chcore_get_fsm_cap();
	chcore_assert(fsm_cap >= 0);
	fs_ipc_struct_for_shell = ipc_register_client(fsm_cap);
	chcore_assert(fs_ipc_struct_for_shell);
	mount_fs("/fakefs.srv", "/fakefs");	
	printf("\nFSM ");
	builtin_cmd("ls /");
	printf("\nFSM ");
	builtin_cmd("ls /fakefs");
	printf("\nFSM ");
	builtin_cmd("cat /fakefs/test.txt");
	printf("\nFSM ");
	builtin_cmd("cat /test.txt");
	printf("\n");

	chcore_fsm_test();

}