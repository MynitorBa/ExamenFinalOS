#ifndef TRON_H // Guarda de inclusión para el juego TRON
#define TRON_H

#include <stdint.h> // Incluye tipos de enteros fijos
#include "sync.h"   // Incluye definiciones de sincronización (mutex_t, sem_t)

/* Inicializar el juego TRON para 4 jugadores */
/** Inicializa todos los componentes y variables del juego. */
void tron_init(void);

/* Reiniciar el juego */
/** Resetea el juego a su estado inicial. */
void tron_reset(void);

/* Limpiar y detener el juego */
/** Libera recursos y detiene las tareas del juego. */
void tron_cleanup(void);

/* Tareas del juego */
/** Función de tarea para manejo de entrada (input). */
void tron_input_task(void);
/** Función de tarea para la lógica principal (movimiento, colisiones). */
void tron_logic_task(void);
/** Función de tarea para el renderizado/dibujo de la pantalla. */
void tron_render_task(void);

/* Tarea de música de fondo */
/** Función de tarea dedicada a la reproducción de la música. */
void tron_music_task(void);

/* Estados del juego */
/** Enumeración de los posibles estados del juego. */
typedef enum {
    TRON_MENU,      /** Estado de menú principal. */
    TRON_RUNNING,   /** El juego está en curso. */
    TRON_PAUSED,    /** El juego está en pausa. */
    TRON_GAME_OVER  /** El juego ha finalizado. */
} tron_state_t;

/* Dirección de movimiento */
/** Enumeración para definir la dirección de movimiento de las motos. */
typedef enum {
    TRON_UP = 0,    /** Mover hacia arriba. */
    TRON_RIGHT = 1, /** Mover hacia la derecha. */
    TRON_DOWN = 2,  /** Mover hacia abajo. */
    TRON_LEFT = 3   /** Mover hacia la izquierda. */
} tron_direction_t;

/* Jugador ganador */
/** Enumeración para identificar al ganador de la partida. */
typedef enum {
    TRON_NO_WINNER = 0,  /** Aún no hay ganador. */
    TRON_PLAYER1_WINS = 1,/** Jugador 1 es el ganador. */
    TRON_PLAYER2_WINS = 2,/** Jugador 2 es el ganador. */
    TRON_PLAYER3_WINS = 3,/** Jugador 3 es el ganador. */
    TRON_PLAYER4_WINS = 4,/** Jugador 4 es el ganador. */
    TRON_DRAW = 5        /** Empate. */
} tron_winner_t;

/* API pública del juego */
/**
 * Obtiene el estado actual del juego.
 * @return Estado actual (tron_state_t).
 */
tron_state_t tron_get_state(void);
/** Obtiene la puntuación/duración de vida del Jugador 1. */
uint32_t tron_get_score_p1(void);
/** Obtiene la puntuación/duración de vida del Jugador 2. */
uint32_t tron_get_score_p2(void);
/** Obtiene la puntuación/duración de vida del Jugador 3. */
uint32_t tron_get_score_p3(void);
/** Obtiene la puntuación/duración de vida del Jugador 4. */
uint32_t tron_get_score_p4(void);
/**
 * Obtiene el ganador de la partida (solo válido en GAME_OVER).
 * @return Ganador (tron_winner_t).
 */
tron_winner_t tron_get_winner(void);

/* Mutexes para protección de recursos compartidos */
/** Mutex para proteger el acceso al estado general del juego. */
extern mutex_t tron_game_state_mutex;
/** Mutex para proteger los datos de los rastros/paredes dejadas. */
extern mutex_t tron_trail_mutex;
/** Mutex para proteger los datos de las motocicletas. */
extern mutex_t tron_bike_mutex;

/* Semáforos para sincronización de tareas */
/** Semáforo que indica que la entrada ha sido procesada. */
extern sem_t tron_input_ready_sem;
/** Semáforo que indica que la lógica ha sido actualizada. */
extern sem_t tron_logic_ready_sem;
/** Semáforo que indica que el renderizado debe comenzar. */
extern sem_t tron_render_ready_sem;

#endif // TRON_H
