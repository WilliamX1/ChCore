/* Scheduler related functions are implemented here */
#include <sched/sched.h>
#include <arch/machine/smp.h>
#include <common/kprint.h>
#include <machine.h>
#include <mm/kmalloc.h>
#include <common/list.h>
#include <common/util.h>
#include <object/thread.h>
#include <common/macro.h>
#include <common/errno.h>
#include <common/types.h>
#include <object/thread.h>
#include <irq/irq.h>
#include <sched/context.h>

/* in arch/sched/idle.S */
void idle_thread_routine(void);

/* Metadata for ready queue */
struct queue_meta {
        struct list_head queue_head;
        u32 queue_len;
        char pad[pad_to_cache_line(sizeof(u32) + sizeof(struct list_head))];
};

/*
 * rr_ready_queue
 * Per-CPU ready queue for ready tasks.
 */
struct queue_meta rr_ready_queue_meta[PLAT_CPU_NUM];

/*
 * RR policy also has idle threads.
 * When no active user threads in ready queue,
 * we will choose the idle thread to execute.
 * Idle thread will **NOT** be in the RQ.
 */
struct thread idle_threads[PLAT_CPU_NUM];

int rr_sched_enqueue(struct thread *thread)
{
        return 0;
}

int rr_sched_dequeue(struct thread *thread)
{
        return 0;
}

struct thread *rr_sched_choose_thread(void)
{
        struct thread *thread = NULL;
        return thread;
}

static inline void rr_sched_refill_budget(struct thread *target, u32 budget)
{
}

int rr_sched(void)
{
        return 0;
}

int rr_sched_init(void)
{
        return 0;
}

#define MAX_CAP_GROUP_BUF 256

void rr_top(void)
{
}

struct sched_ops rr = {.sched_init = rr_sched_init,
                       .sched = rr_sched,
                       .sched_enqueue = rr_sched_enqueue,
                       .sched_dequeue = rr_sched_dequeue,
                       .sched_top = rr_top};
