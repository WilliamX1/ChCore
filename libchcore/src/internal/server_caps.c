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

#include <chcore/internal/server_caps.h>

/* Defined in __libchcore_init.c */
extern int __chcore_procm_cap;
extern int __chcore_fsm_cap;
extern int __chcore_tmpfs_cap;

int __chcore_get_procm_cap(void)
{
        return __chcore_procm_cap;
}

int __chcore_get_fsm_cap(void)
{
        return __chcore_fsm_cap;
}

int __chcore_get_tmpfs_cap(void)
{
        return __chcore_tmpfs_cap;
}

void __chcore_set_procm_cap(int cap)
{
        __chcore_procm_cap = cap;
}

void __chcore_set_fsm_cap(int cap)
{
        __chcore_fsm_cap = cap;
}

void __chcore_set_tmpfs_cap(int cap)
{
        __chcore_tmpfs_cap = cap;
}
