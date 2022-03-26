#pragma once

#include <chcore/types_arch.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __cplusplus
typedef int bool;
#define true (1)
#define false (0)
#endif

#ifdef __cplusplus
#define __CHCORE_NULL 0
#else
#define __CHCORE_NULL ((void *)0)
#endif

#ifndef NULL
#define NULL __CHCORE_NULL
#endif

#ifdef __cplusplus
}
#endif
