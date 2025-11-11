#ifndef TANQUE_H // Guarda de inclusión para el juego TANQUE
#define TANQUE_H

#include <stdint.h> // Incluye tipos de enteros fijos
#include "sync.h"   // Incluye definiciones de sincronización (mutex_t, sem_t)

/* Inicializar el juego TANQUE */
/** Inicializa todos los componentes y variables del juego. */
void tanque_init(void);

/* Limpiar y detener el juego */
/** Libera recursos y detiene las tareas del juego. */
void tanque_cleanup(void);

/* Tareas del juego */
/** Función de tarea para manejo de entrada (input). */
void tanque_input_task(void);
/** Función de tarea para la lógica principal del juego. */
void tanque_logic_task(void);
/** Función de tarea para el renderizado/dibujo de la pantalla. */
void tanque_render_task(void);

/* Regenerar mapa con semilla aleatoria */
/** Genera un nuevo mapa de juego con elementos aleatorios. */
void tanque_regenerar_mapa(void);

/* Estados del juego */
/** Enumeración de los posibles estados del juego. */
typedef enum {
    TANQUE_RUNNING,   /** El juego está en curso. */
    TANQUE_GAME_OVER  /** El juego ha finalizado. */
} tanque_state_t;

/* Ganador */
/** Enumeración para identificar al ganador de la partida. */
typedef enum {
    TANQUE_WINNER_NONE = 0, /** No hay ganador aún. */
    TANQUE_WINNER_P1_AZUL,  /** Ganador es el Jugador 1 (Azul). */
    TANQUE_WINNER_P2_ROJO,  /** Ganador es el Jugador 2 (Rojo). */
    TANQUE_WINNER_DRAW      /** Empate. */
} tanque_winner_t;

/* API pública del juego */
/**
 * Obtiene el estado actual del juego.
 * @return Estado actual (tanque_state_t).
 */
tanque_state_t tanque_get_state(void);
/**
 * Obtiene el ganador de la partida (solo válido en estado GAME_OVER).
 * @return Ganador (tanque_winner_t).
 */
tanque_winner_t tanque_get_winner(void);

/* Mutexes para protección de recursos compartidos */
/** Mutex para proteger el acceso al estado general del juego. */
extern mutex_t tanque_game_state_mutex;
/** Mutex para proteger los datos del mapa/terreno. */
extern mutex_t tanque_map_mutex;
/** Mutex para proteger los datos de los tanques. */
extern mutex_t tanque_tank_mutex;
/** Mutex para proteger los datos de los proyectiles. */
extern mutex_t tanque_projectile_mutex;

/* Semáforos para sincronización de tareas */
/** Semáforo que indica que la entrada ha sido procesada. */
extern sem_t tanque_input_ready_sem;
/** Semáforo que indica que la lógica ha sido actualizada. */
extern sem_t tanque_logic_ready_sem;
/** Semáforo que indica que el renderizado debe comenzar. */
extern sem_t tanque_render_ready_sem;

#endif // TANQUE_H
