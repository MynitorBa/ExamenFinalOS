#include "context.h" // Incluye definiciones de CTX_TCB y CTX_STACK_SIZE
#include <stddef.h>  // Para NULL
#include <stdint.h>  // Para tipos de enteros fijos

/** Array de bloques de control de tarea (TCB) para las dos tareas. */
CTX_TCB ctx_tasks[2];
/** ID de la tarea actualmente en ejecución (0 o 1). Volátil por interrupciones. */
volatile uint8_t ctx_current = 0;

/** Pila estática para la Tarea 1. */
uint32_t ctx_stack1[CTX_STACK_SIZE];
/** Pila estática para la Tarea 2. */
uint32_t ctx_stack2[CTX_STACK_SIZE];

/* Inicializar stack de tarea */
/**
 * Configura la pila inicial de una nueva tarea, simulando el apilamiento de registros del ARM Cortex-M.
 * @param func Puntero a la función de entrada.
 * @param stack Puntero al array de la pila.
 * @return Puntero al Stack Pointer (SP) inicial (ubicación del registro R4).
 */
static uint32_t* ctx_init_stack(void (*func)(void), uint32_t *stack) {
    uint32_t *stk = &stack[CTX_STACK_SIZE - 1];

    // Apilamiento de registros (simulación del hardware):
    *(stk) = 0x01000000;              /** xPSR (T-bit a 1) */
    *(--stk) = (uint32_t)func;        /** PC (Program Counter) */
    *(--stk) = 0xFFFFFFFD;            /** LR (Link Register - valor de retorno de handler) */
    *(--stk) = 0x12121212;            /** R12 */
    *(--stk) = 0x03030303;            /** R3 */
    *(--stk) = 0x02020202;            /** R2 */
    *(--stk) = 0x01010101;            /** R1 */
    *(--stk) = 0x00000000;            /** R0 */
    // Registros guardados por software
    *(--stk) = 0x11111111;            /** R11 */
    *(--stk) = 0x10101010;            /** R10 */
    *(--stk) = 0x09090909;            /** R9 */
    *(--stk) = 0x08080808;            /** R8 */
    *(--stk) = 0x07070707;            /** R7 */
    *(--stk) = 0x06060606;            /** R6 */
    *(--stk) = 0x05050505;            /** R5 */
    *(--stk) = 0x04040404;            /** R4 */

    return stk; // Nuevo SP (apunta al registro R4)
}

/** Inicializa el sistema de cambio de contexto (Context Switch). */
void ctx_init(void) {
    ctx_current = 0;

    // Limpia los TCBs
    for (uint8_t i = 0; i < 2; i++) {
        ctx_tasks[i].sp = 0;
        ctx_tasks[i].delay_ctx = 0;
        ctx_tasks[i].state_ctx = 0; // READY
    }
}

/**
 * Crea una nueva tarea y configura su pila.
 * @param task_id ID de la tarea (0 o 1).
 * @param func Puntero a la función de entrada.
 */
void ctx_create_task(uint8_t task_id, void (*func)(void)) {
    if (task_id >= 2) return;

    // Asignar pila
    uint32_t *stack = (task_id == 0) ? ctx_stack1 : ctx_stack2;

    // Inicializar el TCB y la pila
    ctx_tasks[task_id].sp = ctx_init_stack(func, stack);
    ctx_tasks[task_id].delay_ctx = 0;
    ctx_tasks[task_id].state_ctx = 0; // READY
}

/**
 * Handler de cambio de contexto (función de planificador simple).
 * * Actualiza los contadores de retardo.
 * * Selecciona la siguiente tarea READY (Round-Robin).
 */
void ctx_switch_handler(void) {
    // Nota: El hardware/assembly guarda el SP actual en el TCB antes de este punto.

    // Actualizar delays y desbloquear tareas
    for (uint8_t i = 0; i < 2; i++) {
        // Si está BLOCKED y tiene delay pendiente
        if (ctx_tasks[i].state_ctx == 1 && ctx_tasks[i].delay_ctx > 0) {
            ctx_tasks[i].delay_ctx--;
            if (ctx_tasks[i].delay_ctx == 0) {
                ctx_tasks[i].state_ctx = 0; // Pasa a READY
            }
        }
    }

    // Buscar siguiente tarea READY (Round-Robin simple)
    uint8_t next = ctx_current;
    for (uint8_t i = 0; i < 2; i++) {
        next = (next + 1) % 2; // Siguiente tarea circular
        if (ctx_tasks[next].state_ctx == 0) { // Si está READY
            break;
        }
    }

    // Actualizar la tarea actual
    ctx_current = next;

    // Nota: El hardware/assembly restauraría el SP de ctx_current después de este punto.
}

/**
 * Bloquea la tarea actual por un tiempo y fuerza un cambio de contexto.
 * @param ms Milisegundos a bloquear.
 */
void ctx_delay(uint32_t ms) {
    // Bloquear la tarea actual
    ctx_tasks[ctx_current].state_ctx = 1;
    ctx_tasks[ctx_current].delay_ctx = ms;
    // Forzar el cambio de contexto
    ctx_switch_handler();
}

/**
 * Obtiene el ID de la tarea actualmente en ejecución.
 * @return ID de la tarea (0 o 1).
 */
uint8_t ctx_get_current(void) {
    return ctx_current;
}

/**
 * Obtiene el estado de bloqueo/listo de una tarea.
 * @param task_id ID de la tarea.
 * @return Estado (0=READY, 1=BLOCKED).
 */
uint8_t ctx_get_task_state(uint8_t task_id) {
    if (task_id >= 2) return 1; // Si es inválido, asumir BLOCKED
    return ctx_tasks[task_id].state_ctx;
}
