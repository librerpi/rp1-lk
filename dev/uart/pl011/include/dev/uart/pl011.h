#pragma once

void pl011_uart_init(int irq, int nr, uintptr_t base);
void pl011_uart_init_early(int nr, uintptr_t base);
enum handler_return pl011_uart_irq(void *arg);
int uart_getc(int port, bool wait);
int pl011_uart_putc(int port, char c);
