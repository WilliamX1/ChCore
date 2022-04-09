#include <chcore/internal/server_caps.h>

/* Defined in __libchcore_init.c */
extern int __chcore_procm_cap;

int __chcore_get_procm_cap(void)
{
        return __chcore_procm_cap;
}

void __chcore_set_procm_cap(int cap)
{
        __chcore_procm_cap = cap;
}
