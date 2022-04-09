#include <chcore/internal/raw_syscall.h>
#include <chcore/thread.h>
#include <chcore/assert.h>
#include <stdio.h>

#define PRIO 255
#define INFO 233

bool running = true;

void *thread_routine(void *arg)
{
        u64 thread_id = (u64)arg;

        printf("Hello, I am thread %u\n", thread_id);

        while (running) {
        }
        return 0;
}

int main(int argc, char *argv[])
{
        int child_thread_cap;

        child_thread_cap =
                chcore_thread_create(thread_routine, 1, PRIO, TYPE_USER);
        if (child_thread_cap < 0)
                printf("Create thread failed, return %d\n", child_thread_cap);

        __chcore_sys_yield();

        printf("Successfully regain the control!\n");
        return 0;
}
