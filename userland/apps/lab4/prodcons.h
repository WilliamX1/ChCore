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

/* Test setting */
#define PROD_THD_CNT  10
#define PROD_ITEM_CNT 30
#define CONS_THD_CNT  10
#define COSM_ITEM_CNT (PROD_THD_CNT * PROD_ITEM_CNT) / CONS_THD_CNT
#define PLAT_CPU_NUM  4

#define PRIO 255

int sleep(int time);
int produce_new();
int consume_msg(int msg);