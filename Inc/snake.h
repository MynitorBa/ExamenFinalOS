/**
 * ============================================================================
 * SNAKE.H - Juego Snake (Movimiento Continuo por Píxeles)
 * ============================================================================
 */

#ifndef SNAKE_H // Guarda de inclusión para el juego SNAKE
#define SNAKE_H

#include <stdint.h> // Incluye tipos de enteros fijos
#include "sync.h"   // Incluye definiciones de sincronización (mutex_t)
#include "api.h"    // Incluye la API principal del sistema (ej: daos_binario_ejecutable_t)

// Direcciones
/** Enumeración para definir la dirección de movimiento de la serpiente. */
typedef enum {
    SNAKE_UP = 0,    /** Mover hacia arriba. */
    SNAKE_RIGHT = 1, /** Mover hacia la derecha. */
    SNAKE_DOWN = 2,  /** Mover hacia abajo. */
    SNAKE_LEFT = 3   /** Mover hacia la izquierda. */
} snake_direction_t;

// Estados del juego
/** Enumeración de los posibles estados del juego Snake. */
typedef enum {
    SNAKE_RUNNING = 0,   /** El juego está actualmente en curso. */
    SNAKE_GAME_OVER      /** El juego ha finalizado. */
} snake_state_t;

// Mutex para protección de recursos
/** Mutex general para proteger el acceso a las variables compartidas del juego Snake. */
extern mutex_t snake_sync_mutex;

// Funciones del ciclo de vida
/** Inicializa todos los componentes y variables del juego. */
void snake_init(void);
/** Resetea el juego a su estado inicial. */
void snake_reset(void);
/** Libera recursos y realiza la limpieza al finalizar. */
void snake_cleanup(void);

// Tareas
/** Función de tarea para manejo de entrada (input). */
void snake_input_task(void);
/** Función de tarea para la lógica de movimiento y colisiones. */
void snake_logic_task(void);
/** Función de tarea para el renderizado/dibujo de la pantalla. */
void snake_render_task(void);

// API de estadísticas
/**
 * Obtiene el estado actual del juego.
 * @return Estado actual (snake_state_t).
 */
snake_state_t snake_get_state(void);
/**
 * Obtiene la puntuación actual del jugador.
 * @return Puntuación.
 */
uint32_t snake_get_score(void);

// Función de binario para el loader de DaOS
/**
 * Obtiene la estructura binaria del juego para ser cargada por el loader.
 * @return Puntero a la estructura daos_binario_ejecutable_t.
 */
daos_binario_ejecutable_t* snake_get_binario(void);

#endif // SNAKE_H
