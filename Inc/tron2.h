#ifndef TRON2_H
#define TRON2_H

#include <stdint.h>
#include "sync.h"

/* Inicializar el juego TRON para 2 jugadores */
void tron2_init(void);

/* Reiniciar el juego */
void tron2_reset(void);

/* Limpiar y detener el juego */
void tron2_cleanup(void);

/* Tareas del juego */
void tron2_input_task(void);
void tron2_logic_task(void);
void tron2_render_task(void);

/* Tarea de música de fondo */
void tron2_music_task(void);

/* Estados del juego */
typedef enum {
    TRON2_MENU,
    TRON2_RUNNING,
    TRON2_PAUSED,
    TRON2_GAME_OVER
} tron2_state_t;

/* Dirección de movimiento */
typedef enum {
    TRON2_UP = 0,
    TRON2_RIGHT = 1,
    TRON2_DOWN = 2,
    TRON2_LEFT = 3
} tron2_direction_t;

/* Jugador ganador */
typedef enum {
    TRON2_NO_WINNER = 0,
    TRON2_PLAYER1_WINS = 1,
    TRON2_PLAYER2_WINS = 2,
    TRON2_DRAW = 3
} tron2_winner_t;

/* API pública del juego */
tron2_state_t tron2_get_state(void);
uint32_t tron2_get_score_p1(void);
uint32_t tron2_get_score_p2(void);
tron2_winner_t tron2_get_winner(void);

/* Mutexes para protección de recursos compartidos */
extern mutex_t tron2_game_state_mutex;
extern mutex_t tron2_trail_mutex;
extern mutex_t tron2_bike_mutex;

/* Semáforos para sincronización de tareas */
extern sem_t tron2_input_ready_sem;
extern sem_t tron2_logic_ready_sem;
extern sem_t tron2_render_ready_sem;

#endif // TRON2_H
