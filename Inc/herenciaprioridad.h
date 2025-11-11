/**
 * ============================================================================
 * HERENCIAPRIORIDAD.H - Prueba de Herencia de Prioridad Automática
 * ============================================================================
 */

#ifndef HERENCIAPRIORIDAD_H // Guarda de inclusión
#define HERENCIAPRIORIDAD_H

#include <stdint.h> // Incluye tipos de enteros fijos

/** Inicializa la demostración de Herencia de Prioridad. */
void herenciaprioridad_init(void);
/** Resetea los contadores y estados de la demostración. */
void herenciaprioridad_reset(void);
/** Libera los recursos usados por la demostración. */
void herenciaprioridad_cleanup(void);

/** Función de tarea para manejo de entrada (input). */
void herenciaprioridad_input_task(void);
/** Función de tarea para la lógica de la prueba. */
void herenciaprioridad_logic_task(void);
/** Función de tarea para el renderizado/dibujo. */
void herenciaprioridad_render_task(void);

// Getters para estadísticas
/**
 * Obtiene el contador de la tarea de alta prioridad.
 * @return Contador.
 */
uint32_t herenciaprioridad_get_count_high(void);

/**
 * Obtiene el contador de la tarea de baja prioridad.
 * @return Contador.
 */
uint32_t herenciaprioridad_get_count_low(void);

/**
 * Obtiene el número de veces que la tarea de alta prioridad fue bloqueada.
 * @return Contador de bloqueos.
 */
uint32_t herenciaprioridad_get_blocked_count(void);

/**
 * Obtiene el número de veces que se aplicó la herencia de prioridad.
 * @return Contador de herencias.
 */
uint32_t herenciaprioridad_get_inheritance_count(void);

/**
 * Obtiene el puntero a la estructura binaria del demo.
 * @return Puntero a la estructura binaria.
 */
void* herenciaprioridad_get_binario(void);

#endif // HERENCIAPRIORIDAD_H
