#pragma once

#include <common/types.h>

void uart_init(void);
u32 uart_recv(void);
void uart_send(u32 c);

#define NB_UART_NRET ((u32)-1)
/* Non-blocking receive */
u32 nb_uart_recv(void);
