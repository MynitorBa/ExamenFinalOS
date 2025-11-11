#include "sync.h"
#include "sched.h"

/* ===== FUNCIONES AUXILIARES ===== */

static inline void enter_critical(void) {
    __asm volatile("cpsid i" ::: "memory");
}

static inline void exit_critical(void) {
    __asm volatile("cpsie i" ::: "memory");
}

/* ===== MUTEX (YA EXISTENTE) ===== */

void mutex_init(mutex_t *m) {
    m->locked = 0;
    m->owner_task_id = -1;
    m->original_priority = -1;
    m->waiting_tasks = 0;
    m->inheritance_count = 0;
}

int mutex_lock(mutex_t *m) {
    int current_task = get_current_task_id();

    enter_critical();

    if (!m->locked) {
        m->locked = 1;
        m->owner_task_id = current_task;
        m->original_priority = get_task_priority(current_task);
        m->waiting_tasks = 0;
        exit_critical();
        return 1;
    }

    TaskPriority my_priority = get_task_priority(current_task);
    TaskPriority owner_priority = get_task_priority(m->owner_task_id);

    if (my_priority > owner_priority) {
        task_set_priority(m->owner_task_id, my_priority);
        m->inheritance_count++;
    }

    m->waiting_tasks++;
    exit_critical();
    return 0;
}

void mutex_unlock(mutex_t *m) {
    int current_task = get_current_task_id();

    enter_critical();

    if (m->owner_task_id != current_task) {
        exit_critical();
        return;
    }

    TaskPriority restore_priority = (TaskPriority)m->original_priority;

    m->locked = 0;
    m->owner_task_id = -1;
    m->original_priority = -1;
    m->waiting_tasks = 0;

    if (restore_priority >= PRIO_IDLE && restore_priority <= PRIO_CRITICAL) {
        task_set_priority(current_task, restore_priority);
        __asm volatile("dsb" ::: "memory");
        __asm volatile("isb" ::: "memory");
    }

    exit_critical();
    task_yield();
}

int mutex_trylock(mutex_t *m) {
    enter_critical();

    if (m->locked) {
        exit_critical();
        return 0;
    }

    int current_task = get_current_task_id();
    m->locked = 1;
    m->owner_task_id = current_task;
    m->original_priority = get_task_priority(current_task);
    m->waiting_tasks = 0;

    exit_critical();
    return 1;
}

uint32_t mutex_get_inheritance_count(mutex_t *m) {
    return m->inheritance_count;
}

void mutex_reset_inheritance_count(mutex_t *m) {
    m->inheritance_count = 0;
}

/* ===== SEMÁFOROS CON HERENCIA DE PRIORIDAD ===== */

/**
 * INICIALIZACIÓN DEL SEMÁFORO
 */
void sem_init(sem_t *s, int initial, int max) {
    s->count = initial;
    s->max_count = max;
    s->holder_task_id = -1;
    s->original_priority = -1;
    s->waiting_tasks = 0;
    s->inheritance_count = 0;
}

/**
 * SEM_WAIT CON HERENCIA DE PRIORIDAD
 *
 * Comportamiento similar a mutex_lock():
 * 1. Si hay recursos disponibles (count > 0):
 *    - Decrementa count
 *    - Si count llega a 0, registra como holder
 *    - Retorna 1
 *
 * 2. Si NO hay recursos (count == 0):
 *    - Aplica herencia de prioridad al holder actual
 *    - Retorna 0 (tarea debe terminar)
 */
int sem_wait(sem_t *s) {
    int current_task = get_current_task_id();

    enter_critical();

    // CASO 1: Hay recursos disponibles
    if (s->count > 0) {
        s->count--;

        // Si llegamos a 0, esta tarea es ahora el holder exclusivo
        if (s->count == 0) {
            s->holder_task_id = current_task;
            s->original_priority = get_task_priority(current_task);
            s->waiting_tasks = 0;
        }

        exit_critical();
        return 1;  // Recurso adquirido
    }

    // CASO 2: NO hay recursos (count == 0)
    // Aplicar herencia de prioridad al holder actual
    if (s->holder_task_id >= 0) {
        TaskPriority my_priority = get_task_priority(current_task);
        TaskPriority holder_priority = get_task_priority(s->holder_task_id);

        if (my_priority > holder_priority) {
            // Elevar prioridad del holder para que libere pronto
            task_set_priority(s->holder_task_id, my_priority);
            s->inheritance_count++;
        }

        s->waiting_tasks++;
    }

    exit_critical();
    return 0;  // No se pudo adquirir, tarea debe terminar
}

/**
 * SEM_POST CON RESTAURACIÓN DE PRIORIDAD
 *
 * Comportamiento similar a mutex_unlock():
 * 1. Incrementa count
 * 2. Si el holder está liberando (count pasa de 0 a 1):
 *    - Restaura prioridad original
 *    - Limpia holder_task_id
 * 3. Fuerza yield para rescheduling
 */
void sem_post(sem_t *s) {
    int current_task = get_current_task_id();

    enter_critical();

    // Verificar si esta tarea es el holder que debe restaurar prioridad
    int was_holder = (s->holder_task_id == current_task && s->count == 0);

    // Incrementar count (respetar max_count si está definido)
    if (s->max_count == 0 || s->count < s->max_count) {
        s->count++;
    }

    // RESTAURAR PRIORIDAD si estamos liberando el último recurso
    if (was_holder) {
        TaskPriority restore_priority = (TaskPriority)s->original_priority;

        // Limpiar estado del holder
        s->holder_task_id = -1;
        s->original_priority = -1;
        s->waiting_tasks = 0;

        // Restaurar prioridad dentro de critical section
        if (restore_priority >= PRIO_IDLE && restore_priority <= PRIO_CRITICAL) {
            task_set_priority(current_task, restore_priority);
            __asm volatile("dsb" ::: "memory");
            __asm volatile("isb" ::: "memory");
        }
    }

    exit_critical();

    // Forzar rescheduling para que tareas esperando puedan ejecutar
    task_yield();
}

/**
 * SEM_TRYWAIT SIN HERENCIA
 *
 * Intento simple sin aplicar herencia de prioridad
 */
int sem_trywait(sem_t *s) {
    int current_task = get_current_task_id();

    enter_critical();

    if (s->count <= 0) {
        exit_critical();
        return 0;  // No disponible
    }

    s->count--;

    // Si llegamos a 0, registrar como holder
    if (s->count == 0) {
        s->holder_task_id = current_task;
        s->original_priority = get_task_priority(current_task);
        s->waiting_tasks = 0;
    }

    exit_critical();
    return 1;  // Recurso adquirido
}

/* ===== FUNCIONES DE ESTADÍSTICAS ===== */

uint32_t sem_get_inheritance_count(sem_t *s) {
    return s->inheritance_count;
}

void sem_reset_inheritance_count(sem_t *s) {
    s->inheritance_count = 0;
}

int sem_get_holder(sem_t *s) {
    return s->holder_task_id;
}
