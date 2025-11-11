#ifndef SCHED_H // Guarda de inclusión para el planificador
#define SCHED_H

#include <stdint.h> // Incluye tipos de enteros fijos

/** Enumeración para definir los niveles de prioridad de las tareas. */
typedef enum {
    PRIO_IDLE = 0,     /** Prioridad de inactividad. */
    PRIO_LOW = 1,      /** Prioridad baja. */
    PRIO_NORMAL = 2,   /** Prioridad normal (por defecto). */
    PRIO_HIGH = 3,     /** Prioridad alta. */
    PRIO_CRITICAL = 4  /** Prioridad crítica. */
} TaskPriority;

/** Enumeración para definir el estado de ejecución de una tarea. */
typedef enum {
    TASK_READY = 0,    /** La tarea está lista para ejecutarse. */
    TASK_BLOCKED,      /** La tarea está esperando un evento (ej: tiempo, recurso). */
    TASK_SUSPENDED     /** La tarea está suspendida manualmente. */
} TaskState;

/**
 * Crea una nueva tarea en el planificador.
 * @param func Puntero a la función de entrada (entry point) de la tarea.
 * @param priority Prioridad inicial de la tarea.
 */
void task_create(void (*func)(void), TaskPriority priority);

/**
 * Inicia el planificador de tareas (no retorna).
 */
void sched_start(void);

/**
 * Bloquea la tarea actual por un número de milisegundos.
 * @param ms Milisegundos a esperar.
 */
void task_delay(uint32_t ms);

/**
 * Cede el control de la CPU (yield) a otra tarea lista.
 */
void task_yield(void);

/**
 * Establece la prioridad de una tarea específica.
 * @param task_id ID de la tarea.
 * @param priority Nueva prioridad.
 */
void task_set_priority(uint8_t task_id, TaskPriority priority);

/**
 * Obtiene el ID de la tarea actualmente en ejecución.
 * @return ID de la tarea.
 */
uint8_t get_current_task_id(void);

/**
 * Obtiene la prioridad de una tarea por su ID.
 * @param task_id ID de la tarea.
 * @return Prioridad de la tarea.
 */
TaskPriority get_task_priority(uint8_t task_id);

/**
 * Obtiene el tiempo transcurrido del sistema en milisegundos.
 * @return Tiempo en ms.
 */
uint32_t millis(void);

/**
 * Obtiene el número de veces que una tarea no pudo ejecutarse a tiempo.
 * @param task_id ID de la tarea.
 * @return Contador de sobrecargas (overruns).
 */
uint32_t task_get_overruns(uint8_t task_id);

/**
 * Termina la tarea actual.
 */
void task_exit(void);

/**
 * Obtiene el número total de tareas en el sistema.
 * @return Conteo de tareas.
 */
uint8_t task_get_count(void);

/**
 * Detiene y elimina todas las tareas del sistema.
 */
void sched_kill_all_tasks(void);

/** Estructura con información de una tarea. */
typedef struct {
    uint8_t id;
    TaskState state;
    TaskPriority priority;
    uint32_t cpu_time; /** Tiempo de CPU consumido. */
} TaskInfo;

/**
 * Rellena una lista con información de las tareas.
 * @param list Puntero al buffer de estructuras.
 * @param max_tasks Tamaño máximo del buffer.
 * @return Número de tareas listadas.
 */
int get_task_list(TaskInfo* list, int max_tasks);

/** Macro alias para `task_delay`. */
#define sleep_ms(x) task_delay(x)

#endif // SCHED_H
