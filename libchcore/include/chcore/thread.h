#pragma once

#include <chcore/types.h>

#define MAX_PRIO 255

#ifdef __cplusplus
extern "C" {
#endif

struct thread_args {
        u64 cap_group_cap;
        u64 stack;
        u64 pc;
        u64 arg;
        u32 prio;
        /* 0: TYPE_USER; 1: TYPE_SHADOW */
        u32 type;
};

enum thread_type {
        TYPE_USER = 0,
        TYPE_SHADOW = 1, /* SHADOW thread is used to achieve migrate IPC */
};

int chcore_thread_create(void *(*func)(void *), u64 arg, u32 prio, u32 type);

#ifdef __cplusplus
}
#endif
