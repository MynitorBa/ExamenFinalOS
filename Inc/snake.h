/**
 * ============================================================================
 * SNAKE.H - Juego Snake (Movimiento Continuo por Píxeles)
 * ============================================================================
 */

#ifndef SNAKE_H
#define SNAKE_H

#include <stdint.h>
#include "sync.h"
#include "api.h"

// Direcciones
typedef enum {
    SNAKE_UP = 0,
    SNAKE_RIGHT = 1,
    SNAKE_DOWN = 2,
    SNAKE_LEFT = 3
} snake_direction_t;

// Estados del juego
typedef enum {
    SNAKE_RUNNING = 0,
    SNAKE_GAME_OVER
} snake_state_t;

// Mutex para protección de recursos
extern mutex_t snake_sync_mutex;

// Funciones del ciclo de vida
void snake_init(void);
void snake_reset(void);
void snake_cleanup(void);

// Tareas
void snake_input_task(void);
void snake_logic_task(void);
void snake_render_task(void);

// API de estadísticas
snake_state_t snake_get_state(void);
uint32_t snake_get_score(void);

// Función de binario para el loader de DaOS
daos_binario_ejecutable_t* snake_get_binario(void);

#endif // SNAKE_H
