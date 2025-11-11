#pragma once
#include <stdint.h>

void uart_init(void);
void uart_putc(char c);
void uart_puts(const char *str);
void uart_putint(uint32_t num);
void uart_newline(void);
