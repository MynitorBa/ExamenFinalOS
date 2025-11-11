#include "button.h"
#include <stdint.h>

/* ============================================================================ */
/*                     DEFINICIONES DE REGISTROS                                */
/* ============================================================================ */

#define RCC_AHB1ENR (*(volatile uint32_t *)(0x40023830))

/* ============================================================================ */
/*                     BOTÓN AZUL (PC13) CON POLLING                            */
/* ============================================================================ */

/* Registros GPIO */
#define GPIOC_MODER (*(volatile uint32_t *)(0x40020800))
#define GPIOC_PUPDR (*(volatile uint32_t *)(0x4002080C))
#define GPIOC_IDR (*(volatile uint32_t *)(0x40020810))

/* Variables para detección de flanco */
static int blue_last_state = 0;
static int blue_was_pressed_flag = 0;

void button_init(void) {
    /* Habilitar clock GPIOC */
    RCC_AHB1ENR |= (1 << 2);

    /* Configurar PC13 como input con pull-up */
    GPIOC_MODER &= ~(0x3 << 26);
    GPIOC_PUPDR &= ~(0x3 << 26);
    GPIOC_PUPDR |= (0x1 << 26);

    blue_last_state = 0;
    blue_was_pressed_flag = 0;
}

int button_read(void) {
    return !(GPIOC_IDR & (1 << 13));
}

void button_update(void) {
    int current_state = button_read();

    if (current_state && !blue_last_state) {
        blue_was_pressed_flag = 1;
    }

    blue_last_state = current_state;
}

int button_was_pressed(void) {
    if (blue_was_pressed_flag) {
        blue_was_pressed_flag = 0;
        return 1;
    }
    return 0;
}

/* ============================================================================ */
/*                     10 BOTONES (D2-D9, D14-D15) CON POLLING                  */
/* ============================================================================ */

#define GPIOA_BASE   0x40020000
#define GPIOB_BASE   0x40020400
#define GPIOC_BASE   0x40020800

typedef struct {
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFR[2];
} GPIO_TypeDef_Buttons;

#define GPIOA   ((GPIO_TypeDef_Buttons*)GPIOA_BASE)
#define GPIOB   ((GPIO_TypeDef_Buttons*)GPIOB_BASE)
#define GPIOC   ((GPIO_TypeDef_Buttons*)GPIOC_BASE)

typedef struct {
    GPIO_TypeDef_Buttons* port;
    int pin;
    const char* name;
    const char* label;
} Button;

static Button buttons[10] = {
    {GPIOA, 10, "D2", "Arriba P1    "},
    {GPIOB,  3, "D3", "Abajo P1     "},
    {GPIOB,  5, "D4", "Izquierda P1 "},
    {GPIOB,  4, "D5", "Derecha P1   "},
    {GPIOB, 10, "D6", "Arriba P2    "},
    {GPIOA,  8, "D7", "Abajo P2     "},
    {GPIOA,  9, "D8", "Izquierda P2 "},
    {GPIOC,  7, "D9", "Derecha P2   "},
    {GPIOB,  9, "D14", "SDA          "},
    {GPIOB,  8, "D15", "SCL          "}
};

static int press_count[10] = {0};
static int last_state[10] = {0};
static int was_pressed_flag[10] = {0};

void buttons_init(void) {
    RCC_AHB1ENR |= (1 << 0) | (1 << 1) | (1 << 2);

    for (int i = 0; i < 10; i++) {
        GPIO_TypeDef_Buttons* port = buttons[i].port;
        int pin = buttons[i].pin;

        port->MODER &= ~(3 << (pin * 2));
        port->PUPDR &= ~(3 << (pin * 2));
        port->PUPDR |= (1 << (pin * 2));
    }
}

int button_read_index(int index) {
    if (index < 0 || index >= 10) return 0;
    return !(buttons[index].port->IDR & (1 << buttons[index].pin));
}

uint32_t button_get_press_count_index(int index) {
    if (index < 0 || index >= 10) return 0;
    return press_count[index];
}

const char* button_get_name(int index) {
    if (index < 0 || index >= 10) return "";
    return buttons[index].name;
}

const char* button_get_label(int index) {
    if (index < 0 || index >= 10) return "";
    return buttons[index].label;
}

void buttons_update(void) {
    for (int i = 0; i < 10; i++) {
        int current = button_read_index(i);

        if (current && !last_state[i]) {
            press_count[i]++;
            was_pressed_flag[i] = 1;
        }

        last_state[i] = current;
    }
}

int button_check_any_pressed(void) {
    for (int i = 0; i < 10; i++) {
        if (was_pressed_flag[i]) {
            was_pressed_flag[i] = 0;
            return 1;
        }
    }
    return 0;
}

void buttons_reset_counts(void) {
    for (int i = 0; i < 10; i++) {
        press_count[i] = 0;
    }
}
