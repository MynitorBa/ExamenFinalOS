/**
 * ============================================================================
 * DEMO_PREM.H - Demostración de Planificación Preemptiva
 * ============================================================================
 */

#ifndef DEMO_PREM_H
#define DEMO_PREM_H

#include <stdint.h>

// Funciones principales del demo
void demo_prem_init(void);
void demo_prem_reset(void);
void demo_prem_cleanup(void);

// Funciones de tareas (compatibilidad con binario_ejecutable)
void demo_prem_input(void);
void demo_prem_logic(void);
void demo_prem_render(void);

// Getters para estadísticas
uint32_t demo_prem_get_count_a(void);
uint32_t demo_prem_get_count_b(void);
uint8_t demo_prem_get_active_task(void);
uint32_t demo_prem_get_context_switches(void);
uint32_t demo_prem_get_cpu_time_a(void);
uint32_t demo_prem_get_cpu_time_b(void);

#endif // DEMO_PREM_H
