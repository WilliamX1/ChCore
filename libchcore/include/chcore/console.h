/*
 * ChCore APIs for debug console, i.e.
 * the UART device drived by the kernel.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void chcore_console_putc(int ch);
int chcore_console_getc(void);

void chcore_console_puts(const char *str);
int chcore_console_printf(const char *fmt, ...);

#ifdef __cplusplus
}
#endif
