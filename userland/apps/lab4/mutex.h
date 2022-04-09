#pragma once
#include <chcore/internal/raw_syscall.h>

struct lock {
        int lock_sem;
};

void lock_init(struct lock *lock);
void lock(struct lock *lock);
void unlock(struct lock *lock);
