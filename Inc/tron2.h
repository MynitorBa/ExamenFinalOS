#ifndef TRON2_H // Guarda de inclusión para el juego TRON2
#define TRON2_H

#include <stdint.h> // Incluye tipos de enteros fijos
#include "sync.h"   // Incluye definiciones de sincronización (mutex_t, sem_t)

/* Inicializar el juego TRON para 2 jugadores */
/** Inicializa todos los componentes y variables del juego. */
void tron2_init(void);

/* Reiniciar el juego */
/** Resetea el juego a su estado inicial. */
void tron2_reset(void);

/* Limpiar y detener el juego */
/** Libera recursos y detiene las tareas del juego. */
void tron2_cleanup(void);

/* Tareas del juego */
/** Función de tarea para manejo de entrada (input). */
void tron2_input_task(void);
/** Función de tarea para la lógica principal (movimiento, colisiones). */
void tron2_logic_task(void);
/** Función de tarea para el renderizado/dibujo de la pantalla. */
void tron2_render_task(void);

/* Tarea de música de fondo */
/** Función de tarea dedicada a la reproducción de la música. */
void tron2_music_task(void);

/* Estados del juego */
/** Enumeración de los posibles estados del juego. */
typedef enum {
    TRON2_MENU,      /** Estado de menú principal. */
    TRON2_RUNNING,   /** El juego está en curso. */
    TRON2_PAUSED,    /** El juego está en pausa. */
    TRON2_GAME_OVER  /** El juego ha finalizado. */
} tron2_state_t;

/* Dirección de movimiento */
/** Enumeración para definir la dirección de movimiento de las motos. */
typedef enum {
    TRON2_UP = 0,    /** Mover hacia arriba. */
    TRON2_RIGHT = 1, /** Mover hacia la derecha. */
    TRON2_DOWN = 2,  /** Mover hacia abajo. */
    TRON2_LEFT = 3   /** Mover hacia la izquierda. */
} tron2_direction_t;

/* Jugador ganador */
/** Enumeración para identificar al ganador de la partida. */
typedef enum {
    TRON2_NO_WINNER = 0,  /** Aún no hay ganador. */
    TRON2_PLAYER1_WINS = 1,/** Jugador 1 es el ganador. */
    TRON2_PLAYER2_WINS = 2,/** Jugador 2 es el ganador. */
    TRON2_DRAW = 3        /** Empate. */
} tron2_winner_t;

/* API pública del juego */
/**
 * Obtiene el estado actual del juego.
 * @return Estado actual (tron2_state_t).
 */
tron2_state_t tron2_get_state(void);
/** Obtiene la puntuación/duración de vida del Jugador 1. */
uint32_t tron2_get_score_p1(void);
/** Obtiene la puntuación/duración de vida del Jugador 2. */
uint32_t tron2_get_score_p2(void);
/**
 * Obtiene el ganador de la partida (solo válido en GAME_OVER).
 * @return Ganador (tron2_winner_t).
 */
tron2_winner_t tron2_get_winner(void);

/* Mutexes para protección de recursos compartidos */
/** Mutex para proteger el acceso al estado general del juego. */
extern mutex_t tron2_game_state_mutex;
/** Mutex para proteger los datos de los rastros/paredes dejadas. */
extern mutex_t tron2_trail_mutex;
/** Mutex para proteger los datos de las motocicletas. */
extern mutex_t tron2_bike_mutex;

/* Semáforos para sincronización de tareas */
/** Semáforo que indica que la entrada ha sido procesada. */
extern sem_t tron2_input_ready_sem;
/** Semáforo que indica que la lógica ha sido actualizada. */
extern sem_t tron2_logic_ready_sem;
/** Semáforo que indica que el renderizado debe comenzar. */
extern sem_t tron2_render_ready_sem;

#endif // TRON2_H
