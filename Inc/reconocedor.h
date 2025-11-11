#ifndef RECONOCEDOR_H
#define RECONOCEDOR_H

#include "api.h"
#include "sync.h"

/**
 * Regenera el mapa con semilla aleatoria basada en tiempo real
 */
void reconocedor_regenerar_mapa(void);

/**
 * Inicializa el juego
 */
void reconocedor_init(void);

/**
 * Reinicia el juego
 */
void reconocedor_reset(void);

/**
 * Limpia y detiene el juego
 */
void reconocedor_cleanup(void);

/**
 * Tareas del juego
 */
void reconocedor_input_task(void);
void reconocedor_logic_task(void);
void reconocedor_render_task(void);

/* Mutexes para protección de recursos compartidos */
extern mutex_t reconocedor_game_state_mutex;
extern mutex_t reconocedor_map_mutex;
extern mutex_t reconocedor_player_mutex;
extern mutex_t reconocedor_maniqui_mutex;

/* Semáforos para sincronización de tareas */
extern sem_t reconocedor_input_ready_sem;
extern sem_t reconocedor_logic_ready_sem;
extern sem_t reconocedor_render_ready_sem;

#endif // RECONOCEDOR_H
