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

#pragma once

#include <arch/sched/arch_sched.h>
#include <arch/sync.h>
#include <common/types.h>
#include <common/list.h>
#include <common/kprint.h>
#include <machine.h>

struct thread;

/* Timer ticks in system */
#if LOG_LEVEL == DEBUG
/* BUDGET represents the number of TICKs */
#define DEFAULT_BUDGET 10
#define TICK_MS        3000
#else
/* BUDGET represents the number of TICKs */
#define DEFAULT_BUDGET 10
#define TICK_MS        10
#endif

#define MAX_PRIO 255
#define MIN_PRIO 0
#define PRIO_NUM (MAX_PRIO + 1)

#define NO_AFF -1

/* Data structures */

#define STATE_STR_LEN 20
enum thread_state {
        TS_INIT = 0,
        TS_READY,
        TS_INTER, /* Intermediate stat used by sched (only for debug) */
        TS_RUNNING,
        TS_EXIT, /* Only for debug use */
        TS_WAITING, /* Waiting IPC or etc */
};

enum thread_exit_state {
        TE_RUNNING = 0,
        TE_EXITING,
        TE_EXITED,
};

#define TYPE_STR_LEN 20
enum thread_type {
        /*
         * Kernel-level threads won't swap gs/fs
         */
        TYPE_IDLE = 0, /* IDLE thread dose not have stack, pause cpu */
        TYPE_KERNEL = 1, /* KERNEL thread has stack */

        /*
         * User-level threads
         * Should be larger than TYPE_KERNEL!
         */
        TYPE_USER = 2,
        TYPE_SHADOW = 3, /* SHADOW thread is used to achieve migrate IPC */
        /* Use as the IPC register callback threads (for recycling) */
        TYPE_TESTS = 4 /* TESTS thread is used by kernel tests */
};

typedef struct sched_cont {
        u32 budget;
        char pad[pad_to_cache_line(sizeof(u32))];
} sched_cont_t;

/* Must be 8-byte aligned */
struct thread_ctx {
        /* Executing Context */
        arch_exec_cont_t ec;
        /* Scheduling Context */
        sched_cont_t *sc;
        /* Thread Type */
        u32 type;
        /* Thread state (can not be modified by other cores) */
        u32 state;
        /* Priority */
        u32 prio;
        /* SMP Affinity */
        s32 affinity;
        /* Current Assigned CPU */
        u32 cpuid;
        /* Thread exit state */
        volatile u32 thread_exit_state;
} __attribute__((aligned(CACHELINE_SZ)));

/* Debug functions */
void print_thread(struct thread *thread);

extern char thread_type[][TYPE_STR_LEN];
extern char thread_state[][STATE_STR_LEN];

void arch_idle_ctx_init(struct thread_ctx *idle_ctx, void (*func)(void));
void arch_switch_context(struct thread *target);
u64 switch_context(void);
int sched_is_running(struct thread *target);
void sched_handle_timer_irq(void);

/* This interface is local to scheduler. */
int switch_to_thread(struct thread *target);

/* This interface can be used in other places in the kernel. */
void sched_to_thread(struct thread *target);

/* Global-shared kernel data */
extern struct thread *current_threads[PLAT_CPU_NUM];

/* Indirect function call may downgrade performance */
struct sched_ops {
        int (*sched_init)(void);
        int (*sched)(void);
        int (*sched_enqueue)(struct thread *thread);
        int (*sched_dequeue)(struct thread *thread);
        /* Debug tools */
        void (*sched_top)(void);
};

/* Provided Scheduling Policies */
extern struct sched_ops rr; /* Simple Round Robin */

/* Chosen Scheduling Policies */
extern struct sched_ops *cur_sched_ops;

int sched_init(struct sched_ops *sched_ops);

static inline int sched(void)
{
        return cur_sched_ops->sched();
}

static inline int sched_enqueue(struct thread *thread)
{
        return cur_sched_ops->sched_enqueue(thread);
}

static inline int sched_dequeue(struct thread *thread)
{
        return cur_sched_ops->sched_dequeue(thread);
}

/* Syscalls */
void sys_yield(void);
void sys_top(void);
