/**
 * ============================================================================
 * DEMO_SCHEDULER_RR.H - Round-Robin Demo Simplificado
 * ============================================================================
 */

#ifndef DEMO_SCHEDULER_RR_H
#define DEMO_SCHEDULER_RR_H

#include <stdint.h>

// Funciones del ciclo de vida
void demo_scheduler_rr_init(void);
void demo_scheduler_rr_reset(void);
void demo_scheduler_rr_cleanup(void);

// Tareas
void demo_scheduler_rr_input(void);
void demo_scheduler_rr_logic(void);
void demo_scheduler_rr_render(void);

// API de estadísticas
uint32_t demo_rr_get_count_a(void);
uint32_t demo_rr_get_count_b(void);
uint8_t demo_rr_get_active_task(void);

#endif // DEMO_SCHEDULER_RR_H
