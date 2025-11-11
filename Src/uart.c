#include "uart.h"

/* Registros UART2 */
#define RCC_APB1ENR (*(volatile uint32_t *)(0x40023840))
#define RCC_AHB1ENR (*(volatile uint32_t *)(0x40023830))

#define GPIOA_MODER (*(volatile uint32_t *)(0x40020000))
#define GPIOA_AFRL (*(volatile uint32_t *)(0x40020020))

#define USART2_SR (*(volatile uint32_t *)(0x40004400))
#define USART2_DR (*(volatile uint32_t *)(0x40004404))
#define USART2_BRR (*(volatile uint32_t *)(0x40004408))
#define USART2_CR1 (*(volatile uint32_t *)(0x4000440C))

#define USART_SR_TXE (1 << 7)
#define USART_SR_TC (1 << 6)
#define USART_CR1_TE (1 << 3)
#define USART_CR1_RE (1 << 2)
#define USART_CR1_UE (1 << 13)

void uart_init(void) {
// Habilitar clocks
RCC_AHB1ENR |= (1 << 0); // GPIOA
RCC_APB1ENR |= (1 << 17); // USART2

// Configurar PA2 (TX) y PA3 (RX) como función alternativa
GPIOA_MODER &= ~((3 << 4) | (3 << 6));
GPIOA_MODER |= (2 << 4) | (2 << 6);

// Seleccionar AF7 (USART2)
GPIOA_AFRL &= ~((0xF << 8) | (0xF << 12));
GPIOA_AFRL |= (7 << 8) | (7 << 12);

// Baudrate 115200 @ 16MHz
USART2_BRR = 139;

// Habilitar TX, RX y USART
USART2_CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

void uart_putc(char c) {
while (!(USART2_SR & USART_SR_TXE));
USART2_DR = c;
}

void uart_puts(const char *str) {
while (*str) {
uart_putc(*str++);
}
}

void uart_putint(uint32_t num) {
char buffer[11];
int i = 0;

if (num == 0) {
uart_putc('0');
return;
}

while (num > 0) {
buffer[i++] = '0' + (num % 10);
num /= 10;
}

while (i > 0) {
uart_putc(buffer[--i]);
}
}

void uart_newline(void) {
uart_putc('\r');
uart_putc('\n');
}
