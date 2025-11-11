/**
 * ============================================================================
 * DEMO_PREM.H - Demostración de Planificación Preemptiva
 * ============================================================================
 */

#ifndef DEMO_PREM_H // Guarda de inclusión
#define DEMO_PREM_H

#include <stdint.h> // Incluye tipos de enteros fijos

// Funciones principales del demo
/** Inicializa el demo de planificación. */
void demo_prem_init(void);
/** Resetea los contadores y estados del demo. */
void demo_prem_reset(void);
/** Realiza la limpieza de recursos. */
void demo_prem_cleanup(void);

// Funciones de tareas (compatibilidad con binario_ejecutable)
/** Función de tarea para manejo de entrada (input). */
void demo_prem_input(void);
/** Función de tarea para la lógica del demo. */
void demo_prem_logic(void);
/** Función de tarea para el renderizado/dibujo. */
void demo_prem_render(void);

// Getters para estadísticas
/**
 * Obtiene el contador de una tarea (A).
 * @return Contador.
 */
uint32_t demo_prem_get_count_a(void);

/**
 * Obtiene el contador de otra tarea (B).
 * @return Contador.
 */
uint32_t demo_prem_get_count_b(void);

/**
 * Obtiene el ID de la tarea que está activa.
 * @return ID de la tarea activa.
 */
uint8_t demo_prem_get_active_task(void);

/**
 * Obtiene el número total de cambios de contexto (context switches).
 * @return Contador de switches.
 */
uint32_t demo_prem_get_context_switches(void);

/**
 * Obtiene el tiempo de CPU acumulado por la tarea A.
 * @return Tiempo de CPU en unidades de tiempo.
 */
uint32_t demo_prem_get_cpu_time_a(void);

/**
 * Obtiene el tiempo de CPU acumulado por la tarea B.
 * @return Tiempo de CPU en unidades de tiempo.
 */
uint32_t demo_prem_get_cpu_time_b(void);

#endif // DEMO_PREM_H
