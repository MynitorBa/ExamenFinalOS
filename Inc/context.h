#pragma once // Directiva alternativa de guarda
#include <stdint.h> // Incluye tipos de enteros fijos

/** Tamaño de la pila (stack) para cada contexto de tarea. */
#define CTX_STACK_SIZE 256

/** Estructura del Bloque de Control de Tarea (TCB) para manejo de contexto. */
typedef struct {
    uint32_t *sp;       /** Puntero de pila (stack pointer). Debe ser el primer miembro. */
    uint32_t delay_ctx; /** Contador de retardo para bloqueo temporizado. */
    uint8_t state_ctx;  /** Estado de la tarea: 0=ready, 1=blocked */
} CTX_TCB;

/**
 * Inicializa el sistema de cambio de contexto (context switch).
 */
void ctx_init(void);

/**
 * Crea una nueva tarea en el sistema para ser gestionada.
 * @param task_id ID único para la tarea.
 * @param func Puntero a la función de entrada (entry point) de la tarea.
 */
void ctx_create_task(uint8_t task_id, void (*func)(void));

/**
 * Handler principal para realizar el cambio de contexto.
 * Es típicamente llamado por una interrupción de temporizador (timer).
 */
void ctx_switch_handler(void);

/**
 * Bloquea la tarea actual por un tiempo especificado.
 * @param ms Milisegundos que la tarea permanecerá bloqueada.
 */
void ctx_delay(uint32_t ms);

/**
 * Obtiene el ID de la tarea actualmente en ejecución.
 * @return ID de la tarea actual (uint8_t).
 */
uint8_t ctx_get_current(void);

/**
 * Obtiene el estado actual de una tarea específica.
 * @param task_id ID de la tarea a consultar.
 * @return Estado de la tarea (0=ready, 1=blocked).
 */
uint8_t ctx_get_task_state(uint8_t task_id);
