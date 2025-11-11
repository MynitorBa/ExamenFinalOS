#include "aleatorio.h"
#include "sched.h"  // Para millis() y get_current_task_id()
#include <stddef.h> // Para size_t
#include <stdint.h> // Para uint32_t

// ========================================================================
// GENERADOR XORSHIFT32
// ========================================================================
/** Estado interno del generador pseudoaleatorio (semilla). */
static uint32_t estado = 123456789;
/** Contador para forzar la mezcla de entropía periódicamente. */
static uint32_t contador_mezclas = 0;

/**
 * Obtiene entropía del hardware mezclando fuentes de bajo nivel.
 * @return Un valor uint32_t basado en varias fuentes del sistema.
 */
static uint32_t obtener_entropia_sistema(void) {
    uint32_t entropia = 0;

    // FUENTE 1: Milisegundos del sistema (fácilmente disponible)
    entropia ^= millis();

    // FUENTE 2: Segundos (cambio más lento, mezclado con constante grande)
    entropia ^= (millis() / 1000) * 2654435761U;

    // FUENTE 3: Dirección de la pila (introduce variabilidad de contexto)
    volatile uint8_t stack_marker = 0;
    entropia ^= (uint32_t)(uintptr_t)&stack_marker;

    // FUENTE 4: ID de la tarea actual (depende del planificador)
    entropia ^= ((uint32_t)get_current_task_id()) << 24;

    // FUENTE 5: Contador interno (asegura progresión y mezcla)
    contador_mezclas++;
    entropia ^= contador_mezclas * 1103515245;

    return entropia;
}

/**
 * Función hash MurmurHash3-like para mezclar bits rápidamente.
 * @param x Valor de entrada.
 * @return Valor mezclado.
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

/**
 * Inicializa el generador aleatorio usando múltiples muestras de entropía.
 */
void aleatorio_init(void) {
    // Tomar varias muestras espaciadas por retrasos para aumentar la variabilidad temporal.
    task_delay(5);
    uint32_t entropia1 = obtener_entropia_sistema();
    task_delay(3);
    uint32_t entropia2 = obtener_entropia_sistema();
    task_delay(2);
    uint32_t entropia3 = obtener_entropia_sistema();

    // Mezclar las tres muestras para generar la semilla inicial.
    estado = hash_mezcla(entropia1 ^ entropia2 ^ entropia3);

    // Asegurar que la semilla inicial no sea cero (XORSHIFT no funciona con 0).
    if (estado == 0) {
        estado = 0xDEADBEEF;
    }

    // Descartar los primeros 20 valores (warm-up) para mejorar la aleatoriedad inicial.
    for (uint8_t i = 0; i < 20; i++) {
        aleatorio_obtener();
    }

    // Resetear el contador de mezclas para uso normal.
    contador_mezclas = 0;
}

/**
 * Obtiene el siguiente número pseudoaleatorio de 32 bits usando XORSHIFT32.
 * @return Número aleatorio uint32_t.
 */
uint32_t aleatorio_obtener(void) {
    // Inyectar entropía adicional periódicamente para evitar ciclos cortos.
    if ((contador_mezclas & 0xFF) == 0) {
        estado ^= hash_mezcla(millis());
    }

    // Algoritmo XORSHIFT32 para generar el siguiente estado.
    estado ^= estado << 13;
    estado ^= estado >> 17;
    estado ^= estado << 5;

    contador_mezclas++;
    return estado;
}

/**
 * Obtiene un número aleatorio en un rango específico [min, max] (inclusivo).
 * @param min Valor mínimo (inclusivo).
 * @param max Valor máximo (inclusivo).
 * @return Número entre min y max.
 */
uint32_t aleatorio_rango(uint32_t min, uint32_t max) {
    if (min >= max) return min;

    uint32_t rango = max - min + 1;
    uint32_t rand = aleatorio_obtener();

    // Usar el operador módulo para acotar el valor.
    return min + (rand % rango);
}

/**
 * Obtiene un byte aleatorio (0-255).
 * @return Byte aleatorio.
 */
uint8_t aleatorio_byte(void) {
    // Tomar solo el byte menos significativo.
    return (uint8_t)(aleatorio_obtener() & 0xFF);
}

/**
 * Refresca la semilla con el tiempo actual del sistema y otras fuentes.
 */
void aleatorio_refrescar_semilla(void) {
    uint32_t nueva_entropia = obtener_entropia_sistema();
    // Mezclar (XOR) la nueva entropía con el estado actual.
    estado ^= hash_mezcla(nueva_entropia);
}
