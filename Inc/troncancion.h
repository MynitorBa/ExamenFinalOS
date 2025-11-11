#ifndef TRONCANCION_H // Guarda de inclusión para la música/sonidos de TRON
#define TRONCANCION_H

#include <stdint.h> // Incluye tipos de enteros fijos

// Definiciones de registros GPIO
/** Dirección base del Periférico GPIOA. */
#define GPIOA_BASE      0x40020000
/** Dirección base del Periférico GPIOB. */
#define GPIOB_BASE      0x40020400
/** Dirección base del Periférico GPIOC. */
#define GPIOC_BASE      0x40020800

/** Puntero volátil al Registro de Datos de Salida (ODR) de GPIOA. */
#define GPIOA_ODR       (*(volatile uint32_t*)(GPIOA_BASE + 0x14))
/** Puntero volátil al Registro de Datos de Salida (ODR) de GPIOB. */
#define GPIOB_ODR       (*(volatile uint32_t*)(GPIOB_BASE + 0x14))
/** Puntero volátil al Registro de Datos de Salida (ODR) de GPIOC. */
#define GPIOC_ODR       (*(volatile uint32_t*)(GPIOC_BASE + 0x14))

// Notas musicales (frecuencias en Hz)
/** Nota Do central (262 Hz). */
#define C4   262
/** Nota Re. */
#define D4   294
/** Nota Mi. */
#define E4   330
/** Nota Fa. */
#define F4   349
/** Nota Sol. */
#define G4   392
/** Nota La. */
#define A4   440
/** Nota Si. */
#define B4   494
/** Nota Do alto. */
#define C5   523
/** Nota Re. */
#define D5   587
/** Nota Mi. */
#define E5   659
/** Nota Fa. */
#define F5   698
/** Nota Sol. */
#define G5   784
/** Nota La. */
#define A5   880
/** Nota Si. */
#define B5   988
/** Nota Do. */
#define C6   1047
/** Nota Re. */
#define D6   1175
/** Nota Mi. */
#define E6   1319
/** Frecuencia de reposo (silencio). */
#define REST 0

// Funciones públicas
/**
 * Retardo activo (busy wait) en microsegundos.
 * @param us Microsegundos de retardo.
 */
void delay_us(uint32_t us);
/**
 * Retardo activo (busy wait) en milisegundos.
 * @param ms Milisegundos de retardo.
 */
void delay_ms(uint32_t ms);
/**
 * Genera un tono con vibrato.
 * @param frequency Frecuencia del tono.
 * @param duration_ms Duración total.
 */
void tone_vibrato(uint16_t frequency, uint32_t duration_ms);
/**
 * Genera un tono simple.
 * @param frequency Frecuencia del tono.
 * @param duration_ms Duración total.
 */
void tone_all(uint16_t frequency, uint32_t duration_ms);
/** Detiene la generación de tonos. */
void noTone_all(void);
/**
 * Reproduce un efecto de arpegio al estilo TRON.
 * @param base_freq Frecuencia inicial.
 */
void arpegio_tron(uint16_t base_freq);
/** Reproduce la melodía navideña de TRON. */
void playTronChristmas(void);
/** Reproduce el efecto de sonido de introducción de TRON. */
void introTron(void);
/** Reproduce un efecto de sonido de fallo/glitch. */
void glitchEffect(void);
/** Reproduce un efecto de sonido de potenciador (power up). */
void powerUp(void);
/** Reproduce el efecto de pulso de red/rejilla. */
void gridPulse(void);
/** Reproduce el efecto de sonido de cierre (outro). */
void outroTron(void);

#endif // TRONCANCION_H
