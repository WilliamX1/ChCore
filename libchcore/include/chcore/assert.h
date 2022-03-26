#pragma once

#include <chcore/console.h>

#define chcore_bug(str)                                              \
        do {                                                         \
                chcore_console_printf(                               \
                        "BUG: %s:%d %s\n", __func__, __LINE__, str); \
                for (;;) {                                           \
                }                                                    \
        } while (0)

#define chcore_bug_on(expr)                                                    \
        do {                                                                   \
                if ((expr)) {                                                  \
                        chcore_console_printf(                                 \
                                "BUG: %s:%d %s\n", __func__, __LINE__, #expr); \
                        for (;;) {                                             \
                        }                                                      \
                }                                                              \
        } while (0)

#define chcore_warn(msg) \
        chcore_console_printf("WARN: %s:%d %s\n", __func__, __LINE__, msg)

#define chcore_warn_on(cond, msg)                                              \
        do {                                                                   \
                if ((cond)) {                                                  \
                        chcore_console_printf("WARN: %s:%d %s on " #cond "\n", \
                                              __func__,                        \
                                              __LINE__,                        \
                                              msg);                            \
                }                                                              \
        } while (0)

#define chcore_assert(expr)                \
        do {                               \
                if (!(expr)) {             \
                        chcore_bug(#expr); \
                }                          \
        } while (0)
