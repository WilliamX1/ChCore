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

void connect_fs(void);

// Mount a file system in `mount_point`
int mount_fs(const char *fspath, const char *mount_point);

// read a command from stdin leading by `prompt`
// put the commond in `buf` and return `buf`
char *readline(const char *prompt);

// run `ls`, `echo`, `cat`, `cd`, `top`
// return true if `cmdline` is a builtin command
int builtin_cmd(char *cmdline);

// run other command, such as execute an executable file
// return true if run sccessfully
int run_cmd(char *cmdline);

int do_complement(char *buf, char *complement, int complement_time);

void do_top();

int getdents(int fd, char *buf, int count);