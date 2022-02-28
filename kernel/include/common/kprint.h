#pragma once

#include <lib/printk.h>

#define WARNING 0
#define INFO    1
#define DEBUG   2

/* LOG_LEVEL is INFO by default */

#if LOG_LEVEL >= WARNING
#define kwarn(fmt, ...) printk("[WARN] file:%s " fmt, __FILE__, ##__VA_ARGS__)
#else
#define kwarn(fmt, ...)
#endif

#if LOG_LEVEL >= INFO
#define kinfo(fmt, ...) printk("[INFO] " fmt, ##__VA_ARGS__)
#else
#define kinfo(fmt, ...)
#endif

#if LOG_LEVEL >= DEBUG
#define kdebug(fmt, ...) printk("[DEBUG] " fmt, ##__VA_ARGS__)
#else
#define kdebug(fmt, ...)
#endif
