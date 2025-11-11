#include "buzzer.h"
#include <stdint.h>

// Registros GPIO
/** Dirección base del Periférico RCC. */
#define RCC_BASE        0x40023800
/** Dirección base del Periférico GPIOA. */
#define GPIOA_BASE      0x40020000
/** Dirección base del Periférico GPIOB. */
#define GPIOB_BASE      0x40020400
/** Dirección base del Periférico GPIOC. */
#define GPIOC_BASE      0x40020800

/** Registro de Habilitación de Clock AHB1. */
#define RCC_AHB1ENR     (*(volatile uint32_t*)(RCC_BASE + 0x30))

// Registros de GPIOA
#define GPIOA_MODER     (*(volatile uint32_t*)(GPIOA_BASE + 0x00))
#define GPIOA_OTYPER    (*(volatile uint32_t*)(GPIOA_BASE + 0x04))
#define GPIOA_OSPEEDR   (*(volatile uint32_t*)(GPIOA_BASE + 0x08))
#define GPIOA_PUPDR     (*(volatile uint32_t*)(GPIOA_BASE + 0x0C))
#define GPIOA_ODR       (*(volatile uint32_t*)(GPIOA_BASE + 0x14)) /** Registro de Datos de Salida. */

// Registros de GPIOB
#define GPIOB_MODER     (*(volatile uint32_t*)(GPIOB_BASE + 0x00))
#define GPIOB_OTYPER    (*(volatile uint32_t*)(GPIOB_BASE + 0x04))
#define GPIOB_OSPEEDR   (*(volatile uint32_t*)(GPIOB_BASE + 0x08))
#define GPIOB_PUPDR     (*(volatile uint32_t*)(GPIOB_BASE + 0x0C))
#define GPIOB_ODR       (*(volatile uint32_t*)(GPIOB_BASE + 0x14)) /** Registro de Datos de Salida. */

// Registros de GPIOC
#define GPIOC_MODER     (*(volatile uint32_t*)(GPIOC_BASE + 0x00))
#define GPIOC_OTYPER    (*(volatile uint32_t*)(GPIOC_BASE + 0x04))
#define GPIOC_OSPEEDR   (*(volatile uint32_t*)(GPIOC_BASE + 0x08))
#define GPIOC_PUPDR     (*(volatile uint32_t*)(GPIOC_BASE + 0x0C))
#define GPIOC_ODR       (*(volatile uint32_t*)(GPIOC_BASE + 0x14)) /** Registro de Datos de Salida. */

// Notas musicales (Hz)
/** Do 4. */
#define C4   262
/** Re 4. */
#define D4   294
/** Mi 4. */
#define E4   330
/** Fa 4. */
#define F4   349
/** Sol 4. */
#define G4   392
/** La 4. */
#define A4   440
/** Si 4. */
#define B4   494
/** Do 5. */
#define C5   523
/** Re 5. */
#define D5   587
/** Mi 5. */
#define E5   659
/** Fa 5. */
#define F5   698
/** Sol 5. */
#define G5   784
/** La 5. */
#define A5   880
/** Si 5. */
#define B5   988
/** Do 6. */
#define C6   1047
/** Silencio. */
#define REST 0

// Feliz Cumpleaños
/** Frecuencias de las notas de la melodía. */
static int melody[] = {
    C5, C5, D5, C5, F5, E5, REST,
    C5, C5, D5, C5, G5, F5, REST,
    C5, C5, C6, A5, F5, E5, D5, REST,
    A5, A5, A5, F5, G5, F5, REST, REST
};

/** Duraciones de las notas (inverso a la longitud). */
static int durations[] = {
    8, 16, 8, 8, 8, 4, 8,
    8, 16, 8, 8, 8, 4, 8,
    8, 16, 8, 8, 8, 8, 4, 8,
    8, 16, 8, 8, 8, 4, 4, 4
};

/** Inicializa los pines PB6, PC7, PA9 y PA10 como salidas push-pull de alta velocidad. */
void Buzzer_Init(void) {
    // Habilitar clocks de GPIOA, GPIOB y GPIOC
    RCC_AHB1ENR |= (1 << 0) | (1 << 1) | (1 << 2);

    // Configuración de PB6 como salida
    GPIOB_MODER &= ~(3 << (6*2));
    GPIOB_MODER |= (1 << (6*2));
    GPIOB_OTYPER &= ~(1 << 6);
    GPIOB_OSPEEDR |= (3 << (6*2));
    GPIOB_PUPDR &= ~(3 << (6*2));
    GPIOB_ODR &= ~(1 << 6); // Inicializar en LOW

    // Configuración de PC7 como salida
    GPIOC_MODER &= ~(3 << (7*2));
    GPIOC_MODER |= (1 << (7*2));
    GPIOC_OTYPER &= ~(1 << 7);
    GPIOC_OSPEEDR |= (3 << (7*2));
    GPIOC_PUPDR &= ~(3 << (7*2));
    GPIOC_ODR &= ~(1 << 7); // Inicializar en LOW

    // Configuración de PA9 como salida
    GPIOA_MODER &= ~(3 << (9*2));
    GPIOA_MODER |= (1 << (9*2));
    GPIOA_OTYPER &= ~(1 << 9);
    GPIOA_OSPEEDR |= (3 << (9*2));
    GPIOA_PUPDR &= ~(3 << (9*2));
    GPIOA_ODR &= ~(1 << 9); // Inicializar en LOW

    // Configuración de PA10 como salida
    GPIOA_MODER &= ~(3 << (10*2));
    GPIOA_MODER |= (1 << (10*2));
    GPIOA_OTYPER &= ~(1 << 10);
    GPIOA_OSPEEDR |= (3 << (10*2));
    GPIOA_PUPDR &= ~(3 << (10*2));
    GPIOA_ODR &= ~(1 << 10); // Inicializar en LOW
}

/** Reproduce la melodía "Feliz Cumpleaños" (mapeada a play_christmas en el header). */
void buzzer_play_christmas(void) {
    for(int i = 0; i < 28; i++) {
        // Calcular la duración real de la nota en ms (ajustado por el tempo base 900)
        int note_duration = 900 / durations[i];

        if(melody[i] != REST) {
            // Calcular el período de la onda (en microsegundos)
            uint32_t period = 1000000 / melody[i];
            // Calcular el número total de ciclos (periodos) a generar
            uint32_t cycles = (note_duration * 1000) / period;

            for(uint32_t c = 0; c < cycles; c++) {
                // Ciclo HIGH (Encender buzzers)
                GPIOB_ODR |= (1 << 6);
                GPIOC_ODR |= (1 << 7);
                GPIOA_ODR |= (1 << 9);
                GPIOA_ODR |= (1 << 10);
                for(volatile uint32_t d = 0; d < period * 4; d++); // Retardo HIGH

                // Ciclo LOW (Apagar buzzers)
                GPIOB_ODR &= ~(1 << 6);
                GPIOC_ODR &= ~(1 << 7);
                GPIOA_ODR &= ~(1 << 9);
                GPIOA_ODR &= ~(1 << 10);
                for(volatile uint32_t d = 0; d < period * 4; d++); // Retardo LOW
            }
        } else {
            // Silencio: Retardo simple (busy wait)
            for(volatile uint32_t d = 0; d < note_duration * 8000; d++);
        }

        // Retardo entre notas para separación
        for(volatile uint32_t d = 0; d < note_duration * 200; d++);
    }

    buzzer_stop_all();
}

/**
 * Genera un tono de frecuencia y duración específicas usando polling.
 * @param frequency Frecuencia del tono en Hz.
 * @param duration_ms Duración del tono en milisegundos.
 */
void buzzer_play_tone(uint32_t frequency, uint32_t duration_ms) {
    if(frequency == 0) {
        // Si la frecuencia es 0 (REST), solo espera
        for(volatile uint32_t d = 0; d < duration_ms * 8000; d++);
        return;
    }

    // Calcular período y ciclos totales
    uint32_t period = 1000000 / frequency; // Periodo en us
    uint32_t cycles = (duration_ms * 1000) / period; // Número de ciclos de la onda

    for(uint32_t c = 0; c < cycles; c++) {
        // HIGH
        GPIOB_ODR |= (1 << 6);
        GPIOC_ODR |= (1 << 7);
        GPIOA_ODR |= (1 << 9);
        GPIOA_ODR |= (1 << 10);
        for(volatile uint32_t d = 0; d < period * 4; d++);

        // LOW
        GPIOB_ODR &= ~(1 << 6);
        GPIOC_ODR &= ~(1 << 7);
        GPIOA_ODR &= ~(1 << 9);
        GPIOA_ODR &= ~(1 << 10);
        for(volatile uint32_t d = 0; d < period * 4; d++);
    }
}

/** Detiene la generación de tonos, poniendo todos los pines en LOW. */
void buzzer_stop_all(void) {
    GPIOB_ODR &= ~(1 << 6);
    GPIOC_ODR &= ~(1 << 7);
    GPIOA_ODR &= ~(1 << 9);
    GPIOA_ODR &= ~(1 << 10);
}
