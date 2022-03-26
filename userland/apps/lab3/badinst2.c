#include <stdio.h>

int main(int argc, char *argv[], char *envp[])
{
        asm volatile("mrs x0, elr_el1");
        printf("FATAL-ERRS: Wrong Answer!\n");
        return 0;
}
