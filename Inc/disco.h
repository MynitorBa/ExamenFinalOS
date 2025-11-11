#ifndef DISCO_H
#define DISCO_H

#include <stdint.h>
#include "sync.h"

/* Inicializar el juego DISCO */
void disco_init(void);

/* Limpiar y detener el juego */
void disco_cleanup(void);

/* Tareas del juego */
void disco_input_task(void);
void disco_logic_task(void);
void disco_render_task(void);

/* Estados del juego */
typedef enum {
    DISCO_RUNNING,
    DISCO_GAME_OVER
} disco_state_t;

/* API pública del juego */
disco_state_t disco_get_state(void);
uint16_t disco_get_score_p1(void);
uint16_t disco_get_score_p2(void);

/* Mutexes para protección de recursos compartidos */
extern mutex_t disco_game_state_mutex;
extern mutex_t disco_platform_mutex;
extern mutex_t disco_player_mutex;
extern mutex_t disco_shot_mutex;

/* Semáforos para sincronización de tareas */
extern sem_t disco_input_ready_sem;
extern sem_t disco_logic_ready_sem;
extern sem_t disco_render_ready_sem;

#endif // DISCO_H
