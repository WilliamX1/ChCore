#include <common/types.h>
#include <common/errno.h>
#include <common/macro.h>
#include <common/lock.h>
#include <common/kprint.h>
#include <arch/sync.h>

#include "ticket.h"

int lock_init(struct lock *l)
{
        return 0;
}

void lock(struct lock *l)
{
        return;
}

int try_lock(struct lock *l)
{
        return 0;
}

void unlock(struct lock *l)
{
}

int is_locked(struct lock *l)
{
        return 0;
}
