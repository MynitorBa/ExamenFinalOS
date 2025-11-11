#pragma once
#include <stdint.h>

#define CTX_STACK_SIZE 256

typedef struct {
    uint32_t *sp;       // Stack pointer (DEBE ser primero)
    uint32_t delay_ctx;
    uint8_t state_ctx;  // 0=ready, 1=blocked
} CTX_TCB;

/* Inicializar el sistema de context switch */
void ctx_init(void);

/* Crear una tarea con context switch */
void ctx_create_task(uint8_t task_id, void (*func)(void));

/* Handler de context switch (llamado por timer) */
void ctx_switch_handler(void);

/* Delay para context switch */
void ctx_delay(uint32_t ms);

/* Obtener tarea actual */
uint8_t ctx_get_current(void);

/* Obtener estado de una tarea (0=ready, 1=blocked) */
uint8_t ctx_get_task_state(uint8_t task_id);
