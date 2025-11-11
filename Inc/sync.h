#ifndef SYNC_H
#define SYNC_H

#include <stdint.h>

/* ===== MUTEX SIMPLE (NO-BLOQUEANTE) ===== */
typedef struct {
    volatile int locked;
    volatile int owner_task_id;
    volatile int original_priority;
    volatile int waiting_tasks;
    volatile uint32_t inheritance_count;
} mutex_t;

void mutex_init(mutex_t *m);

/**
 * mutex_lock() - Intenta adquirir el mutex
 *
 * Retorna:
 *   1 = Mutex adquirido exitosamente
 *   0 = Mutex ocupado, la tarea debe terminar y reintentar después
 */
int mutex_lock(mutex_t *m);

void mutex_unlock(mutex_t *m);
int mutex_trylock(mutex_t *m);

// Funciones de estadísticas
uint32_t mutex_get_inheritance_count(mutex_t *m);
void mutex_reset_inheritance_count(mutex_t *m);

/* ===== SEMÁFOROS CON HERENCIA DE PRIORIDAD ===== */
typedef struct {
    volatile int count;
    volatile int max_count;
    volatile int holder_task_id;          // 🔥 Tarea que tiene el recurso
    volatile int original_priority;       // 🔥 Prioridad original del holder
    volatile int waiting_tasks;           // 🔥 Número de tareas esperando
    volatile uint32_t inheritance_count;  // 🔥 Contador de herencias aplicadas
} sem_t;

void sem_init(sem_t *s, int initial, int max);

/**
 * sem_wait() - Intenta decrementar el semáforo (ADQUIRIR RECURSO)
 *
 * COMPORTAMIENTO:
 * 1. Si count > 0:
 *    - Decrementa count
 *    - Si count llega a 0, registra holder_task_id
 *    - Retorna 1 ✅
 *
 * 2. Si count == 0:
 *    - Aplica herencia de prioridad al holder
 *    - Retorna 0 ❌ (tarea debe terminar y reintentar)
 *
 * Retorna:
 *   1 = Recurso adquirido exitosamente
 *   0 = Recurso no disponible, aplicar herencia y reintentar
 */
int sem_wait(sem_t *s);

/**
 * sem_post() - Incrementa el semáforo (LIBERAR RECURSO)
 *
 * COMPORTAMIENTO:
 * 1. Incrementa count
 * 2. Si el holder está liberando (count pasa de 0 a 1):
 *    - Restaura prioridad original
 *    - Limpia holder_task_id
 * 3. Fuerza yield para que tareas esperando puedan ejecutar
 */
void sem_post(sem_t *s);

/**
 * sem_trywait() - Intenta adquirir sin aplicar herencia
 *
 * Retorna:
 *   1 = Recurso adquirido
 *   0 = Recurso no disponible (NO aplica herencia)
 */
int sem_trywait(sem_t *s);

// 🔥 Funciones de estadísticas
uint32_t sem_get_inheritance_count(sem_t *s);
void sem_reset_inheritance_count(sem_t *s);
int sem_get_holder(sem_t *s);  // Retorna task_id del holder (-1 si no hay)
int sem_get_count(sem_t *s);   // Retorna recursos disponibles

#endif // SYNC_H
