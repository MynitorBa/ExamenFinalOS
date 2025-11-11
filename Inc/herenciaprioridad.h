/**
 * ============================================================================
 * HERENCIAPRIORIDAD.H - Prueba de Herencia de Prioridad Automática
 * ============================================================================
 */

#ifndef HERENCIAPRIORIDAD_H
#define HERENCIAPRIORIDAD_H

#include <stdint.h>

void herenciaprioridad_init(void);
void herenciaprioridad_reset(void);
void herenciaprioridad_cleanup(void);
void herenciaprioridad_input_task(void);
void herenciaprioridad_logic_task(void);
void herenciaprioridad_render_task(void);

// Getters para estadísticas
uint32_t herenciaprioridad_get_count_high(void);
uint32_t herenciaprioridad_get_count_low(void);
uint32_t herenciaprioridad_get_blocked_count(void);
uint32_t herenciaprioridad_get_inheritance_count(void);
void* herenciaprioridad_get_binario(void);

#endif // HERENCIAPRIORIDAD_H
