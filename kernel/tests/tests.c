#include <common/lock.h>
#include <common/kprint.h>
#include <arch/machine/smp.h>
#include <common/macro.h>
#include <mm/kmalloc.h>
#include "tests.h"
#include "barrier.h"

struct lock test_lock;

void init_test(void)
{
        u32 ret = 0;

        global_barrier_init();
        ret = lock_init(&test_lock);
        BUG_ON(ret != 0);
}

void run_test(void)
{
        init_test();
        tst_mutex();
        tst_sched_cooperative();
        tst_sched_preemptive();
        tst_sched_affinity();
        tst_sched();
}
