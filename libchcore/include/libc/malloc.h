#pragma once

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);

#ifdef __cplusplus
}
#endif
