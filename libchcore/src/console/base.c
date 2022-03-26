#include <chcore/console.h>
#include <chcore/assert.h>
#include <chcore/internal/raw_syscall.h>

void chcore_console_putc(int ch)
{
        chcore_assert(ch >= 0 && ch < (char)-1);
        __chcore_sys_putc((char)ch);
}

int chcore_console_getc(void)
{
        int ch = -1;
        while (!(ch >= 0 && ch < (char)-1))
                ch = __chcore_sys_getc();
        return ch;
}

void chcore_console_puts(const char *str)
{
        while (*str) {
                chcore_console_putc(*str++);
        }
}
