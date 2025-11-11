#include "sched.h"

#define MAX_TASKS 16
#define QUANTUM_MS 1
#define AGING_THRESHOLD 50
#define MAX_QUANTUM_SLOTS 3

// Quantum variable según prioridad
#define QUANTUM_CRITICAL 5
#define QUANTUM_HIGH 3
#define QUANTUM_NORMAL 2
#define QUANTUM_LOW 1
#define QUANTUM_IDLE 1

// Límites de ejecuciones consecutivas
#define MAX_LOW_CONSECUTIVE 2
#define MAX_NORMAL_CONSECUTIVE 4

typedef struct {
    void (*func)(void);
    uint32_t next_wake;
    uint32_t last_wake;
    uint32_t overrun_count;
    TaskState state;
    TaskPriority priority;
    TaskPriority base_priority;
    uint32_t cpu_time;
    uint32_t quantum_used;
    uint32_t consecutive_quantums;
    uint32_t ready_timestamp;
    uint32_t consecutive_runs;
} TCB;

static TCB tasks[MAX_TASKS];
static uint8_t num_tasks = 0;
static volatile uint32_t ticks = 0;
static volatile uint8_t current_task = 0;
static volatile uint32_t quantum_counter = 0;
static volatile uint8_t preempt_flag = 0;
static volatile uint8_t force_schedule = 0;

// ============================================================================
// Obtener quantum según prioridad
// ============================================================================
static uint8_t get_task_quantum(TaskPriority priority) {
    switch(priority) {
        case PRIO_CRITICAL: return QUANTUM_CRITICAL;
        case PRIO_HIGH:     return QUANTUM_HIGH;
        case PRIO_NORMAL:   return QUANTUM_NORMAL;
        case PRIO_LOW:      return QUANTUM_LOW;
        case PRIO_IDLE:     return QUANTUM_IDLE;
        default:            return QUANTUM_NORMAL;
    }
}

// ============================================================================
// Verificar si hay tareas de mayor prioridad listas
// ============================================================================
static uint8_t has_higher_priority_ready(uint8_t task_id) {
    TaskPriority current_priority = tasks[task_id].priority;

    for (uint8_t i = 0; i < num_tasks; i++) {
        if (i != task_id && tasks[i].state == TASK_READY) {
            if (tasks[i].priority > current_priority) {
                return 1;
            }
        }
    }
    return 0;
}

// ============================================================================
// Scheduler con límites de ejecución consecutiva
// ============================================================================
static uint8_t find_highest_priority_ready(void) {
    int8_t highest_prio = -1;
    uint8_t selected_task = 0;
    uint32_t earliest_ready = 0xFFFFFFFF;

    // Primera pasada: buscar la tarea de mayor prioridad
    for (uint8_t i = 0; i < num_tasks; i++) {
        if (tasks[i].state == TASK_READY) {
            int8_t task_prio = (int8_t)tasks[i].priority;

            if (task_prio > highest_prio) {
                highest_prio = task_prio;
                selected_task = i;
                earliest_ready = tasks[i].ready_timestamp;
            }
            else if (task_prio == highest_prio) {
                int32_t time_diff = (int32_t)(tasks[i].ready_timestamp - earliest_ready);
                if (time_diff < 0) {
                    selected_task = i;
                    earliest_ready = tasks[i].ready_timestamp;
                }
            }
        }
    }

    // Aplicar límite de ejecuciones consecutivas
    TaskPriority selected_priority = tasks[selected_task].priority;

    // Si es LOW y ha ejecutado demasiadas veces, buscar alternativa
    if (selected_priority == PRIO_LOW &&
        tasks[selected_task].consecutive_runs >= MAX_LOW_CONSECUTIVE) {

        for (uint8_t i = 0; i < num_tasks; i++) {
            if (tasks[i].state == TASK_READY &&
                tasks[i].priority > PRIO_LOW) {
                selected_task = i;
                break;
            }
        }
    }

    // Si es NORMAL y ha ejecutado demasiadas veces, y hay CRITICAL/HIGH disponible
    if (selected_priority == PRIO_NORMAL &&
        tasks[selected_task].consecutive_runs >= MAX_NORMAL_CONSECUTIVE) {

        for (uint8_t i = 0; i < num_tasks; i++) {
            if (tasks[i].state == TASK_READY &&
                (tasks[i].priority == PRIO_CRITICAL || tasks[i].priority == PRIO_HIGH)) {
                selected_task = i;
                break;
            }
        }
    }

    return selected_task;
}

// ============================================================================
// AGING
// ============================================================================
static void apply_aging(void) {
    uint32_t current_time = ticks;

    for (uint8_t i = 0; i < num_tasks; i++) {
        if (tasks[i].state == TASK_READY && i != current_task) {
            uint32_t waiting_time = current_time - tasks[i].ready_timestamp;

            if (waiting_time >= AGING_THRESHOLD && tasks[i].priority < PRIO_CRITICAL) {
                tasks[i].priority++;
                tasks[i].ready_timestamp = current_time;
            }
        }
    }
}

// ============================================================================
// SYSTICK HANDLER con quantum variable
// ============================================================================
void SysTick_Handler(void) {
    ticks++;
    quantum_counter++;

    // 1️ Despertar tareas bloqueadas
    for (uint8_t i = 0; i < num_tasks; i++) {
        if (tasks[i].state == TASK_BLOCKED && ticks >= tasks[i].next_wake) {
            tasks[i].state = TASK_READY;
            tasks[i].ready_timestamp = ticks;
            tasks[i].consecutive_quantums = 0;
            tasks[i].consecutive_runs = 0;
            force_schedule = 1;
        }
    }

    // 2️ Aplicar aging
    apply_aging();

    // 3️ PREEMPTION CHECK con quantum variable
    uint8_t current_quantum_limit = get_task_quantum(tasks[current_task].priority);

    if (quantum_counter >= current_quantum_limit) {
        quantum_counter = 0;
        tasks[current_task].quantum_used++;
        tasks[current_task].consecutive_quantums++;

        // Verificar si hay tareas de mayor prioridad esperando
        if (has_higher_priority_ready(current_task)) {
            preempt_flag = 1;
            force_schedule = 1;
        }
        // O si la tarea actual ha usado todos sus quantums
        else if (tasks[current_task].consecutive_quantums >= MAX_QUANTUM_SLOTS) {
            preempt_flag = 1;
            force_schedule = 1;
        }
    }
}

// ============================================================================
// API PÚBLICA
// ============================================================================
void task_create(void (*func)(void), TaskPriority priority) {
    if (num_tasks >= MAX_TASKS) return;

    tasks[num_tasks].func = func;
    tasks[num_tasks].state = TASK_READY;
    tasks[num_tasks].next_wake = 0;
    tasks[num_tasks].last_wake = 0;
    tasks[num_tasks].overrun_count = 0;
    tasks[num_tasks].priority = priority;
    tasks[num_tasks].base_priority = priority;
    tasks[num_tasks].cpu_time = 0;
    tasks[num_tasks].quantum_used = 0;
    tasks[num_tasks].consecutive_quantums = 0;
    tasks[num_tasks].ready_timestamp = 0;
    tasks[num_tasks].consecutive_runs = 0;

    num_tasks++;
}

void task_delay(uint32_t ms) {
    tasks[current_task].next_wake = ticks + ms;
    tasks[current_task].state = TASK_BLOCKED;

    // CRÍTICO: Restaurar prioridad base cuando la tarea se bloquea
    tasks[current_task].priority = tasks[current_task].base_priority;

    tasks[current_task].quantum_used = 0;
    tasks[current_task].consecutive_quantums = 0;
    tasks[current_task].consecutive_runs = 0;

    force_schedule = 1;
}

void task_yield(void) {
    tasks[current_task].ready_timestamp = ticks;
    tasks[current_task].consecutive_runs = 0;
    preempt_flag = 1;
    force_schedule = 1;
    tasks[current_task].quantum_used = 0;
    tasks[current_task].consecutive_quantums = 0;
}

void task_set_priority(uint8_t task_id, TaskPriority priority) {
    if (task_id >= num_tasks) return;
    tasks[task_id].priority = priority;
    tasks[task_id].base_priority = priority;
    force_schedule = 1;
}

uint8_t get_current_task_id(void) {
    return current_task;
}

TaskPriority get_task_priority(uint8_t task_id) {
    if (task_id >= num_tasks) return PRIO_IDLE;
    return tasks[task_id].priority;
}

uint32_t millis(void) {
    return ticks;
}

uint32_t task_get_overruns(uint8_t task_id) {
    if (task_id >= num_tasks) return 0;
    return tasks[task_id].overrun_count;
}

void task_exit(void) {
    tasks[current_task].state = TASK_SUSPENDED;
    while(1) {
        task_yield();
        task_delay(1000);
    }
}

uint8_t task_get_count(void) {
    return num_tasks;
}

int get_task_list(TaskInfo* list, int max_tasks) {
    int count = (num_tasks < max_tasks) ? num_tasks : max_tasks;
    for (int i = 0; i < count; i++) {
        list[i].id = i;
        list[i].state = tasks[i].state;
        list[i].priority = tasks[i].priority;
        list[i].cpu_time = tasks[i].cpu_time;
    }
    return count;
}

void sched_kill_all_tasks(void) {
    volatile uint32_t* SYST_CSR = (volatile uint32_t*)0xE000E010;

    for (uint8_t i = 0; i < num_tasks; i++) {
        tasks[i].state = TASK_SUSPENDED;
    }

    // Detener el SysTick
    *SYST_CSR = 0x00;

    extern void daos_uart_puts(const char*);
    daos_uart_puts("[SCHED] All tasks killed, SysTick stopped\r\n");
}

// ============================================================================
// SCHEDULER PRINCIPAL con tracking de ejecuciones
// ============================================================================
void sched_start(void) {
    volatile uint32_t* SYST_CSR = (volatile uint32_t*)0xE000E010;
    volatile uint32_t* SYST_RVR = (volatile uint32_t*)0xE000E014;

    if (num_tasks == 0) return;

    *SYST_RVR = 16000 - 1;
    *SYST_CSR = 0x07;

    for (uint8_t i = 0; i < num_tasks; i++) {
        tasks[i].state = TASK_READY;
        tasks[i].ready_timestamp = 0;
        tasks[i].consecutive_runs = 0;
    }

    uint8_t last_executed_task = 0xFF;

    while(1) {
        // Verificar si TODAS las tareas están suspendidas
        uint8_t all_suspended = 1;
        for (uint8_t i = 0; i < num_tasks; i++) {
            if (tasks[i].state != TASK_SUSPENDED) {
                all_suspended = 0;
                break;
            }
        }

        // Si todas las tareas están muertas, SALIR del scheduler
        if (all_suspended) {
            *SYST_CSR = 0x00;  // Detener SysTick
            num_tasks = 0;

            extern void daos_uart_puts(const char*);
            daos_uart_puts("[SCHEDULER] All tasks suspended - Exiting to menu\r\n\r\n");

            return;
        }

        current_task = find_highest_priority_ready();

        // Actualizar contador de ejecuciones consecutivas
        if (current_task == last_executed_task) {
            tasks[current_task].consecutive_runs++;
        } else {
            if (last_executed_task != 0xFF) {
                tasks[last_executed_task].consecutive_runs = 0;
            }
            tasks[current_task].consecutive_runs = 1;
            last_executed_task = current_task;
        }

        preempt_flag = 0;
        force_schedule = 0;

        if (tasks[current_task].state == TASK_READY) {
            uint32_t start_time = ticks;
            tasks[current_task].last_wake = start_time;

            tasks[current_task].func();

            tasks[current_task].cpu_time += (ticks - start_time);
        }
    }
}
