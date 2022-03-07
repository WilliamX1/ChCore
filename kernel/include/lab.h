#pragma once

#include <lib/printk.h>

#define lab_check(expr, test_name)                                        \
        do {                                                              \
                if (expr) {                                               \
                        printk("[TEST] %s: OK\n", test_name);             \
                } else {                                                  \
                        printk("[TEST] %s: FAIL, loc: %s:%d, expr: %s\n", \
                               test_name,                                 \
                               __FILE__,                                  \
                               __LINE__,                                  \
                               #expr);                                    \
                }                                                         \
        } while (0)

#define lab_assert(expr)           \
        do {                       \
                ok = ok && (expr); \
        } while (0)
