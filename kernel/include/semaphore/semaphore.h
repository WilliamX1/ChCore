#pragma once

#include <common/types.h>
#include <common/list.h>

struct semaphore {
        u32 sem_count;
        u32 waiting_threads_count;
        struct list_head waiting_threads;
};

void init_sem(struct semaphore *sem);
s32 wait_sem(struct semaphore *sem, bool is_block);
s32 signal_sem(struct semaphore *sem);

/* Syscalls */
s32 sys_create_sem(void);
s32 sys_wait_sem(u32 sem_cap, bool is_block);
s32 sys_signal_sem(u32 sem_cap);
