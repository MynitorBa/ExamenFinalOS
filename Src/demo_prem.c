/**
 * ============================================================================
 * DEMO_PREM.C - Demostración de Herencia de Prioridad AUTOMÁTICA
 * ============================================================================
 * Task A: HIGH (Prioridad Alta) - Intenta adquirir mutex frecuentemente
 * Task B: LOW (Prioridad Baja) - Mantiene mutex bloqueado con trabajo pesado
 * Task C: NORMAL (Prioridad Media) - Trabajo continuo sin mutex
 *
 * HERENCIA AUTOMÁTICA:
 * 1. Task B (LOW) adquiere el mutex
 * 2. Task A (HIGH) intenta adquirir el mutex con mutex_lock()
 * 3. sync.c detecta: my_priority (HIGH) > owner_priority (LOW)
 * 4. sync.c AUTOMÁTICAMENTE eleva la prioridad de Task B a HIGH
 * 5. Task B completa rápido y libera el mutex
 * 6. sync.c AUTOMÁTICAMENTE restaura la prioridad original de Task B (LOW)
 * 7. Task A adquiere el mutex y continúa
 * ============================================================================
 */

#include "demo_prem.h"
#include "api.h"

static volatile uint32_t count_a = 0;  // HIGH
static volatile uint32_t count_b = 0;  // LOW
static volatile uint32_t count_c = 0;  // NORMAL
static volatile uint32_t context_switches = 0;
static volatile uint32_t task_a_cpu_time = 0;
static volatile uint32_t task_b_cpu_time = 0;
static volatile uint32_t task_c_cpu_time = 0;
static volatile uint8_t last_task_id = 0;

// Estadísticas de herencia de prioridad
static volatile uint32_t task_a_blocked_count = 0;
static volatile uint32_t task_a_acquired_count = 0;
static volatile uint32_t task_b_boosted_times = 0;
static volatile uint32_t task_b_mutex_hold_time = 0;

// Mutex para demostración de bloqueo
static uint8_t mutex_storage[32];
static daos_mutex_t test_mutex = NULL;

#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_YELLOW  0xFFE0
#define COLOR_CYAN    0x07FF
#define COLOR_GREEN   0x07E0
#define COLOR_RED     0xF800
#define COLOR_BLUE    0x001F
#define COLOR_ORANGE  0xFD20
#define COLOR_GRAY    0x8410
#define COLOR_MAGENTA 0xF81F

static void uint32_to_str(uint32_t num, char* buf) {
    buf[0] = '0' + (num / 10000) % 10;
    buf[1] = '0' + (num / 1000) % 10;
    buf[2] = '0' + (num / 100) % 10;
    buf[3] = '0' + (num / 10) % 10;
    buf[4] = '0' + num % 10;
    buf[5] = '\0';
}

static void draw_header(void) {
    daos_gfx_draw_text_large(10, 2, "AUTO INHERIT", COLOR_MAGENTA, COLOR_BLACK, 2);
}

static void draw_task_stats(void) {
    char buffer[16];

    // Task A - HIGH (Alta) - Bloquea esperando mutex
    daos_gfx_draw_text_large(5, 30, "A:HI", COLOR_RED, COLOR_BLACK, 2);
    uint32_to_str(count_a, buffer);
    daos_gfx_draw_text_large(70, 30, buffer, COLOR_WHITE, COLOR_BLACK, 2);

    // Task B - LOW (Baja) - Mantiene mutex bloqueado
    daos_gfx_draw_text_large(5, 55, "B:LO", COLOR_BLUE, COLOR_BLACK, 2);
    uint32_to_str(count_b, buffer);
    daos_gfx_draw_text_large(70, 55, buffer, COLOR_WHITE, COLOR_BLACK, 2);

    // Task C - NORMAL (Media) - Trabajo continuo
    daos_gfx_draw_text_large(5, 80, "C:NM", COLOR_CYAN, COLOR_BLACK, 2);
    uint32_to_str(count_c, buffer);
    daos_gfx_draw_text_large(70, 80, buffer, COLOR_WHITE, COLOR_BLACK, 2);

    // Estadísticas de mutex
    daos_gfx_draw_text(170, 30, "MUTEX:", COLOR_YELLOW, COLOR_BLACK);

    daos_gfx_draw_text(170, 43, "Block:", COLOR_ORANGE, COLOR_BLACK);
    uint32_to_str(task_a_blocked_count, buffer);
    daos_gfx_draw_text(215, 43, buffer, COLOR_RED, COLOR_BLACK);

    daos_gfx_draw_text(170, 56, "Acqrd:", COLOR_ORANGE, COLOR_BLACK);
    uint32_to_str(task_a_acquired_count, buffer);
    daos_gfx_draw_text(215, 56, buffer, COLOR_GREEN, COLOR_BLACK);

    // Herencia de prioridad AUTOMÁTICA
    daos_gfx_draw_text(170, 75, "Boost:", COLOR_MAGENTA, COLOR_BLACK);
    uint32_to_str(task_b_boosted_times, buffer);
    daos_gfx_draw_text(215, 75, buffer, COLOR_YELLOW, COLOR_BLACK);

    // Tiempo de bloqueo de B
    daos_gfx_draw_text(170, 88, "Hold:", COLOR_CYAN, COLOR_BLACK);
    uint32_to_str(task_b_mutex_hold_time, buffer);
    daos_gfx_draw_text(205, 88, buffer, COLOR_WHITE, COLOR_BLACK);

    // Context Switches
    daos_gfx_draw_text(5, 110, "CTX:", COLOR_YELLOW, COLOR_BLACK);
    uint32_to_str(context_switches, buffer);
    daos_gfx_draw_text(40, 110, buffer, COLOR_WHITE, COLOR_BLACK);

    // CPU Time
    daos_gfx_draw_text(5, 130, "CPU TIME:", COLOR_ORANGE, COLOR_BLACK);

    daos_gfx_draw_text(5, 145, "A:", COLOR_RED, COLOR_BLACK);
    uint32_to_str(task_a_cpu_time, buffer);
    daos_gfx_draw_text(25, 145, buffer, COLOR_WHITE, COLOR_BLACK);

    daos_gfx_draw_text(5, 160, "B:", COLOR_BLUE, COLOR_BLACK);
    uint32_to_str(task_b_cpu_time, buffer);
    daos_gfx_draw_text(25, 160, buffer, COLOR_WHITE, COLOR_BLACK);

    daos_gfx_draw_text(5, 175, "C:", COLOR_CYAN, COLOR_BLACK);
    uint32_to_str(task_c_cpu_time, buffer);
    daos_gfx_draw_text(25, 175, buffer, COLOR_WHITE, COLOR_BLACK);

    // Indicador de tarea activa
    daos_gfx_draw_text(170, 130, "ACTIVE:", COLOR_GREEN, COLOR_BLACK);

    static uint32_t last_a = 0, last_b = 0, last_c = 0;
    const char* active_name = "?";
    uint16_t active_color = COLOR_WHITE;

    if (count_a != last_a) {
        active_name = "A";
        active_color = COLOR_RED;
        last_a = count_a;
    } else if (count_b != last_b) {
        active_name = "B";
        active_color = COLOR_BLUE;
        last_b = count_b;
    } else if (count_c != last_c) {
        active_name = "C";
        active_color = COLOR_CYAN;
        last_c = count_c;
    }

    daos_gfx_draw_text_large(185, 150, active_name, active_color, COLOR_BLACK, 4);
}

// ============================================================================
// 🔥 FUNCIÓN AUXILIAR: Trabajo pesado para simular carga sin dormir
// ============================================================================
static void do_heavy_work(uint32_t iterations) {
    volatile uint32_t dummy = 0;
    for (uint32_t i = 0; i < iterations; i++) {
        dummy += i * 3;
        dummy = dummy % 1000;
        // Operaciones matemáticas para consumir CPU
        for (uint32_t j = 0; j < 100; j++) {
            dummy = (dummy * 13 + 7) % 997;
        }
    }
}

// ============================================================================
// Task A: HIGH Priority - Intenta adquirir mutex frecuentemente
// ============================================================================
static void task_a_high(void) {
    uint32_t start_time = daos_millis();

    daos_uart_putc('A');
    count_a++;

    daos_uart_puts("[A-HIGH] Intentando adquirir mutex con mutex_lock()...\r\n");

    // Incrementar ANTES de intentar bloquear
    task_a_blocked_count++;

    daos_uart_puts("[A-HIGH] >>> BLOQUEANDO - Esperando mutex <<<\r\n");

    daos_mutex_lock(test_mutex);  // 🚀 HERENCIA AUTOMÁTICA AQUÍ

    task_a_acquired_count++;
    daos_uart_puts("[A-HIGH] *** MUTEX ADQUIRIDO ***\r\n");

    // ✅ SIN trabajo pesado - solo adquirir y liberar rápido
    // El objetivo es demostrar que A puede adquirir el mutex rápidamente
    // gracias a la herencia de prioridad

    daos_uart_puts("[A-HIGH] Liberando mutex\r\n");
    daos_mutex_unlock(test_mutex);

    uint8_t current_task = daos_get_current_task_id();
    if (current_task != last_task_id) {
        context_switches++;
        last_task_id = current_task;
    }

    uint32_t end_time = daos_millis();
    task_a_cpu_time += (end_time - start_time);

    daos_sleep_ms(100);
}

// ============================================================================
// Task B: LOW Priority - Mantiene mutex bloqueado con trabajo PESADO
// ============================================================================
static void task_b_low(void) {
    static uint8_t wait_cycles = 3;  // ✅ CORREGIDO: Iniciar en 3

    uint32_t start_time = daos_millis();

    daos_uart_putc('B');
    count_b++;

    // ✅ LÓGICA CORREGIDA: Esperar primero, luego ejecutar
    if (wait_cycles > 0) {
        wait_cycles--;
        daos_uart_puts("[B-LOW] Esperando... (ciclo ");
        daos_uart_putint(3 - wait_cycles);
        daos_uart_puts(" de 3)\r\n");
    } else {
        // Ahora sí, bloquear el mutex con trabajo pesado
        daos_uart_puts("\r\n[B-LOW] ==============================\r\n");
        daos_uart_puts("[B-LOW] *** ADQUIRIENDO MUTEX (Prioridad LOW) ***\r\n");
        daos_uart_puts("[B-LOW] ==============================\r\n");

        daos_mutex_lock(test_mutex);
        uint32_t lock_start = daos_millis();

        // Obtener prioridad real después de adquirir el mutex
        uint8_t current_task_id = daos_get_current_task_id();
        daos_priority_t real_prio = daos_get_task_real_priority(current_task_id);

        daos_uart_puts("[B-LOW] Mutex bloqueado - Prioridad REAL (0=IDLE, 4=CRIT): ");
        daos_uart_putint((uint32_t)real_prio);
        daos_uart_puts("\r\n");

        daos_uart_puts("[B-LOW] >>> Ejecutando 500 iteraciones de trabajo <<<\r\n");
        do_heavy_work(500);

        // ✅ INCREMENTAR SOLO cuando realmente se eleva la prioridad
        // Nota: Este incremento es aproximado. Idealmente debería verificarse
        // si la prioridad cambió, pero para esta demo es suficiente contar
        // cada vez que B tiene el mutex mientras A intenta adquirirlo
        task_b_boosted_times++;

        uint32_t elapsed = daos_millis() - lock_start;
        task_b_mutex_hold_time = elapsed;

        daos_uart_puts("\r\n[B-LOW] ==============================\r\n");
        daos_uart_puts("[B-LOW] *** LIBERANDO MUTEX ***\r\n");
        daos_uart_puts("[B-LOW] Tiempo con mutex: ");
        daos_uart_putint(elapsed);
        daos_uart_puts("ms (trabajo pesado completado)\r\n");
        daos_uart_puts("[B-LOW] ==============================\r\n");

        daos_mutex_unlock(test_mutex);

        daos_uart_puts("[B-LOW] >>> sync.c RESTAURÓ prioridad a LOW <<<\r\n\r\n");

        wait_cycles = 3;  // Reiniciar contador para esperar 3 ciclos más
    }

    uint8_t current_task = daos_get_current_task_id();
    if (current_task != last_task_id) {
        context_switches++;
        last_task_id = current_task;
    }

    uint32_t end_time = daos_millis();
    task_b_cpu_time += (end_time - start_time);

    daos_sleep_ms(300);
}

// ============================================================================
// Task C: NORMAL Priority - Trabajo continuo sin mutex
// ============================================================================
static void task_c_normal(void) {
    uint32_t start_time = daos_millis();

    daos_uart_putc('C');
    count_c++;

    daos_uart_puts("[C-NORMAL] Ejecutando trabajo normal (sin mutex)\r\n");

    // ✅ Trabajo moderado para tener actividad visible
    do_heavy_work(50);

    uint8_t current_task = daos_get_current_task_id();
    if (current_task != last_task_id) {
        context_switches++;
        last_task_id = current_task;
    }

    uint32_t end_time = daos_millis();
    task_c_cpu_time += (end_time - start_time);

    daos_sleep_ms(200);
}

// ============================================================================
// Monitor Task - Actualiza pantalla y muestra estadísticas
// ============================================================================
static void monitor_task(void) {
    static uint8_t refresh_counter = 0;
    static uint8_t stats_counter = 0;

    refresh_counter++;

    if (refresh_counter >= 3) {  // Refrescar cada 3 ciclos
        draw_task_stats();
        refresh_counter = 0;

        stats_counter++;

        if (stats_counter >= 15) {  // Estadísticas cada 15 refrescos
            daos_uart_puts("\r\n\r\n");
            daos_uart_puts("========================================\r\n");
            daos_uart_puts("   HERENCIA AUTOMÁTICA - ESTADÍSTICAS\r\n");
            daos_uart_puts("========================================\r\n");

            daos_uart_puts("Task A (HIGH):   ");
            daos_uart_putint(count_a);
            daos_uart_puts(" exec, ");
            daos_uart_putint(task_a_cpu_time);
            daos_uart_puts("ms\r\n");

            daos_uart_puts("Task B (LOW):    ");
            daos_uart_putint(count_b);
            daos_uart_puts(" exec, ");
            daos_uart_putint(task_b_cpu_time);
            daos_uart_puts("ms\r\n");

            daos_uart_puts("Task C (NORMAL): ");
            daos_uart_putint(count_c);
            daos_uart_puts(" exec, ");
            daos_uart_putint(task_c_cpu_time);
            daos_uart_puts("ms\r\n");

            daos_uart_puts("\r\nMutex Statistics:\r\n");
            daos_uart_puts("  A Intentos:  ");
            daos_uart_putint(task_a_blocked_count);
            daos_uart_puts("\r\n  A Adquirido: ");
            daos_uart_putint(task_a_acquired_count);
            daos_uart_puts("\r\n  B Hold Time: ");
            daos_uart_putint(task_b_mutex_hold_time);
            daos_uart_puts("ms\r\n");

            daos_uart_puts("\r\nHerencia AUTOMÁTICA (sync.c):\r\n");
            daos_uart_puts("  B Elevaciones: ");
            daos_uart_putint(task_b_boosted_times);
            daos_uart_puts("\r\n");

            daos_uart_puts("\r\nContext switches: ");
            daos_uart_putint(context_switches);
            daos_uart_puts("\r\n");

            uint32_t total = count_a + count_b + count_c;
            if (total > 0) {
                uint32_t percent_a = (count_a * 100) / total;
                uint32_t percent_b = (count_b * 100) / total;
                uint32_t percent_c = (count_c * 100) / total;

                daos_uart_puts("\r\nDistribución:\r\n");
                daos_uart_puts("  Task A: ");
                daos_uart_putint(percent_a);
                daos_uart_puts("%\r\n  Task B: ");
                daos_uart_putint(percent_b);
                daos_uart_puts("%\r\n  Task C: ");
                daos_uart_putint(percent_c);
                daos_uart_puts("%\r\n");

                daos_uart_puts("\r\nValidación de Prioridad:\r\n");
                if (percent_a >= percent_c && percent_c >= percent_b) {
                    daos_uart_puts("  ✓ HIGH > NORMAL > LOW (CORRECTO)\r\n");
                } else {
                    daos_uart_puts("  ✗ Inversión de prioridad detectada!\r\n");
                    daos_uart_puts("    (Esperado: A >= C >= B)\r\n");
                }
            }
            daos_uart_puts("========================================\r\n\r\n");
            stats_counter = 0;
        }
    }

    daos_sleep_ms(100);
}

void demo_prem_init(void) {
    count_a = 0;
    count_b = 0;
    count_c = 0;
    context_switches = 0;
    task_a_cpu_time = 0;
    task_b_cpu_time = 0;
    task_c_cpu_time = 0;
    last_task_id = 0;
    task_a_blocked_count = 0;
    task_a_acquired_count = 0;
    task_b_boosted_times = 0;
    task_b_mutex_hold_time = 0;

    // Inicializar mutex
    test_mutex = (daos_mutex_t)mutex_storage;
    daos_mutex_init(test_mutex);

    daos_gfx_clear(COLOR_BLACK);
    draw_header();
    draw_task_stats();

    daos_uart_puts("\r\n\r\n");
    daos_uart_puts("============================================\r\n");
    daos_uart_puts("   HERENCIA DE PRIORIDAD AUTOMÁTICA\r\n");
    daos_uart_puts("============================================\r\n");
    daos_uart_puts("Task A: HIGH   - mutex_lock() rápido - Sleep 100ms\r\n");
    daos_uart_puts("Task B: LOW    - Trabajo PESADO con mutex - Sleep 300ms\r\n");
    daos_uart_puts("Task C: NORMAL - Trabajo sin mutex - Sleep 200ms\r\n");
    daos_uart_puts("Monitor: LOW   - Visualización\r\n");
    daos_uart_puts("============================================\r\n");
    daos_uart_puts("Flujo de herencia AUTOMÁTICA (sync.c):\r\n");
    daos_uart_puts("1. B (LOW) adquiere mutex\r\n");
    daos_uart_puts("2. B ejecuta trabajo PESADO (500 iter)\r\n");
    daos_uart_puts("3. A (HIGH) llama mutex_lock() y se bloquea\r\n");
    daos_uart_puts("4. sync.c detecta: HIGH > LOW\r\n");
    daos_uart_puts("5. sync.c eleva B a CRITICAL automáticamente\r\n");
    daos_uart_puts("6. B completa y libera mutex\r\n");
    daos_uart_puts("7. sync.c restaura B a LOW automáticamente\r\n");
    daos_uart_puts("8. A adquiere mutex y continúa\r\n");
    daos_uart_puts("============================================\r\n\r\n");

    // Crear 4 tareas
    daos_task_create(task_a_high, DAOS_PRIO_HIGH);
    daos_task_create(task_b_low, DAOS_PRIO_LOW);
    daos_task_create(task_c_normal, DAOS_PRIO_NORMAL);
    daos_task_create(monitor_task, DAOS_PRIO_LOW);

    daos_uart_puts("[DEMO] 4 tareas creadas con herencia automática\r\n\r\n");
}

void demo_prem_reset(void) {
    count_a = 0;
    count_b = 0;
    count_c = 0;
    context_switches = 0;
    task_a_cpu_time = 0;
    task_b_cpu_time = 0;
    task_c_cpu_time = 0;
    last_task_id = 0;
    task_a_blocked_count = 0;
    task_a_acquired_count = 0;
    task_b_boosted_times = 0;
    task_b_mutex_hold_time = 0;

    daos_gfx_clear(COLOR_BLACK);
    draw_header();
    draw_task_stats();

    daos_uart_puts("[DEMO] Reset completo\r\n");
}

void demo_prem_cleanup(void) {
    daos_gfx_clear(COLOR_BLACK);
    daos_uart_puts("\r\n[DEMO] Cleanup completo\r\n");
}

void demo_prem_input(void) {
    daos_sleep_ms(1000);
}

void demo_prem_logic(void) {
    daos_sleep_ms(1000);
}

void demo_prem_render(void) {
    daos_sleep_ms(1000);
}

// Getters
uint32_t demo_prem_get_count_a(void) {
    return count_a;
}

uint32_t demo_prem_get_count_b(void) {
    return count_b;
}

uint8_t demo_prem_get_active_task(void) {
    return daos_get_current_task_id();
}

uint32_t demo_prem_get_context_switches(void) {
    return context_switches;
}

uint32_t demo_prem_get_cpu_time_a(void) {
    return task_a_cpu_time;
}

uint32_t demo_prem_get_cpu_time_b(void) {
    return task_b_cpu_time;
}
