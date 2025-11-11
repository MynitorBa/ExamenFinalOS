/**
 * ============================================================================
 * DEMO_SCHEDULER_RR.H - Round-Robin Demo Simplificado
 * ============================================================================
 */

#ifndef DEMO_SCHEDULER_RR_H // Guarda de inclusión
#define DEMO_SCHEDULER_RR_H

#include <stdint.h> // Incluye tipos de enteros fijos

// Funciones del ciclo de vida
/** Inicializa el demo de Round-Robin. */
void demo_scheduler_rr_init(void);
/** Resetea los contadores y estados. */
void demo_scheduler_rr_reset(void);
/** Realiza la limpieza de recursos. */
void demo_scheduler_rr_cleanup(void);

// Tareas (compatibilidad con binario_ejecutable)
/** Función de tarea para manejo de entrada (input). */
void demo_scheduler_rr_input(void);
/** Función de tarea para la lógica del demo. */
void demo_scheduler_rr_logic(void);
/** Función de tarea para el renderizado/dibujo. */
void demo_scheduler_rr_render(void);

// API de estadísticas
/**
 * Obtiene el contador de una tarea (A).
 * @return Contador.
 */
uint32_t demo_rr_get_count_a(void);

/**
 * Obtiene el contador de otra tarea (B).
 * @return Contador.
 */
uint32_t demo_rr_get_count_b(void);

/**
 * Obtiene el ID de la tarea que está activa.
 * @return ID de la tarea activa.
 */
uint8_t demo_rr_get_active_task(void);

#endif // DEMO_SCHEDULER_RR_H
