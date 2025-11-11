#include "context.h"

CTX_TCB ctx_tasks[2];
volatile uint8_t ctx_current = 0;

uint32_t ctx_stack1[CTX_STACK_SIZE];
uint32_t ctx_stack2[CTX_STACK_SIZE];

/* Inicializar stack de tarea */
static uint32_t* ctx_init_stack(void (*func)(void), uint32_t *stack) {
    uint32_t *stk = &stack[CTX_STACK_SIZE - 1];

    *(stk) = 0x01000000;              // xPSR
    *(--stk) = (uint32_t)func;        // PC
    *(--stk) = 0xFFFFFFFD;            // LR
    *(--stk) = 0x12121212;            // R12
    *(--stk) = 0x03030303;            // R3
    *(--stk) = 0x02020202;            // R2
    *(--stk) = 0x01010101;            // R1
    *(--stk) = 0x00000000;            // R0
    *(--stk) = 0x11111111;            // R11
    *(--stk) = 0x10101010;            // R10
    *(--stk) = 0x09090909;            // R9
    *(--stk) = 0x08080808;            // R8
    *(--stk) = 0x07070707;            // R7
    *(--stk) = 0x06060606;            // R6
    *(--stk) = 0x05050505;            // R5
    *(--stk) = 0x04040404;            // R4

    return stk;
}

void ctx_init(void) {
    ctx_current = 0;

    for (uint8_t i = 0; i < 2; i++) {
        ctx_tasks[i].sp = 0;
        ctx_tasks[i].delay_ctx = 0;
        ctx_tasks[i].state_ctx = 0;
    }
}

void ctx_create_task(uint8_t task_id, void (*func)(void)) {
    if (task_id >= 2) return;

    uint32_t *stack = (task_id == 0) ? ctx_stack1 : ctx_stack2;

    ctx_tasks[task_id].sp = ctx_init_stack(func, stack);
    ctx_tasks[task_id].delay_ctx = 0;
    ctx_tasks[task_id].state_ctx = 0;
}

void ctx_switch_handler(void) {
    // Guardar contexto actual (simulado)
    // En hardware real se guardaría el SP actual

    // Actualizar delays
    for (uint8_t i = 0; i < 2; i++) {
        if (ctx_tasks[i].state_ctx == 1 && ctx_tasks[i].delay_ctx > 0) {
            ctx_tasks[i].delay_ctx--;
            if (ctx_tasks[i].delay_ctx == 0) {
                ctx_tasks[i].state_ctx = 0;
            }
        }
    }

    // Buscar siguiente tarea READY
    uint8_t next = ctx_current;
    for (uint8_t i = 0; i < 2; i++) {
        next = (next + 1) % 2;
        if (ctx_tasks[next].state_ctx == 0) {
            break;
        }
    }

    ctx_current = next;

    // Restaurar contexto (simulado)
    // En hardware real, esto restauraría R4-R11 y PSP
}

void ctx_delay(uint32_t ms) {
    ctx_tasks[ctx_current].state_ctx = 1;
    ctx_tasks[ctx_current].delay_ctx = ms;
    ctx_switch_handler();
}

uint8_t ctx_get_current(void) {
    return ctx_current;
}

uint8_t ctx_get_task_state(uint8_t task_id) {
    if (task_id >= 2) return 1;
    return ctx_tasks[task_id].state_ctx;
}
