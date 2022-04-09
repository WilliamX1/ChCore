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