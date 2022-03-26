#pragma once

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

void memset(void *dest, int c, size_t n);
void memcpy(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);

int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);

size_t strlen(const char *s);

char *strstr(const char *haystack, const char *needle);

char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t n);

#ifdef __cplusplus
}
#endif
