#pragma once

#include <common/types.h>

struct lock {
        volatile u64 slock;
};

int lock_init(struct lock *lock);
void lock(struct lock *lock);
/* returns 0 on success, -1 otherwise */
int try_lock(struct lock *lock);
void unlock(struct lock *lock);
int is_locked(struct lock *lock);

/* Global locks */
extern struct lock big_kernel_lock;
void kernel_lock_init(void);
void lock_kernel(void);
void unlock_kernel(void);
