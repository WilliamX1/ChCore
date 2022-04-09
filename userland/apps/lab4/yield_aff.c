#include <chcore/internal/raw_syscall.h>
#include <chcore/thread.h>
#include <chcore/assert.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
        int ret = 0;
        int aff = 3;

        printf("Main thread on cpu %u\n", __chcore_sys_get_cpu_id());
        ret = __chcore_sys_set_affinity(-1, aff);
        chcore_assert(ret == 0);
        printf("Main thread set affinity %d\n", aff);
        __chcore_sys_yield();
        ret = __chcore_sys_get_affinity(-1);
        printf("Main thread affinity %d\n", ret);
        printf("Main thread exits on cpu_id: %u\n", __chcore_sys_get_cpu_id());
        return 0;
}
