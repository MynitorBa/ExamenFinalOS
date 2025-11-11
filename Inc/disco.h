#ifndef DISCO_H // Guarda de inclusión para el juego DISCO
#define DISCO_H

#include <stdint.h> // Incluye tipos de enteros fijos
#include "sync.h"   // Incluye definiciones de sincronización (mutex_t, sem_t)

/* Inicializar el juego DISCO */
/** Inicializa todos los componentes y variables del juego. */
void disco_init(void);

/* Limpiar y detener el juego */
/** Libera recursos y detiene las tareas del juego. */
void disco_cleanup(void);

/* Tareas del juego */
/** Función de tarea para manejo de entrada (input). */
void disco_input_task(void);
/** Función de tarea para la lógica principal del juego. */
void disco_logic_task(void);
/** Función de tarea para el renderizado/dibujo de la pantalla. */
void disco_render_task(void);

/* Estados del juego */
/** Enumeración de los posibles estados del juego DISCO. */
typedef enum {
    DISCO_RUNNING,   /** El juego está actualmente en curso. */
    DISCO_GAME_OVER  /** El juego ha finalizado. */
} disco_state_t;

/* API pública del juego */
/**
 * Obtiene el estado actual del juego.
 * @return Estado actual (disco_state_t).
 */
disco_state_t disco_get_state(void);
/**
 * Obtiene la puntuación del jugador 1.
 * @return Puntuación.
 */
uint16_t disco_get_score_p1(void);
/**
 * Obtiene la puntuación del jugador 2.
 * @return Puntuación.
 */
uint16_t disco_get_score_p2(void);

/* Mutexes para protección de recursos compartidos */
/** Mutex para proteger el acceso al estado general del juego. */
extern mutex_t disco_game_state_mutex;
/** Mutex para proteger los datos de la plataforma/tablero. */
extern mutex_t disco_platform_mutex;
/** Mutex para proteger los datos de los jugadores. */
extern mutex_t disco_player_mutex;
/** Mutex para proteger los datos de los disparos/proyectiles. */
extern mutex_t disco_shot_mutex;

/* Semáforos para sincronización de tareas */
/** Semáforo que indica que la entrada ha sido procesada. */
extern sem_t disco_input_ready_sem;
/** Semáforo que indica que la lógica ha sido actualizada. */
extern sem_t disco_logic_ready_sem;
/** Semáforo que indica que el renderizado debe comenzar. */
extern sem_t disco_render_ready_sem;

#endif // DISCO_H
