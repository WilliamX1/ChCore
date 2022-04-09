#include <stdio.h>

int main(int argc, char *argv[], char *envp[])
{
        char ch = 0;
        ch = cgetc();
        printf("YOUR: [%c]\n", ch);
        return 0;
}