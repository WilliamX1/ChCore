#pragma once

struct spinlock {
        volatile int val;
};

void spinlock_init(struct spinlock *lock);
void spinlock_lock(struct spinlock *lock);
void spinlock_unlock(struct spinlock *lock);