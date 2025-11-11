#include "buzzer.h"

// Registros GPIO
#define RCC_BASE        0x40023800
#define GPIOA_BASE      0x40020000
#define GPIOB_BASE      0x40020400
#define GPIOC_BASE      0x40020800

#define RCC_AHB1ENR     (*(volatile uint32_t*)(RCC_BASE + 0x30))

#define GPIOA_MODER     (*(volatile uint32_t*)(GPIOA_BASE + 0x00))
#define GPIOA_OTYPER    (*(volatile uint32_t*)(GPIOA_BASE + 0x04))
#define GPIOA_OSPEEDR   (*(volatile uint32_t*)(GPIOA_BASE + 0x08))
#define GPIOA_PUPDR     (*(volatile uint32_t*)(GPIOA_BASE + 0x0C))
#define GPIOA_ODR       (*(volatile uint32_t*)(GPIOA_BASE + 0x14))

#define GPIOB_MODER     (*(volatile uint32_t*)(GPIOB_BASE + 0x00))
#define GPIOB_OTYPER    (*(volatile uint32_t*)(GPIOB_BASE + 0x04))
#define GPIOB_OSPEEDR   (*(volatile uint32_t*)(GPIOB_BASE + 0x08))
#define GPIOB_PUPDR     (*(volatile uint32_t*)(GPIOB_BASE + 0x0C))
#define GPIOB_ODR       (*(volatile uint32_t*)(GPIOB_BASE + 0x14))

#define GPIOC_MODER     (*(volatile uint32_t*)(GPIOC_BASE + 0x00))
#define GPIOC_OTYPER    (*(volatile uint32_t*)(GPIOC_BASE + 0x04))
#define GPIOC_OSPEEDR   (*(volatile uint32_t*)(GPIOC_BASE + 0x08))
#define GPIOC_PUPDR     (*(volatile uint32_t*)(GPIOC_BASE + 0x0C))
#define GPIOC_ODR       (*(volatile uint32_t*)(GPIOC_BASE + 0x14))

// Notas musicales (Hz)
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
#define REST 0

// Feliz Cumpleaños
static int melody[] = {
    C5, C5, D5, C5, F5, E5, REST,
    C5, C5, D5, C5, G5, F5, REST,
    C5, C5, C6, A5, F5, E5, D5, REST,
    A5, A5, A5, F5, G5, F5, REST, REST
};

static int durations[] = {
    8, 16, 8, 8, 8, 4, 8,
    8, 16, 8, 8, 8, 4, 8,
    8, 16, 8, 8, 8, 8, 4, 8,
    8, 16, 8, 8, 8, 4, 4, 4
};

void Buzzer_Init(void) {
    // Habilitar clocks
    RCC_AHB1ENR |= (1 << 0) | (1 << 1) | (1 << 2);

    // PB6
    GPIOB_MODER &= ~(3 << (6*2));
    GPIOB_MODER |= (1 << (6*2));
    GPIOB_OTYPER &= ~(1 << 6);
    GPIOB_OSPEEDR |= (3 << (6*2));
    GPIOB_PUPDR &= ~(3 << (6*2));
    GPIOB_ODR &= ~(1 << 6);

    // PC7
    GPIOC_MODER &= ~(3 << (7*2));
    GPIOC_MODER |= (1 << (7*2));
    GPIOC_OTYPER &= ~(1 << 7);
    GPIOC_OSPEEDR |= (3 << (7*2));
    GPIOC_PUPDR &= ~(3 << (7*2));
    GPIOC_ODR &= ~(1 << 7);

    // PA9
    GPIOA_MODER &= ~(3 << (9*2));
    GPIOA_MODER |= (1 << (9*2));
    GPIOA_OTYPER &= ~(1 << 9);
    GPIOA_OSPEEDR |= (3 << (9*2));
    GPIOA_PUPDR &= ~(3 << (9*2));
    GPIOA_ODR &= ~(1 << 9);

    // PA10
    GPIOA_MODER &= ~(3 << (10*2));
    GPIOA_MODER |= (1 << (10*2));
    GPIOA_OTYPER &= ~(1 << 10);
    GPIOA_OSPEEDR |= (3 << (10*2));
    GPIOA_PUPDR &= ~(3 << (10*2));
    GPIOA_ODR &= ~(1 << 10);
}

void buzzer_play_christmas(void) {
    for(int i = 0; i < 28; i++) {
        int note_duration = 900 / durations[i];

        if(melody[i] != REST) {
            uint32_t period = 1000000 / melody[i];
            uint32_t cycles = (note_duration * 1000) / period;

            for(uint32_t c = 0; c < cycles; c++) {
                // Encender buzzers
                GPIOB_ODR |= (1 << 6);
                GPIOC_ODR |= (1 << 7);
                GPIOA_ODR |= (1 << 9);
                GPIOA_ODR |= (1 << 10);
                for(volatile uint32_t d = 0; d < period * 4; d++);

                // Apagar buzzers
                GPIOB_ODR &= ~(1 << 6);
                GPIOC_ODR &= ~(1 << 7);
                GPIOA_ODR &= ~(1 << 9);
                GPIOA_ODR &= ~(1 << 10);
                for(volatile uint32_t d = 0; d < period * 4; d++);
            }
        } else {
            for(volatile uint32_t d = 0; d < note_duration * 8000; d++);
        }

        for(volatile uint32_t d = 0; d < note_duration * 200; d++);
    }

    buzzer_stop_all();
}

void buzzer_play_tone(uint32_t frequency, uint32_t duration_ms) {
    if(frequency == 0) {
        for(volatile uint32_t d = 0; d < duration_ms * 8000; d++);
        return;
    }

    uint32_t period = 1000000 / frequency;
    uint32_t cycles = (duration_ms * 1000) / period;

    for(uint32_t c = 0; c < cycles; c++) {
        GPIOB_ODR |= (1 << 6);
        GPIOC_ODR |= (1 << 7);
        GPIOA_ODR |= (1 << 9);
        GPIOA_ODR |= (1 << 10);
        for(volatile uint32_t d = 0; d < period * 4; d++);

        GPIOB_ODR &= ~(1 << 6);
        GPIOC_ODR &= ~(1 << 7);
        GPIOA_ODR &= ~(1 << 9);
        GPIOA_ODR &= ~(1 << 10);
        for(volatile uint32_t d = 0; d < period * 4; d++);
    }
}

void buzzer_stop_all(void) {
    GPIOB_ODR &= ~(1 << 6);
    GPIOC_ODR &= ~(1 << 7);
    GPIOA_ODR &= ~(1 << 9);
    GPIOA_ODR &= ~(1 << 10);
}
