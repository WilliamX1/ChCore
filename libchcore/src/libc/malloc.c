#include <malloc.h>

#include <string.h>
#include <chcore/assert.h>
#include <chcore/capability.h>
#include <chcore/memory.h>

#define PMO_SIZE 0x1000
#define MAP_VA   0x1000000

#define MALLOC_SZ (50ull * 1024 * 1024)
static char *malloc_buf_;
static size_t malloc_header_ = 0;

void *malloc(size_t size)
{
        chcore_bug_on(malloc_header_ + size > MALLOC_SZ);

        if (malloc_header_ == 0) {
                int pmo_cap, r;
                pmo_cap = chcore_pmo_create(MALLOC_SZ, PMO_ANONYM);
                chcore_bug_on(pmo_cap < 0);
                r = chcore_pmo_map(
                        SELF_CAP, pmo_cap, MAP_VA, VM_READ | VM_WRITE);
                chcore_bug_on(r < 0);

                malloc_buf_ = (char *)MAP_VA;
        }

        void *ptr = (void *)&malloc_buf_[malloc_header_];
        malloc_header_ += size;

        return ptr;
}

void free(void *ptr)
{
        (void)ptr;
}

void *calloc(size_t nmemb, size_t size)
{
        (void)nmemb;
        (void)size;

        void *ptr = malloc(nmemb * size);
        memset(ptr, 0, nmemb * size);

        return ptr;
}
