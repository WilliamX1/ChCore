#include <stdio.h>

int main(int argc, char *argv[], char *envp[])
{
        asm volatile(".byte 0x40, 0x00, 0x00, 0x00");
        printf("FATAL-ERRS: Wrong Answer!\n");
        return 0;
}
