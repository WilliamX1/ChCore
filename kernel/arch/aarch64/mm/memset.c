#include <common/types.h>

void memset(void *dst, char ch, size_t size)
{
        char *buf;
        size_t i;

        buf = (char *)dst;
        for (i = 0; i < size; ++i)
                buf[i] = ch;
}
