#include "aleatorio.h"
#include "sched.h"  // Para millis() y get_current_task_id()

// ========================================================================
// GENERADOR XORSHIFT32
// ========================================================================
static uint32_t estado = 123456789;
static uint32_t contador_mezclas = 0;

/**
 * Obtiene entropía del hardware
 */
static uint32_t obtener_entropia_sistema(void) {
    uint32_t entropia = 0;

    // FUENTE 1: Milisegundos del sistema
    entropia ^= millis();

    // FUENTE 2: Segundos
    entropia ^= (millis() / 1000) * 2654435761U;

    // FUENTE 3: Dirección de la pila
    volatile uint8_t stack_marker = 0;
    entropia ^= (uint32_t)(uintptr_t)&stack_marker;

    // FUENTE 4: ID de la tarea actual
    entropia ^= ((uint32_t)get_current_task_id()) << 24;

    // FUENTE 5: Contador interno
    contador_mezclas++;
    entropia ^= contador_mezclas * 1103515245;

    return entropia;
}

/**
 * Función hash para mezclar
 */
static uint32_t hash_mezcla(uint32_t x) {
    x ^= x >> 16;
    x *= 0x85ebca6b;
    x ^= x >> 13;
    x *= 0xc2b2ae35;
    x ^= x >> 16;
    return x;
}

// ========================================================================
// IMPLEMENTACIÓN PÚBLICA
// ========================================================================

void aleatorio_init(void) {
    task_delay(5);

    uint32_t entropia1 = obtener_entropia_sistema();
    task_delay(3);
    uint32_t entropia2 = obtener_entropia_sistema();
    task_delay(2);
    uint32_t entropia3 = obtener_entropia_sistema();

    estado = hash_mezcla(entropia1 ^ entropia2 ^ entropia3);

    if (estado == 0) {
        estado = 0xDEADBEEF;
    }

    for (uint8_t i = 0; i < 20; i++) {
        aleatorio_obtener();
    }

    contador_mezclas = 0;
}

uint32_t aleatorio_obtener(void) {
    if ((contador_mezclas & 0xFF) == 0) {
        estado ^= hash_mezcla(millis());
    }

    estado ^= estado << 13;
    estado ^= estado >> 17;
    estado ^= estado << 5;

    contador_mezclas++;
    return estado;
}

uint32_t aleatorio_rango(uint32_t min, uint32_t max) {
    if (min >= max) return min;

    uint32_t rango = max - min + 1;
    uint32_t rand = aleatorio_obtener();

    return min + (rand % rango);
}

uint8_t aleatorio_byte(void) {
    return (uint8_t)(aleatorio_obtener() & 0xFF);
}

void aleatorio_refrescar_semilla(void) {
    uint32_t nueva_entropia = obtener_entropia_sistema();
    estado ^= hash_mezcla(nueva_entropia);
}
