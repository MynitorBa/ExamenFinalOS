#ifndef TANQUE_H
#define TANQUE_H

#include <stdint.h>
#include "sync.h"

/* Inicializar el juego TANQUE */
void tanque_init(void);

/* Limpiar y detener el juego */
void tanque_cleanup(void);

/* Tareas del juego */
void tanque_input_task(void);
void tanque_logic_task(void);
void tanque_render_task(void);

/* Regenerar mapa con semilla aleatoria */
void tanque_regenerar_mapa(void);

/* Estados del juego */
typedef enum {
    TANQUE_RUNNING,
    TANQUE_GAME_OVER
} tanque_state_t;

/* Ganador */
typedef enum {
    TANQUE_WINNER_NONE = 0,
    TANQUE_WINNER_P1_AZUL,
    TANQUE_WINNER_P2_ROJO,
    TANQUE_WINNER_DRAW
} tanque_winner_t;

/* API pública del juego */
tanque_state_t tanque_get_state(void);
tanque_winner_t tanque_get_winner(void);

/* Mutexes para protección de recursos compartidos */
extern mutex_t tanque_game_state_mutex;
extern mutex_t tanque_map_mutex;
extern mutex_t tanque_tank_mutex;
extern mutex_t tanque_projectile_mutex;

/* Semáforos para sincronización de tareas */
extern sem_t tanque_input_ready_sem;
extern sem_t tanque_logic_ready_sem;
extern sem_t tanque_render_ready_sem;

#endif // TANQUE_H
