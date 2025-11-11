/**
 * ============================================================================
 * HERENCIAPRIORIDAD.C - Prueba de Herencia de Prioridad REAL
 * ============================================================================
 */

#include "herenciaprioridad.h"
#include "api.h"

// Contadores
static volatile uint32_t count_a = 0;
static volatile uint32_t count_b = 0;
static volatile uint32_t b_execution_count = 0;
static volatile uint32_t a_blocked_count = 0;
static volatile uint32_t a_acquired_count = 0;
static volatile uint32_t context_switches = 0;
static volatile uint8_t last_task_id = 0;

// 🔥 NUEVO: Flag para saber si B tiene el mutex entre ejecuciones
static volatile uint8_t b_has_mutex = 0;

// Mutex compartido
static uint8_t mutex_storage[32];
static daos_mutex_t shared_mutex = NULL;

// Colores
#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_RED     0xF800
#define COLOR_BLUE    0x001F
#define COLOR_YELLOW  0xFFE0
#define COLOR_GREEN   0x07E0
#define COLOR_CYAN    0x07FF
#define COLOR_MAGENTA 0xF81F
#define COLOR_ORANGE  0xFD20

// ============================================================================
// Utilidades
// ============================================================================
static void uint32_to_str(uint32_t num, char* buf) {
    buf[0] = '0' + (num / 10000) % 10;
    buf[1] = '0' + (num / 1000) % 10;
    buf[2] = '0' + (num / 100) % 10;
    buf[3] = '0' + (num / 10) % 10;
    buf[4] = '0' + num % 10;
    buf[5] = '\0';
}

static void draw_header(void) {
    daos_gfx_draw_text_large(10, 2, "PRIORITY INHERIT", COLOR_MAGENTA, COLOR_BLACK, 2);
}

static void draw_stats(void) {
    char buffer[16];

    // Tarea A
    daos_gfx_draw_text_large(5, 30, "A(HIGH):", COLOR_RED, COLOR_BLACK, 2);
    uint32_to_str(count_a, buffer);
    daos_gfx_draw_text_large(120, 30, buffer, COLOR_WHITE, COLOR_BLACK, 2);

    // Tarea B
    daos_gfx_draw_text_large(5, 55, "B(LOW):", COLOR_BLUE, COLOR_BLACK, 2);
    uint32_to_str(count_b, buffer);
    daos_gfx_draw_text_large(120, 55, buffer, COLOR_WHITE, COLOR_BLACK, 2);

    // A bloqueada
    daos_gfx_draw_text(5, 85, "A Blocked:", COLOR_ORANGE, COLOR_BLACK);
    uint32_to_str(a_blocked_count, buffer);
    daos_gfx_draw_text(80, 85, buffer, COLOR_RED, COLOR_BLACK);

    // A adquirió
    daos_gfx_draw_text(5, 100, "A Acquired:", COLOR_ORANGE, COLOR_BLACK);
    uint32_to_str(a_acquired_count, buffer);
    daos_gfx_draw_text(80, 100, buffer, COLOR_GREEN, COLOR_BLACK);

    // 🔥 HERENCIAS REALES desde el mutex
    uint32_t inheritance_count = daos_mutex_get_inheritance_count(shared_mutex);
    daos_gfx_draw_text_large(5, 125, "INHERIT:", COLOR_MAGENTA, COLOR_BLACK, 2);
    uint32_to_str(inheritance_count, buffer);
    daos_gfx_draw_text_large(120, 125, buffer, COLOR_YELLOW, COLOR_BLACK, 2);

    // Context switches
    daos_gfx_draw_text(5, 155, "CTX:", COLOR_GREEN, COLOR_BLACK);
    uint32_to_str(context_switches, buffer);
    daos_gfx_draw_text(45, 155, buffer, COLOR_WHITE, COLOR_BLACK);
}

// ============================================================================
// Trabajo pesado
// ============================================================================
static void do_heavy_work(uint32_t iterations) {
    volatile uint32_t dummy = 0;
    for (uint32_t i = 0; i < iterations; i++) {
        dummy += i * 3;
        dummy = dummy % 1000;
        for (uint32_t j = 0; j < 100; j++) {
            dummy = (dummy * 13 + 7) % 997;
        }
    }
}

// ============================================================================
// Tarea A (HIGH): Intenta adquirir mutex con mutex_lock simplificado
// ============================================================================
static void task_a(void) {
    count_a++;

    daos_uart_putc('A');
    daos_uart_puts("[A/HIGH] Intentando adquirir mutex...\r\n");

    uint32_t start_time = daos_millis();

    // 🔥 USAR MUTEX_LOCK_EX (aplica herencia automáticamente)
    int acquired = daos_mutex_lock_ex(shared_mutex);

    if (!acquired) {
        // ❌ Mutex ocupado - Herencia YA APLICADA por mutex_lock()
        a_blocked_count++;
        uint32_t wait_time = daos_millis() - start_time;
        daos_uart_puts("[A/HIGH] *** BLOQUEADA (mutex ocupado) - esperó ");
        daos_uart_putint(wait_time);
        daos_uart_puts("ms ***\r\n\r\n");

        daos_sleep_ms(10);  // Dormir y reintentar
        return;
    }

    // ✅ MUTEX ADQUIRIDO
    uint32_t wait_time = daos_millis() - start_time;
    a_acquired_count++;

    daos_uart_puts("[A/HIGH] *** MUTEX ADQUIRIDO (esperó ");
    daos_uart_putint(wait_time);
    daos_uart_puts("ms) ***\r\n");

    // Simular trabajo crítico breve
    do_heavy_work(100);

    // Liberar inmediatamente
    daos_mutex_unlock(shared_mutex);
    daos_uart_puts("[A/HIGH] Mutex liberado\r\n\r\n");

    // Actualizar context switches
    uint8_t current_task = daos_get_current_task_id();
    if (current_task != last_task_id) {
        context_switches++;
        last_task_id = current_task;
    }

    // Dormir antes de reintentar
    daos_sleep_ms(50);
}

// ============================================================================
// 🔥 Tarea B (LOW): MÁQUINA DE ESTADOS para retener mutex
// ============================================================================
static void task_b(void) {
    count_b++;

    daos_uart_putc('B');

    // 🔥 ESTADO 1: Adquirir mutex por primera vez
    if (b_execution_count == 0) {
        daos_uart_puts("\r\n[B/LOW] EJECUCIÓN 1/4 - INTENTANDO ADQUIRIR MUTEX\r\n");

        int acquired = daos_mutex_trylock(shared_mutex);

        if (!acquired) {
            daos_uart_puts("[B/LOW] Mutex ocupado, reintentando...\r\n\r\n");
            daos_sleep_ms(50);
            return;
        }

        // ✅ Mutex adquirido
        b_has_mutex = 1;
        b_execution_count = 1;

        uint8_t task_id = daos_get_current_task_id();
        daos_priority_t prio = daos_get_task_real_priority(task_id);

        daos_uart_puts("[B/LOW] ✅ MUTEX ADQUIRIDO - Prioridad: ");
        daos_uart_putint((uint32_t)prio);
        daos_uart_puts("\r\n");

        // Trabajo pesado
        daos_uart_puts("[B/LOW] Haciendo trabajo pesado...\r\n");
        do_heavy_work(800);

        daos_uart_puts("[B/LOW] *** MUTEX RETENIDO (1/4) ***\r\n\r\n");

        // 🔥 CRÍTICO: NO LIBERAR EL MUTEX, solo dormir
        daos_sleep_ms(200);
        return;
    }

    // 🔥 ESTADOS 2-3: Mantener mutex sin liberarlo
    if (b_execution_count >= 1 && b_execution_count < 3) {
        b_execution_count++;

        daos_uart_puts("\r\n[B/LOW] EJECUCIÓN ");
        daos_uart_putint(b_execution_count);
        daos_uart_puts("/4 - MANTENIENDO MUTEX\r\n");

        // Verificar que aún tenemos el mutex
        uint8_t task_id = daos_get_current_task_id();
        daos_priority_t prio = daos_get_task_real_priority(task_id);
        daos_uart_puts("[B/LOW] Prioridad actual: ");
        daos_uart_putint((uint32_t)prio);
        daos_uart_puts("\r\n");

        do_heavy_work(800);

        daos_uart_puts("[B/LOW] *** MUTEX AÚN RETENIDO (");
        daos_uart_putint(b_execution_count);
        daos_uart_puts("/4) ***\r\n\r\n");

        // 🔥 CRÍTICO: NO LIBERAR EL MUTEX
        daos_sleep_ms(200);
        return;
    }

    // 🔥 ESTADO 4: FINALMENTE LIBERAR MUTEX Y RESTAURAR PRIORIDAD
    if (b_execution_count == 3) {
        b_execution_count = 4;

        daos_uart_puts("\r\n[B/LOW] EJECUCIÓN 4/4 - LIBERANDO MUTEX\r\n");

        do_heavy_work(800);

        // 🔥 LIBERAR MUTEX Y RESTAURAR PRIORIDAD
        if (b_has_mutex) {
            uint8_t task_id = daos_get_current_task_id();
            daos_priority_t prio_before = daos_get_task_real_priority(task_id);

            daos_uart_puts("[B/LOW] Prioridad antes de liberar: ");
            daos_uart_putint((uint32_t)prio_before);
            daos_uart_puts("\r\n");

            // Liberar mutex (esto restaura prioridad automáticamente)
            daos_mutex_unlock(shared_mutex);
            b_has_mutex = 0;

            daos_priority_t prio_after = daos_get_task_real_priority(task_id);
            daos_uart_puts("[B/LOW] Prioridad después de liberar: ");
            daos_uart_putint((uint32_t)prio_after);
            daos_uart_puts("\r\n");

            daos_uart_puts("[B/LOW] ✅ *** MUTEX LIBERADO *** ✅\r\n");
        }

        daos_uart_puts("[B/LOW] Ciclo completado. Reiniciando.\r\n\r\n");

        // Reiniciar contador para próximo ciclo
        b_execution_count = 0;

        // Actualizar context switches
        uint8_t current_task = daos_get_current_task_id();
        if (current_task != last_task_id) {
            context_switches++;
            last_task_id = current_task;
        }

        daos_sleep_ms(500);
        return;
    }
}

// ============================================================================
// Monitor: Actualiza pantalla
// ============================================================================
static void monitor_task(void) {
    static uint8_t refresh_counter = 0;

    refresh_counter++;

    if (refresh_counter >= 5) {
        draw_stats();
        refresh_counter = 0;

        // 🔥 Obtener herencias REALES del mutex
        uint32_t inheritance_count = daos_mutex_get_inheritance_count(shared_mutex);

        daos_uart_puts("\r\n--- ESTADÍSTICAS ---\r\n");
        daos_uart_puts("A ejecuciones: ");
        daos_uart_putint(count_a);
        daos_uart_puts("\r\nB ejecuciones: ");
        daos_uart_putint(count_b);
        daos_uart_puts("\r\nB en estado: ");
        daos_uart_putint(b_execution_count);
        daos_uart_puts("/4\r\n");
        daos_uart_puts("A bloqueada: ");
        daos_uart_putint(a_blocked_count);
        daos_uart_puts("\r\nA adquirió: ");
        daos_uart_putint(a_acquired_count);
        daos_uart_puts("\r\n>>> HERENCIAS: ");
        daos_uart_putint(inheritance_count);
        daos_uart_puts(" <<<\r\n");
        daos_uart_puts("Context switches: ");
        daos_uart_putint(context_switches);
        daos_uart_puts("\r\n\r\n");
    }

    daos_sleep_ms(100);
}

// ============================================================================
// Inicialización
// ============================================================================
void herenciaprioridad_init(void) {
    count_a = 0;
    count_b = 0;
    b_execution_count = 0;
    a_blocked_count = 0;
    a_acquired_count = 0;
    context_switches = 0;
    last_task_id = 0;
    b_has_mutex = 0;

    // Inicializar mutex
    shared_mutex = (daos_mutex_t)mutex_storage;
    daos_mutex_init(shared_mutex);

    // Pantalla
    daos_gfx_clear(COLOR_BLACK);
    draw_header();
    draw_stats();

    daos_uart_puts("\r\n============================================\r\n");
    daos_uart_puts("   PRUEBA DE HERENCIA DE PRIORIDAD\r\n");
    daos_uart_puts("     MUTEX SIMPLE NO-BLOQUEANTE\r\n");
    daos_uart_puts("============================================\r\n");
    daos_uart_puts("Tarea A (HIGH): Intenta adquirir mutex\r\n");
    daos_uart_puts("  - Si ocupado → Termina y reintenta\r\n");
    daos_uart_puts("  - Herencia de prioridad aplicada\r\n");
    daos_uart_puts("\r\n");
    daos_uart_puts("Tarea B (LOW): Retiene mutex 4 ejecuciones\r\n");
    daos_uart_puts("  - Máquina de estados (1→2→3→4)\r\n");
    daos_uart_puts("  - B hereda prioridad HIGH de A\r\n");
    daos_uart_puts("============================================\r\n\r\n");

    // Crear tareas
    daos_task_create(task_a, DAOS_PRIO_HIGH);
    daos_task_create(task_b, DAOS_PRIO_LOW);
    daos_task_create(monitor_task, DAOS_PRIO_LOW);

    daos_uart_puts("[INIT] 3 tareas creadas\r\n\r\n");
}

void herenciaprioridad_reset(void) {
    count_a = 0;
    count_b = 0;
    b_execution_count = 0;
    a_blocked_count = 0;
    a_acquired_count = 0;
    context_switches = 0;
    last_task_id = 0;
    b_has_mutex = 0;

    daos_mutex_reset_inheritance_count(shared_mutex);

    daos_gfx_clear(COLOR_BLACK);
    draw_header();
    draw_stats();

    daos_uart_puts("[RESET] Contadores reiniciados\r\n");
}

void herenciaprioridad_cleanup(void) {
    daos_gfx_clear(COLOR_BLACK);
    daos_uart_puts("\r\n[CLEANUP] Prueba finalizada\r\n");
}

void herenciaprioridad_input_task(void) {
    daos_sleep_ms(1000);
}

void herenciaprioridad_logic_task(void) {
    daos_sleep_ms(1000);
}

void herenciaprioridad_render_task(void) {
    daos_sleep_ms(1000);
}

uint32_t herenciaprioridad_get_count_high(void) {
    return count_a;
}

uint32_t herenciaprioridad_get_count_low(void) {
    return count_b;
}

uint32_t herenciaprioridad_get_blocked_count(void) {
    return a_blocked_count;
}

uint32_t herenciaprioridad_get_inheritance_count(void) {
    return daos_mutex_get_inheritance_count(shared_mutex);
}

void* herenciaprioridad_get_binario(void) {
    return NULL;
}
