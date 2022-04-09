#pragma once

#include <sys/types.h>
#include <chcore/memory.h>
#include <chcore/internal/idman.h>

int spawn(const char *filename, int *new_thread_cap);
