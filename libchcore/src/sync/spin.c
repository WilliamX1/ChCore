#include <sync/spin.h>
#include <chcore/internal/raw_syscall.h>

void spinlock_init(struct spinlock *lock)
{
        lock->val = 0;
}

void spinlock_lock(struct spinlock *lock)
{
        while (__sync_lock_test_and_set(&lock->val, 1)) {
                while (lock->val != 0)
                        __chcore_sys_yield();
        }
}

void spinlock_unlock(struct spinlock *lock)
{
        __sync_lock_release(&lock->val);
}