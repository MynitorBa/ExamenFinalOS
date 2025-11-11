#include "button.h"
#include <stdint.h>

/* ============================================================================ */
/* DEFINICIONES DE REGISTROS                                */
/* ============================================================================ */

/** Puntero al registro de habilitación de Clock de Periféricos AHB1 (RCC_AHB1ENR). */
#define RCC_AHB1ENR (*(volatile uint32_t *)(0x40023830))

/* ============================================================================ */
/* BOTÓN AZUL (PC13) CON POLLING                            */
/* ============================================================================ */

/* Registros GPIO del Puerto C */
#define GPIOC_MODER (*(volatile uint32_t *)(0x40020800)) /** Registro Modo (Input/Output). */
#define GPIOC_PUPDR (*(volatile uint32_t *)(0x4002080C)) /** Registro Pull-up/Pull-down. */
#define GPIOC_IDR (*(volatile uint32_t *)(0x40020810))   /** Registro Datos de Entrada. */

/* Variables para detección de flanco */
static int blue_last_state = 0;        /** Estado previo del botón azul. */
static int blue_was_pressed_flag = 0;  /** Bandera para detección de flanco ascendente (presión). */

/** Inicializa el botón azul (PC13) como input con pull-up. */
void button_init(void) {
    /* Habilitar clock GPIOC */
    RCC_AHB1ENR |= (1 << 2);

    /* Configurar PC13 como input (MODER) con pull-up (PUPDR) */
    GPIOC_MODER &= ~(0x3 << 26);
    GPIOC_PUPDR &= ~(0x3 << 26);
    GPIOC_PUPDR |= (0x1 << 26);

    blue_last_state = 0;
    blue_was_pressed_flag = 0;
}

/**
 * Lee el estado actual del botón azul (PC13).
 * @return 1 si está presionado (lógica negada), 0 si está liberado.
 */
int button_read(void) {
    return !(GPIOC_IDR & (1 << 13));
}

/** Actualiza las variables de estado para detección de flanco. */
void button_update(void) {
    int current_state = button_read();

    // Detección de flanco ascendente (presionado y el estado anterior era liberado)
    if (current_state && !blue_last_state) {
        blue_was_pressed_flag = 1;
    }

    blue_last_state = current_state;
}

/**
 * Verifica si el botón azul fue presionado desde la última llamada.
 * @return 1 si se detectó una presión, 0 si no.
 */
int button_was_pressed(void) {
    if (blue_was_pressed_flag) {
        blue_was_pressed_flag = 0;
        return 1;
    }
    return 0;
}

/* ============================================================================ */
/* 10 BOTONES (D2-D9, D14-D15) CON POLLING                  */
/* ============================================================================ */

#define GPIOA_BASE   0x40020000
#define GPIOB_BASE   0x40020400
#define GPIOC_BASE   0x40020800

/** Estructura de mapeo de registros GPIO (parcial). */
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

/** Estructura para describir un botón. */
typedef struct {
    GPIO_TypeDef_Buttons* port; /** Puerto GPIO al que pertenece. */
    int pin;                    /** Número de pin. */
    const char* name;           /** Nombre del pin (ej. "D2"). */
    const char* label;          /** Etiqueta funcional (ej. "Arriba P1"). */
} Button;

/** Array que define los 10 botones del sistema. */
static Button buttons[10] = {
    {GPIOA, 10, "D2", "Arriba P1    "},
    {GPIOB,  3, "D3", "Abajo P1     "},
    {GPIOB,  5, "D4", "Izquierda P1 "},
    {GPIOB,  4, "D5", "Derecha P1   "},
    {GPIOB, 10, "D6", "Arriba P2    "},
    {GPIOA,  8, "D7", "Abajo P2     "},
    {GPIOA,  9, "D8", "Izquierda P2 "},
    {GPIOC,  7, "D9", "Derecha P2   "},
    {GPIOB,  9, "D14", "SDA          "}, // Mapeado como botón
    {GPIOB,  8, "D15", "SCL          "}  // Mapeado como botón
};

static int press_count[10] = {0};    /** Conteo de presiones por botón. */
static int last_state[10] = {0};     /** Último estado leído por botón. */
static int was_pressed_flag[10] = {0};/** Bandera de nueva presión por botón. */

/** Inicializa los 10 botones como inputs con pull-up. */
void buttons_init(void) {
    /* Habilitar clocks de GPIOA, GPIOB y GPIOC */
    RCC_AHB1ENR |= (1 << 0) | (1 << 1) | (1 << 2);

    for (int i = 0; i < 10; i++) {
        GPIO_TypeDef_Buttons* port = buttons[i].port;
        int pin = buttons[i].pin;

        // Configurar como Input y habilitar Pull-up
        port->MODER &= ~(3 << (pin * 2));
        port->PUPDR &= ~(3 << (pin * 2));
        port->PUPDR |= (1 << (pin * 2));
    }
}

/**
 * Lee el estado de un botón por su índice.
 * @param index Índice del botón (0-9).
 * @return 1 si presionado, 0 si liberado.
 */
int button_read_index(int index) {
    if (index < 0 || index >= 10) return 0;
    // Lectura, NOT del pin IDR (asume pull-up, activo bajo)
    return !(buttons[index].port->IDR & (1 << buttons[index].pin));
}

/**
 * Obtiene el contador de presiones de un botón.
 * @param index Índice del botón.
 * @return Contador de presiones.
 */
uint32_t button_get_press_count_index(int index) {
    if (index < 0 || index >= 10) return 0;
    return press_count[index];
}

/**
 * Obtiene el nombre del pin del botón.
 * @param index Índice del botón.
 * @return Nombre del pin (ej: "D2").
 */
const char* button_get_name(int index) {
    if (index < 0 || index >= 10) return "";
    return buttons[index].name;
}

/**
 * Obtiene la etiqueta funcional del botón.
 * @param index Índice del botón.
 * @return Etiqueta (ej: "Arriba P1").
 */
const char* button_get_label(int index) {
    if (index < 0 || index >= 10) return "";
    return buttons[index].label;
}

/** Actualiza el estado y los contadores de todos los 10 botones. */
void buttons_update(void) {
    for (int i = 0; i < 10; i++) {
        int current = button_read_index(i);

        // Detección de flanco ascendente
        if (current && !last_state[i]) {
            press_count[i]++;
            was_pressed_flag[i] = 1;
        }

        last_state[i] = current;
    }
}

/**
 * Verifica si algún botón ha sido presionado desde la última verificación.
 * @return 1 si se detectó alguna nueva presión, 0 si no.
 */
int button_check_any_pressed(void) {
    for (int i = 0; i < 10; i++) {
        if (was_pressed_flag[i]) {
            was_pressed_flag[i] = 0; // Se consume el evento
            return 1;
        }
    }
    return 0;
}

/** Resetea los contadores de presiones de todos los botones. */
void buttons_reset_counts(void) {
    for (int i = 0; i < 10; i++) {
        press_count[i] = 0;
    }
}
