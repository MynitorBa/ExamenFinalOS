#ifndef RECONOCEDOR_H // Guarda de inclusión para el juego RECONOCEDOR
#define RECONOCEDOR_H

#include "api.h" // Incluye la API principal del sistema
#include "sync.h" // Incluye definiciones de sincronización (mutex_t, sem_t)

/**
 * Regenera el mapa con semilla aleatoria basada en tiempo real.
 */
void reconocedor_regenerar_mapa(void);

/**
 * Inicializa el juego.
 */
void reconocedor_init(void);

/**
 * Reinicia el juego.
 */
void reconocedor_reset(void);

/**
 * Limpia y detiene el juego.
 */
void reconocedor_cleanup(void);

/**
 * Tareas del juego
 */
/** Función de tarea para manejo de entrada (input). */
void reconocedor_input_task(void);
/** Función de tarea para la lógica principal del juego. */
void reconocedor_logic_task(void);
/** Función de tarea para el renderizado/dibujo de la pantalla. */
void reconocedor_render_task(void);

/* Mutexes para protección de recursos compartidos */
/** Mutex para proteger el acceso al estado general del juego. */
extern mutex_t reconocedor_game_state_mutex;
/** Mutex para proteger los datos del mapa/tablero. */
extern mutex_t reconocedor_map_mutex;
/** Mutex para proteger los datos del jugador principal. */
extern mutex_t reconocedor_player_mutex;
/** Mutex para proteger los datos del enemigo/maniquí. */
extern mutex_t reconocedor_maniqui_mutex;

/* Semáforos para sincronización de tareas */
/** Semáforo que indica que la entrada ha sido procesada. */
extern sem_t reconocedor_input_ready_sem;
/** Semáforo que indica que la lógica ha sido actualizada. */
extern sem_t reconocedor_logic_ready_sem;
/** Semáforo que indica que el renderizado debe comenzar. */
extern sem_t reconocedor_render_ready_sem;

#endif // RECONOCEDOR_H
