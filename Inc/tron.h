#ifndef TRON_H
#define TRON_H

#include <stdint.h>
#include "sync.h"

/* Inicializar el juego TRON para 4 jugadores */
void tron_init(void);

/* Reiniciar el juego */
void tron_reset(void);

/* Limpiar y detener el juego */
void tron_cleanup(void);

/* Tareas del juego */
void tron_input_task(void);
void tron_logic_task(void);
void tron_render_task(void);

/* Tarea de música de fondo */
void tron_music_task(void);

/* Estados del juego */
typedef enum {
    TRON_MENU,
    TRON_RUNNING,
    TRON_PAUSED,
    TRON_GAME_OVER
} tron_state_t;

/* Dirección de movimiento */
typedef enum {
    TRON_UP = 0,
    TRON_RIGHT = 1,
    TRON_DOWN = 2,
    TRON_LEFT = 3
} tron_direction_t;

/* Jugador ganador */
typedef enum {
    TRON_NO_WINNER = 0,
    TRON_PLAYER1_WINS = 1,
    TRON_PLAYER2_WINS = 2,
    TRON_PLAYER3_WINS = 3,
    TRON_PLAYER4_WINS = 4,
    TRON_DRAW = 5
} tron_winner_t;

/* API pública del juego */
tron_state_t tron_get_state(void);
uint32_t tron_get_score_p1(void);
uint32_t tron_get_score_p2(void);
uint32_t tron_get_score_p3(void);
uint32_t tron_get_score_p4(void);
tron_winner_t tron_get_winner(void);

/* Mutexes para protección de recursos compartidos */
extern mutex_t tron_game_state_mutex;
extern mutex_t tron_trail_mutex;
extern mutex_t tron_bike_mutex;

/* Semáforos para sincronización de tareas */
extern sem_t tron_input_ready_sem;
extern sem_t tron_logic_ready_sem;
extern sem_t tron_render_ready_sem;

#endif // TRON_H
