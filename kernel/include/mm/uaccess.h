#pragma once

#include <common/types.h>

int copy_from_user(char *kbuf, char *ubuf, size_t size);
int copy_to_user(char *ubuf, char *kbuf, size_t size);
