#ifndef TRONCANCION_H
#define TRONCANCION_H

#include <stdint.h>

// Definiciones de registros GPIO
#define GPIOA_BASE      0x40020000
#define GPIOB_BASE      0x40020400
#define GPIOC_BASE      0x40020800

#define GPIOA_ODR       (*(volatile uint32_t*)(GPIOA_BASE + 0x14))
#define GPIOB_ODR       (*(volatile uint32_t*)(GPIOB_BASE + 0x14))
#define GPIOC_ODR       (*(volatile uint32_t*)(GPIOC_BASE + 0x14))

// Notas musicales (frecuencias en Hz)
#define C4   262
#define D4   294
#define E4   330
#define F4   349
#define G4   392
#define A4   440
#define B4   494
#define C5   523
#define D5   587
#define E5   659
#define F5   698
#define G5   784
#define A5   880
#define B5   988
#define C6   1047
#define D6   1175
#define E6   1319
#define REST 0

// Funciones públicas
void delay_us(uint32_t us);
void delay_ms(uint32_t ms);
void tone_vibrato(uint16_t frequency, uint32_t duration_ms);
void tone_all(uint16_t frequency, uint32_t duration_ms);
void noTone_all(void);
void arpegio_tron(uint16_t base_freq);
void playTronChristmas(void);
void introTron(void);
void glitchEffect(void);
void powerUp(void);
void gridPulse(void);
void outroTron(void);

#endif // TRONCANCION_H
