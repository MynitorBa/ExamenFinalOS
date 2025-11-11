#ifndef SYNC_H // Guarda de inclusión para la librería de sincronización
#define SYNC_H

#include <stdint.h> // Incluye tipos de enteros fijos

/* ===== MUTEX SIMPLE (NO-BLOQUEANTE) ===== */

/** Estructura de Mutex (Exclusión Mutua). */
typedef struct {
    volatile int locked;                 /** Estado de bloqueo: 1 si está bloqueado, 0 si está libre. */
    volatile int owner_task_id;          /** ID de la tarea que posee el mutex. */
    volatile int original_priority;      /** Prioridad original del dueño (para herencia). */
    volatile int waiting_tasks;          /** Número de tareas esperando por el mutex. */
    volatile uint32_t inheritance_count; /** Contador de veces que se aplicó la herencia de prioridad. */
} mutex_t;

/** Inicializa un Mutex, dejándolo desbloqueado. */
void mutex_init(mutex_t *m);

/**
 * Intenta adquirir el Mutex.
 * @return 1 si adquirido, 0 si ocupado (la tarea debe terminar y reintentar).
 */
int mutex_lock(mutex_t *m);

/** Libera el Mutex, restaurando la prioridad del dueño si fue heredada. */
void mutex_unlock(mutex_t *m);

/**
 * Intenta adquirir el Mutex sin esperar ni aplicar herencia.
 * @return 1 si adquirido, 0 si ocupado.
 */
int mutex_trylock(mutex_t *m);

// Funciones de estadísticas
/**
 * Obtiene el contador de herencia de prioridad.
 * @return Contador de herencias.
 */
uint32_t mutex_get_inheritance_count(mutex_t *m);

/** Resetea el contador de herencia de prioridad. */
void mutex_reset_inheritance_count(mutex_t *m);

/* ===== SEMÁFOROS CON HERENCIA DE PRIORIDAD ===== */

/** Estructura de Semáforo de Conteo. */
typedef struct {
    volatile int count;                  /** Conteo actual de recursos disponibles. */
    volatile int max_count;              /** Conteo máximo de recursos. */
    volatile int holder_task_id;          /** Tarea que tiene el recurso (si count == 0). */
    volatile int original_priority;       /** Prioridad original del holder (para herencia). */
    volatile int waiting_tasks;           /** Número de tareas esperando. */
    volatile uint32_t inheritance_count;  /** Contador de herencias aplicadas. */
} sem_t;

/**
 * Inicializa un Semáforo.
 * @param initial Conteo inicial de recursos.
 * @param max Conteo máximo de recursos.
 */
void sem_init(sem_t *s, int initial, int max);

/**
 * Intenta adquirir un recurso del Semáforo. Aplica herencia si es necesario.
 * @return 1 si adquirido, 0 si no disponible (y se aplicó herencia).
 */
int sem_wait(sem_t *s);

/**
 * Libera un recurso del Semáforo. Restaura la prioridad del holder si aplica.
 */
void sem_post(sem_t *s);

/**
 * Intenta adquirir recurso sin aplicar herencia ni bloquear.
 * @return 1 si adquirido, 0 si no disponible.
 */
int sem_trywait(sem_t *s);

// 🔥 Funciones de estadísticas
/**
 * Obtiene el contador de herencia de prioridad del Semáforo.
 * @return Contador de herencias.
 */
uint32_t sem_get_inheritance_count(sem_t *s);

/** Resetea el contador de herencia de prioridad del Semáforo. */
void sem_reset_inheritance_count(sem_t *s);

/**
 * Retorna el ID de la tarea poseedora del recurso (holder).
 * @return ID del holder o -1 si no hay.
 */
int sem_get_holder(sem_t *s);

/**
 * Retorna el número de recursos disponibles.
 * @return Conteo actual.
 */
int sem_get_count(sem_t *s);

#endif // SYNC_H
