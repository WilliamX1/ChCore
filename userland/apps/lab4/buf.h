#pragma once
#include <sync/spin.h>

#define BUF_SIZE 5

void buffer_init(void);
void buffer_add_safe(int msg);
int buffer_remove_safe(void);
