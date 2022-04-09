#define PRIO 255
#include <chcore/thread.h>
#include <chcore/internal/raw_syscall.h>
#include <stdio.h>

void delay(int time)
{
        for (int i = 0; i < time; i++)
                for (int j = 0; j < 100000; j++)
                        asm volatile("nop");
}

void *sem_routine0(void *arg)
{
        int sem = (int)(unsigned long)arg;
        __chcore_sys_wait_sem(sem, 1);
        printf("Hello thread 1! wait sem %d return\n", sem);
        printf("Thread 1 return\n");
        return 0;
}

void *sem_routine1(void *arg)
{
        int sem = (int)(unsigned long)arg;
        printf("Hello thread 2! Before delay!\n");
        delay(1000);
        printf("Hello thread 2! Before signal!\n");
        __chcore_sys_signal_sem(sem);
        printf("Thread 2 return\n");
        return 0;
}

int main(int argc, char *argv[], char *envp[])
{
        int sem = __chcore_sys_create_sem();
        chcore_thread_create(sem_routine0, sem, 128, TYPE_USER);
        chcore_thread_create(sem_routine1, sem, 0, TYPE_USER);
        return 0;
}
