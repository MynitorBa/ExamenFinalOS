#pragma once // Directiva alternativa de guarda
#include <stdint.h> // Incluye tipos de enteros fijos

/** Inicializa el periférico UART (Universal Asynchronous Receiver-Transmitter). */
void uart_init(void);

/**
 * Envía un solo carácter a través de la UART.
 * @param c Carácter a enviar.
 */
void uart_putc(char c);

/**
 * Envía una cadena de caracteres (string) a través de la UART.
 * @param str Puntero a la cadena a enviar.
 */
void uart_puts(const char *str);

/**
 * Envía un entero sin signo (uint32_t) a través de la UART, convertido a texto.
 * @param num Número a enviar.
 */
void uart_putint(uint32_t num);

/** Envía un carácter de nueva línea a través de la UART. */
void uart_newline(void);
