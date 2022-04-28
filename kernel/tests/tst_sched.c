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

#include <common/lock.h>
#include <common/kprint.h>
#include <arch/machine/smp.h>
#include <common/macro.h>
#include <mm/kmalloc.h>
#include <object/thread.h>
#include <sched/context.h>
#include <sched/sched.h>
#include <machine.h>
#include "barrier.h"

/* Only for test use */
/* Metadata for ready queue */
struct queue_meta {
        struct list_head queue_head;
        u32 queue_len;
        char pad[pad_to_cache_line(sizeof(u32) + sizeof(struct list_head))];
};

extern struct thread *rr_sched_choose_thread(void);
extern struct queue_meta rr_ready_queue_meta[PLAT_CPU_NUM];

#define TEST_NUM   1
#define THREAD_NUM 8

struct lock test_lock;

volatile int sched_start_flag = 0;
volatile int sched_finish_flag = 0;

static void atomic_sched(void)
{
        lock(&test_lock);
        sched();
        unlock(&test_lock);

        BUG_ON(!current_thread);
        BUG_ON(!current_thread->thread_ctx);
}

static int atomic_sched_enqueue(struct thread *thread)
{
        int ret;

        lock(&test_lock);
        ret = sched_enqueue(thread);
        unlock(&test_lock);

        return ret;
}

static struct thread *atomic_sched_choose_thread(void)
{
        struct thread *ret = NULL;

        lock(&test_lock);
        ret = rr_sched_choose_thread();
        unlock(&test_lock);

        return ret;
}

static struct thread *create_test_thread(int prio, int aff)
{
        int i = 0;
        struct thread *thread = NULL;

        lock(&test_lock);
        thread = kmalloc(sizeof(struct thread));
        BUG_ON(!(thread->thread_ctx = create_thread_ctx(TYPE_TESTS)));
        init_thread_ctx(thread, 0, 0, prio, TYPE_TESTS, aff);
        for (i = 0; i < REG_NUM; i++)
                thread->thread_ctx->ec.reg[i] = prio;
        unlock(&test_lock);

        return thread;
}

static void check_thread_ctx(void)
{
        int i = 0;
        struct thread_ctx *thread_ctx = NULL;

        lock(&test_lock);
        thread_ctx = (struct thread_ctx *)switch_context();
        unlock(&test_lock);
        for (i = 0; i < REG_NUM; i++)
                BUG_ON(thread_ctx->ec.reg[i] != thread_ctx->prio);
}

static void free_test_thread(struct thread *thread)
{
        BUG_ON(thread->thread_ctx->type == TYPE_IDLE);
        lock(&test_lock);
        destroy_thread_ctx(thread);
        kfree(thread);
        unlock(&test_lock);
}

void tst_sched_param()
{
        int i = 0;
        u32 local_thread_num = 4;
        u32 cpuid = smp_get_cpu_id();
        struct thread *threads[4];
        struct thread *idle_thread = NULL;
        struct thread *thread = NULL;
        struct thread_ctx *thread_ctx = NULL;

        global_barrier();
        /* Init threads */
        for (i = 0; i < local_thread_num; i++) {
                threads[i] = create_test_thread(i, cpuid);
        }

        { // test enqueue
                BUG_ON(!sched_enqueue(NULL));
                {
                        thread_ctx = threads[0]->thread_ctx;
                        threads[0]->thread_ctx = NULL;
                        BUG_ON(!sched_enqueue(threads[0]));
                        threads[0]->thread_ctx = thread_ctx;

                        BUG_ON(!list_empty(
                                &(rr_ready_queue_meta[cpuid].queue_head)));
                }
                {
                        threads[0]->thread_ctx->state = TS_READY;
                        BUG_ON(!sched_enqueue(threads[0]));
                        threads[0]->thread_ctx->state = TS_INIT;
                }
                {
                        for (i = 0; i < local_thread_num; i++) {
                                BUG_ON(sched_enqueue(threads[i]));
                                BUG_ON(threads[i]->thread_ctx->cpuid != cpuid);
                                BUG_ON(threads[i]->thread_ctx->state
                                       != TS_READY);
                        }

                        BUG_ON(!sched_enqueue(threads[0]));
                }
        }

        { // test dequeue
                BUG_ON(!sched_dequeue(NULL));
                {
                        thread_ctx = threads[0]->thread_ctx;
                        threads[0]->thread_ctx = NULL;
                        BUG_ON(!sched_dequeue(threads[0]));
                        threads[0]->thread_ctx = thread_ctx;
                }

                {
                        BUG_ON(sched_dequeue(threads[2]));
                        BUG_ON(sched_dequeue(threads[1]));
                        BUG_ON(sched_dequeue(threads[3]));
                        BUG_ON(sched_dequeue(threads[0]));

                        for (i = 0; i < local_thread_num; i++) {
                                BUG_ON(threads[i]->thread_ctx->state
                                       != TS_INTER);
                        }

                        BUG_ON(!list_empty(
                                &(rr_ready_queue_meta[cpuid].queue_head)));
                }
        }

        { // test choose_thread & idle thread
                idle_thread = rr_sched_choose_thread();
                BUG_ON(idle_thread->thread_ctx->type != TYPE_IDLE);

                {
                        BUG_ON(!list_empty(
                                &(rr_ready_queue_meta[cpuid].queue_head)));
                        BUG_ON(sched_enqueue(idle_thread));
                        BUG_ON(!list_empty(
                                &(rr_ready_queue_meta[cpuid].queue_head)));
                }

                {
                        BUG_ON(sched_enqueue(threads[0]));
                        BUG_ON(!sched_dequeue(idle_thread));

                        thread = rr_sched_choose_thread();
                        BUG_ON(thread != threads[0]);

                        BUG_ON(!list_empty(
                                &(rr_ready_queue_meta[cpuid].queue_head)));
                }

                {
                        BUG_ON(sched_enqueue(threads[3]));
                        BUG_ON(sched_enqueue(threads[2]));
                        BUG_ON(sched_enqueue(threads[0]));
                        BUG_ON(sched_enqueue(threads[1]));

                        thread = rr_sched_choose_thread();
                        BUG_ON(thread != threads[3]);

                        BUG_ON(sched_dequeue(threads[2]));
                        BUG_ON(sched_dequeue(threads[1]));

                        thread = rr_sched_choose_thread();
                        BUG_ON(thread != threads[0]);

                        thread = rr_sched_choose_thread();
                        BUG_ON(thread != idle_thread);
                }
        }

        { // test sched
                BUG_ON(sched_enqueue(threads[3]));
                BUG_ON(sched_enqueue(threads[2]));
                BUG_ON(sched_enqueue(threads[0]));
                BUG_ON(sched_enqueue(threads[1]));

                sched();
                current_thread->thread_ctx->sc->budget = 0;
                BUG_ON(threads[3]->thread_ctx->state != TS_RUNNING);
                BUG_ON(!sched_dequeue(threads[3]));

                thread = rr_sched_choose_thread();
                BUG_ON(thread != threads[2]);

                sched();
                current_thread->thread_ctx->sc->budget = 0;
                BUG_ON(threads[0]->thread_ctx->state != TS_RUNNING);
                BUG_ON(!sched_dequeue(threads[0]));

                thread = rr_sched_choose_thread();
                BUG_ON(thread != threads[1]);

                sched();
                current_thread->thread_ctx->sc->budget = 0;
                BUG_ON(threads[3]->thread_ctx->state != TS_RUNNING);
                BUG_ON(!sched_dequeue(threads[3]));

                thread = rr_sched_choose_thread();
                BUG_ON(thread != threads[0]);

                BUG_ON(current_thread != threads[3]);
                current_thread = NULL;

                BUG_ON(!list_empty(&(rr_ready_queue_meta[cpuid].queue_head)));
        }

        for (i = 0; i < local_thread_num; i++) {
                free_test_thread(threads[i]);
        }

        global_barrier();
}

void tst_sched_queue()
{
        int i = 0;
        struct thread *threads[THREAD_NUM];

        global_barrier();
        /* Init threads */
        for (i = 0; i < THREAD_NUM; i++) {
                threads[i] = create_test_thread(i % MAX_PRIO, smp_get_cpu_id());
                BUG_ON(sched_enqueue(threads[i]));
        }

        for (i = 0; i < THREAD_NUM; i++) {
                sched();
                current_thread->thread_ctx->sc->budget = 0;
                BUG_ON(current_thread != threads[i]);
                check_thread_ctx();
        }

        for (i = 0; i < THREAD_NUM; i++) {
                sched();
                current_thread->thread_ctx->sc->budget = 0;
                BUG_ON(current_thread != threads[i]);
                free_test_thread(current_thread);
                current_thread = NULL;
        }
        global_barrier();
}

void tst_sched_cooperative(void)
{
        tst_sched_param();
        tst_sched_queue();
        if (smp_get_cpu_id() == 0)
                kinfo("Pass tst_sched_cooperative!\n");
}

void tst_sched_budget(void)
{
        int i = 0;
        u32 local_thread_num = 4;
        struct thread *threads[4];

        /* Init threads */
        for (i = 0; i < local_thread_num; i++) {
                threads[i] = create_test_thread(i, smp_get_cpu_id());
                BUG_ON(sched_enqueue(threads[i]));
        }
        BUG_ON(current_thread);

        global_barrier();

        sched();
        BUG_ON(!current_thread->thread_ctx->sc);
        for (i = 0; i < local_thread_num; i++) {
                sched();
                BUG_ON(current_thread != threads[0]);
        }

        current_thread->thread_ctx->sc->budget = 0;
        sched();
        for (i = 0; i < local_thread_num; i++) {
                sched();
                BUG_ON(current_thread != threads[1]);
        }

        current_thread->thread_ctx->sc->budget = 0;
        for (i = 0; i < local_thread_num; i++) {
                sched();
                BUG_ON(!current_thread->thread_ctx->sc);
                BUG_ON(current_thread != threads[(i + 2) % PLAT_CPU_NUM]);
                free_test_thread(threads[(i + 2) % PLAT_CPU_NUM]);
                current_thread = NULL;
        }

        global_barrier();
}

void tst_sched_timer(void)
{
        int i = 0, j = 0;
        u32 local_thread_num = 4;
        struct thread *threads[4];

        /* Init threads */
        for (i = 0; i < local_thread_num; i++) {
                threads[i] = create_test_thread(i, smp_get_cpu_id());
                BUG_ON(sched_enqueue(threads[i]));
        }
        BUG_ON(current_thread);

        global_barrier();

        for (j = 0; j < DEFAULT_BUDGET; j++) {
                sched_handle_timer_irq();
        }
        sched();
        BUG_ON(!current_thread->thread_ctx->sc);
        for (i = 0; i < local_thread_num; i++) {
                sched();
                BUG_ON(current_thread != threads[0]);
        }

        threads[0]->thread_ctx->sc->budget = 0;
        for (j = 0; j < DEFAULT_BUDGET; j++) {
                sched_handle_timer_irq();
        }
        sched();
        for (i = 0; i < local_thread_num; i++) {
                sched();
                BUG_ON(current_thread != threads[1]);
        }

        BUG_ON(DEFAULT_BUDGET <= 1);
        sched_handle_timer_irq();
        sched();
        BUG_ON(current_thread != threads[1]);

        for (j = 0; j < DEFAULT_BUDGET; j++) {
                sched_handle_timer_irq();
        }
        for (i = 0; i < local_thread_num; i++) {
                sched();
                BUG_ON(!current_thread->thread_ctx->sc);
                BUG_ON(current_thread != threads[(i + 2) % PLAT_CPU_NUM]);
                free_test_thread(threads[(i + 2) % PLAT_CPU_NUM]);
                current_thread = NULL;
        }
        global_barrier();
}

void tst_sched_preemptive(void)
{
        tst_sched_budget();
        tst_sched_timer();
        if (smp_get_cpu_id() == 0) {
                kinfo("Pass tst_sched_preemptive!\n");
        }
}

void tst_sched_aff_param()
{
        u32 cpuid = smp_get_cpu_id();
        struct thread *thread = NULL, *idle_thread = NULL;

        global_barrier();

        /* should return idle thread */
        thread = atomic_sched_choose_thread();
        BUG_ON(thread->thread_ctx->type != TYPE_IDLE);

        thread = create_test_thread(0, 6);
        BUG_ON(!sched_enqueue(thread));
        BUG_ON(!sched_dequeue(thread));
        idle_thread = rr_sched_choose_thread();
        BUG_ON(idle_thread->thread_ctx->type != TYPE_IDLE);

        free_test_thread(thread);
        BUG_ON(!list_empty(&(rr_ready_queue_meta[cpuid].queue_head)));
        global_barrier();
}

void tst_sched_aff()
{
        int i = 0;
        u32 cpuid = smp_get_cpu_id();
        struct thread *thread = NULL;
        struct thread *threads[THREAD_NUM + 1];

        global_barrier();
        /* should return idle thread */
        thread = atomic_sched_choose_thread();
        BUG_ON(thread->thread_ctx->type != TYPE_IDLE);
        global_barrier();
        if (smp_get_cpu_id() == 0) {
                for (i = 0; i < THREAD_NUM + 1; i++) {
                        threads[i] = create_test_thread(i, cpuid);
                        BUG_ON(sched_enqueue(threads[i]));
                }
                for (i = 0; i < PLAT_CPU_NUM; i++) {
                        if (i != cpuid) {
                                BUG_ON(!list_empty(
                                        &(rr_ready_queue_meta[i].queue_head)));
                        }
                }

                threads[0]->thread_ctx->sc->budget = 0;
                atomic_sched();
                for (i = 0; i < THREAD_NUM + 1; i++) {
                        threads[i]->thread_ctx->affinity = i % PLAT_CPU_NUM;
                        threads[i]->thread_ctx->sc->budget = 0;
                        atomic_sched();
                }
                for (i = 0; i < THREAD_NUM / PLAT_CPU_NUM; i++) {
                        threads[i * PLAT_CPU_NUM]->thread_ctx->sc->budget = 0;
                        atomic_sched();
                }
        }

        global_barrier();
        for (i = 0; i < THREAD_NUM / PLAT_CPU_NUM; i++) {
                thread = atomic_sched_choose_thread();
                BUG_ON(thread->thread_ctx->cpuid != cpuid);
                BUG_ON(thread->thread_ctx->affinity % PLAT_CPU_NUM != cpuid);
                BUG_ON(thread->thread_ctx->prio != i * PLAT_CPU_NUM + cpuid);
                current_thread = NULL;
        }
        global_barrier();
        if (smp_get_cpu_id() == 0) {
                for (i = 0; i < THREAD_NUM + 1; i++) {
                        free_test_thread(threads[i]);
                }
        }
        global_barrier();
}

void tst_sched_affinity()
{
        tst_sched_aff_param();
        tst_sched_aff();
        if (smp_get_cpu_id() == 0) {
                kinfo("Pass tst_sched_affinity!\n");
        }
}

void tst_sched()
{
        int i = 0, j = 0, k = 0;
        u32 cpuid = smp_get_cpu_id();
        struct thread *thread = NULL;

        /* should return idle thread */
        thread = atomic_sched_choose_thread();
        BUG_ON(thread->thread_ctx->type != TYPE_IDLE);

        global_barrier();

        /* Init threads */
        for (i = 0; i < PLAT_CPU_NUM; i++) {
                for (j = 0; j < THREAD_NUM; j++) {
                        thread = create_test_thread(k, i);
                        BUG_ON(atomic_sched_enqueue(thread));
                        k++;
                }
        }

        for (j = 0; j < TEST_NUM; j++) {
                /* Each core try to get those threads */
                for (i = 0; i < THREAD_NUM * PLAT_CPU_NUM; i++) {
                        /* get thread and dequeue from ready queue */
                        do {
                                /* do it again if choose idle thread */
                                atomic_sched();
                                current_thread->thread_ctx->sc->budget = 0;
                        } while (current_thread->thread_ctx->type == TYPE_IDLE);
                        BUG_ON(!current_thread->thread_ctx->sc);
                        /* Current thread set affinitiy */
                        current_thread->thread_ctx->affinity =
                                (i + cpuid) % PLAT_CPU_NUM;
                        check_thread_ctx();
                }
        }

        for (i = 0; i < THREAD_NUM * PLAT_CPU_NUM; i++) {
                /* get thread and dequeue from ready queue */
                do {
                        /* do it again if choose idle thread */
                        atomic_sched();
                        current_thread->thread_ctx->sc->budget = 0;
                } while (current_thread->thread_ctx->type == TYPE_IDLE);
                BUG_ON(!current_thread->thread_ctx->sc);
                free_test_thread(current_thread);
                current_thread = NULL;
        }

        global_barrier();

        /* should return idle thread */
        thread = atomic_sched_choose_thread();
        BUG_ON(thread->thread_ctx->type != TYPE_IDLE);

        global_barrier();

        if (smp_get_cpu_id() == 0) {
                kinfo("Pass tst_sched!\n");
        }
}
