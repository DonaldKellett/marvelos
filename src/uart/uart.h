#ifndef UART_H
#define UART_H

#include <stddef.h>

// 0x10000000 is memory-mapped address of UART according to device tree
#define UART_ADDR 0x10000000

void uart_init(size_t);
int kputchar(int);
int kputs(const char *);
void kprintf(const char *format, ...);

#endif