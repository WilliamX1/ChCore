#include <sync/spin.h>
#include "buf.h"

volatile int buffer_write_cnt = 0;
volatile int buffer_read_cnt = 0;
struct spinlock buffer_lock;
int buffer[BUF_SIZE];

void buffer_init(void)
{
        spinlock_init(&buffer_lock);
}

void buffer_add_safe(int msg)
{
        spinlock_lock(&buffer_lock);
        buffer[buffer_write_cnt] = msg;
        buffer_write_cnt = (buffer_write_cnt + 1) % BUF_SIZE;
        spinlock_unlock(&buffer_lock);
}

int buffer_remove_safe(void)
{
        spinlock_lock(&buffer_lock);
        int ret = buffer[buffer_read_cnt];
        buffer_read_cnt = (buffer_read_cnt + 1) % BUF_SIZE;
        spinlock_unlock(&buffer_lock);
        return ret;
}
