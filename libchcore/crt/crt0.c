#include <chcore/types.h>
#include <chcore/internal/raw_syscall.h>
#include <chcore/internal/crt0_arch.h>
#include <chcore/assert.h>

int main();

void _start_c(long *p)
{
        int argc = (int)p[0];
        char **argv = (void *)(p + 1);
        main(argc, argv);
        __chcore_sys_thread_exit(); /* Should recycle process in real system */
        chcore_bug("Should not reach here!\n");
}
