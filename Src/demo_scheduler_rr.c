/**
 * ============================================================================
 * DEMO_SCHEDULER_RR.C - Round-Robin con 4 Tareas y Prioridades
 * ============================================================================
 * Task A: CRITICAL (Prioridad Alta)
 * Task B: NORMAL (Prioridad Media)
 * Task C: NORMAL (Prioridad Media) - RR con Task B
 * Task D: LOW (Prioridad Baja)
 * ============================================================================
 */

#include "demo_scheduler_rr.h"
#include "api.h"

static volatile uint32_t count_a = 0;  // CRITICAL
static volatile uint32_t count_b = 0;  // NORMAL
static volatile uint32_t count_c = 0;  // NORMAL
static volatile uint32_t count_d = 0;  // LOW
static volatile uint32_t context_switches = 0;
static volatile uint32_t task_a_cpu_time = 0;
static volatile uint32_t task_b_cpu_time = 0;
static volatile uint32_t task_c_cpu_time = 0;
static volatile uint32_t task_d_cpu_time = 0;
static volatile uint8_t last_task_id = 0;

#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_YELLOW  0xFFE0
#define COLOR_CYAN    0x07FF
#define COLOR_MAGENTA 0xF81F
#define COLOR_GREEN   0x07E0
#define COLOR_RED     0xF800
#define COLOR_BLUE    0x001F
#define COLOR_ORANGE  0xFD20

static void uint32_to_str(uint32_t num, char* buf) {
    buf[0] = '0' + (num / 10000) % 10;
    buf[1] = '0' + (num / 1000) % 10;
    buf[2] = '0' + (num / 100) % 10;
    buf[3] = '0' + (num / 10) % 10;
    buf[4] = '0' + num % 10;
    buf[5] = '\0';
}

static void draw_header(void) {
    daos_gfx_draw_text_large(20, 5, "PRIORITY SCHEDULER",
                             COLOR_YELLOW, COLOR_BLACK, 2);
}

static void draw_task_stats(void) {
    char buffer[16];
    uint8_t tid = daos_get_current_task_id();

    // Task A - CRITICAL (Alta)
    daos_gfx_draw_text_large(10, 35, "A:CRIT", COLOR_RED, COLOR_BLACK, 2);
    uint32_to_str(count_a, buffer);
    daos_gfx_draw_text_large(95, 35, buffer, COLOR_WHITE, COLOR_BLACK, 2);

    // Task B - NORMAL (Media)
    daos_gfx_draw_text_large(10, 65, "B:NORM", COLOR_CYAN, COLOR_BLACK, 2);
    uint32_to_str(count_b, buffer);
    daos_gfx_draw_text_large(95, 65, buffer, COLOR_WHITE, COLOR_BLACK, 2);

    // Task C - NORMAL (Media)
    daos_gfx_draw_text_large(10, 95, "C:NORM", COLOR_MAGENTA, COLOR_BLACK, 2);
    uint32_to_str(count_c, buffer);
    daos_gfx_draw_text_large(95, 95, buffer, COLOR_WHITE, COLOR_BLACK, 2);

    // Task D - LOW (Baja)
    daos_gfx_draw_text_large(10, 125, "D:LOW", COLOR_BLUE, COLOR_BLACK, 2);
    uint32_to_str(count_d, buffer);
    daos_gfx_draw_text_large(95, 125, buffer, COLOR_WHITE, COLOR_BLACK, 2);

    // Active Task Indicator
    daos_gfx_draw_text(180, 40, "ACTIVE:", COLOR_GREEN, COLOR_BLACK);

    uint16_t active_color = COLOR_WHITE;
    const char* active_name = "?";

    // Identificar tarea activa (aproximación por contadores)
    static uint32_t last_a = 0, last_b = 0, last_c = 0, last_d = 0;

    if (count_a != last_a) {
        active_color = COLOR_RED;
        active_name = "A";
        last_a = count_a;
    } else if (count_b != last_b) {
        active_color = COLOR_CYAN;
        active_name = "B";
        last_b = count_b;
    } else if (count_c != last_c) {
        active_color = COLOR_MAGENTA;
        active_name = "C";
        last_c = count_c;
    } else if (count_d != last_d) {
        active_color = COLOR_BLUE;
        active_name = "D";
        last_d = count_d;
    }

    daos_gfx_draw_text_large(185, 60, active_name, active_color, COLOR_BLACK, 4);

    // Context Switches
    daos_gfx_draw_text(180, 100, "CTX SW:", COLOR_YELLOW, COLOR_BLACK);
    uint32_to_str(context_switches, buffer);
    daos_gfx_draw_text(180, 112, buffer, COLOR_WHITE, COLOR_BLACK);

    // Priority Distribution
    daos_gfx_draw_text(10, 160, "PRIORITY DIST:", COLOR_ORANGE, COLOR_BLACK);
    uint32_t total = count_a + count_b + count_c + count_d;
    if (total > 0) {
        uint32_t percent_a = (count_a * 100) / total;
        uint32_t percent_b = (count_b * 100) / total;
        uint32_t percent_c = (count_c * 100) / total;
        uint32_t percent_d = (count_d * 100) / total;

        char pbuf[8];
        pbuf[0] = 'A'; pbuf[1] = ':';
        pbuf[2] = '0' + (percent_a / 10);
        pbuf[3] = '0' + (percent_a % 10);
        pbuf[4] = '%'; pbuf[5] = '\0';
        daos_gfx_draw_text(10, 175, pbuf, COLOR_RED, COLOR_BLACK);

        pbuf[0] = 'B'; pbuf[1] = ':';
        pbuf[2] = '0' + (percent_b / 10);
        pbuf[3] = '0' + (percent_b % 10);
        pbuf[4] = '%'; pbuf[5] = '\0';
        daos_gfx_draw_text(70, 175, pbuf, COLOR_CYAN, COLOR_BLACK);

        pbuf[0] = 'C'; pbuf[1] = ':';
        pbuf[2] = '0' + (percent_c / 10);
        pbuf[3] = '0' + (percent_c % 10);
        pbuf[4] = '%'; pbuf[5] = '\0';
        daos_gfx_draw_text(130, 175, pbuf, COLOR_MAGENTA, COLOR_BLACK);

        pbuf[0] = 'D'; pbuf[1] = ':';
        pbuf[2] = '0' + (percent_d / 10);
        pbuf[3] = '0' + (percent_d % 10);
        pbuf[4] = '%'; pbuf[5] = '\0';
        daos_gfx_draw_text(190, 175, pbuf, COLOR_BLUE, COLOR_BLACK);

        // Round-Robin check para tareas NORMAL (B y C)
        uint32_t normal_total = count_b + count_c;
        if (normal_total > 0) {
            uint32_t rr_diff = (count_b > count_c) ?
                              (count_b - count_c) : (count_c - count_b);
            uint32_t rr_percent = (rr_diff * 100) / normal_total;

            daos_gfx_draw_text(10, 195, "RR B/C:", COLOR_YELLOW, COLOR_BLACK);
            if (rr_percent <= 10) {
                daos_gfx_draw_text(70, 195, "EQUIT", COLOR_GREEN, COLOR_BLACK);
            } else {
                daos_gfx_draw_text(70, 195, "DESEQ", COLOR_RED, COLOR_BLACK);
            }
        }

        // Priority check
        daos_gfx_draw_text(10, 210, "PRIO:", COLOR_YELLOW, COLOR_BLACK);
        if (percent_a >= percent_b && percent_b >= percent_d) {
            daos_gfx_draw_text(50, 210, "OK", COLOR_GREEN, COLOR_BLACK);
        } else {
            daos_gfx_draw_text(50, 210, "ERR", COLOR_RED, COLOR_BLACK);
        }
    }
}

// Task A: CRITICAL Priority (Alta)
static void task_a_critical(void) {
    uint32_t start_time = daos_millis();

    daos_uart_putc('A');
    count_a++;

    uint8_t current_task = daos_get_current_task_id();
    if (current_task != last_task_id) {
        context_switches++;
        last_task_id = current_task;
    }

    uint32_t end_time = daos_millis();
    task_a_cpu_time += (end_time - start_time);

    daos_sleep_ms(46);
}

// Task B: NORMAL Priority (Media)
static void task_b_normal(void) {
    uint32_t start_time = daos_millis();

    daos_uart_putc('B');
    count_b++;

    uint8_t current_task = daos_get_current_task_id();
    if (current_task != last_task_id) {
        context_switches++;
        last_task_id = current_task;
    }

    uint32_t end_time = daos_millis();
    task_b_cpu_time += (end_time - start_time);

    daos_sleep_ms(50);
}

// Task C: NORMAL Priority (Media) - Round-Robin con B
static void task_c_normal(void) {
    uint32_t start_time = daos_millis();

    daos_uart_putc('C');
    count_c++;

    uint8_t current_task = daos_get_current_task_id();
    if (current_task != last_task_id) {
        context_switches++;
        last_task_id = current_task;
    }

    uint32_t end_time = daos_millis();
    task_c_cpu_time += (end_time - start_time);

    daos_sleep_ms(50);
}

// Task D: LOW Priority (Baja)
static void task_d_low(void) {
    uint32_t start_time = daos_millis();

    daos_uart_putc('D');
    count_d++;

    uint8_t current_task = daos_get_current_task_id();
    if (current_task != last_task_id) {
        context_switches++;
        last_task_id = current_task;
    }

    uint32_t end_time = daos_millis();
    task_d_cpu_time += (end_time - start_time);

    daos_sleep_ms(52);
}

static void monitor_task(void) {
    static uint8_t refresh_counter = 0;
    static uint8_t stats_counter = 0;

    refresh_counter++;

    if (refresh_counter >= 5) {
        draw_task_stats();
        refresh_counter = 0;

        stats_counter++;

        if (stats_counter >= 20) {
            daos_uart_puts("\r\n\r\n=== SCHEDULER STATS (4 TASKS) ===\r\n");

            daos_uart_puts("Task A (CRITICAL): ");
            daos_uart_putint(count_a);
            daos_uart_puts(" exec, ");
            daos_uart_putint(task_a_cpu_time);
            daos_uart_puts("ms\r\n");

            daos_uart_puts("Task B (NORMAL):   ");
            daos_uart_putint(count_b);
            daos_uart_puts(" exec, ");
            daos_uart_putint(task_b_cpu_time);
            daos_uart_puts("ms\r\n");

            daos_uart_puts("Task C (NORMAL):   ");
            daos_uart_putint(count_c);
            daos_uart_puts(" exec, ");
            daos_uart_putint(task_c_cpu_time);
            daos_uart_puts("ms\r\n");

            daos_uart_puts("Task D (LOW):      ");
            daos_uart_putint(count_d);
            daos_uart_puts(" exec, ");
            daos_uart_putint(task_d_cpu_time);
            daos_uart_puts("ms\r\n");

            daos_uart_puts("\r\nContext switches: ");
            daos_uart_putint(context_switches);
            daos_uart_puts("\r\n\r\n");

            uint32_t total = count_a + count_b + count_c + count_d;
            if (total > 0) {
                uint32_t percent_a = (count_a * 100) / total;
                uint32_t percent_b = (count_b * 100) / total;
                uint32_t percent_c = (count_c * 100) / total;
                uint32_t percent_d = (count_d * 100) / total;

                daos_uart_puts("Distribution:\r\n");
                daos_uart_puts("  Task A: ");
                daos_uart_putint(percent_a);
                daos_uart_puts("%\r\n  Task B: ");
                daos_uart_putint(percent_b);
                daos_uart_puts("%\r\n  Task C: ");
                daos_uart_putint(percent_c);
                daos_uart_puts("%\r\n  Task D: ");
                daos_uart_putint(percent_d);
                daos_uart_puts("%\r\n\r\n");

                // Verificar Round-Robin entre B y C
                uint32_t normal_total = count_b + count_c;
                if (normal_total > 0) {
                    uint32_t rr_b = (count_b * 100) / normal_total;
                    uint32_t rr_c = (count_c * 100) / normal_total;

                    daos_uart_puts("Round-Robin (NORMAL):\r\n");
                    daos_uart_puts("  B: ");
                    daos_uart_putint(rr_b);
                    daos_uart_puts("%  C: ");
                    daos_uart_putint(rr_c);
                    daos_uart_puts("%\r\n");

                    if (rr_b >= 45 && rr_b <= 55) {
                        daos_uart_puts("  ✓ EQUITATIVO\r\n");
                    } else {
                        daos_uart_puts("  ✗ DESBALANCEADO\r\n");
                    }
                }

                // Verificar jerarquía de prioridades
                daos_uart_puts("\r\nPriority Order:\r\n");
                if (percent_a >= percent_b && percent_b >= percent_d) {
                    daos_uart_puts("  ✓ CRITICAL > NORMAL > LOW\r\n");
                } else {
                    daos_uart_puts("  ✗ Priority order violated!\r\n");
                }
            }
            daos_uart_puts("==================================\r\n\r\n");
            stats_counter = 0;
        }
    }

    daos_sleep_ms(100);
}

void demo_scheduler_rr_init(void) {
    count_a = 0;
    count_b = 0;
    count_c = 0;
    count_d = 0;
    context_switches = 0;
    task_a_cpu_time = 0;
    task_b_cpu_time = 0;
    task_c_cpu_time = 0;
    task_d_cpu_time = 0;
    last_task_id = 0;

    daos_gfx_clear(COLOR_BLACK);
    draw_header();
    draw_task_stats();

    daos_uart_puts("\r\n\r\n");
    daos_uart_puts("====================================\r\n");
    daos_uart_puts("   PRIORITY SCHEDULER DEMO\r\n");
    daos_uart_puts("====================================\r\n");
    daos_uart_puts("Task A: CRITICAL (High Priority)\r\n");
    daos_uart_puts("Task B: NORMAL   (Medium Priority)\r\n");
    daos_uart_puts("Task C: NORMAL   (Medium Priority)\r\n");
    daos_uart_puts("Task D: LOW      (Low Priority)\r\n");
    daos_uart_puts("Monitor: Visualization (LOW)\r\n");
    daos_uart_puts("====================================\r\n");
    daos_uart_puts("Expected behavior:\r\n");
    daos_uart_puts("- A executes most (CRITICAL)\r\n");
    daos_uart_puts("- B & C share equally (RR)\r\n");
    daos_uart_puts("- D executes least (LOW)\r\n");
    daos_uart_puts("====================================\r\n\r\n");

    // Crear 4 tareas con diferentes prioridades
    daos_task_create(task_a_critical, DAOS_PRIO_CRITICAL);
    daos_task_create(task_b_normal, DAOS_PRIO_NORMAL);
    daos_task_create(task_c_normal, DAOS_PRIO_NORMAL);
    daos_task_create(task_d_low, DAOS_PRIO_LOW);
    daos_task_create(monitor_task, DAOS_PRIO_LOW);

    daos_uart_puts("[DEMO] 5 tareas creadas:\r\n");
    daos_uart_puts("  - task_a_critical (CRITICAL)\r\n");
    daos_uart_puts("  - task_b_normal (NORMAL)\r\n");
    daos_uart_puts("  - task_c_normal (NORMAL)\r\n");
    daos_uart_puts("  - task_d_low (LOW)\r\n");
    daos_uart_puts("  - monitor_task (LOW)\r\n\r\n");
}

void demo_scheduler_rr_reset(void) {
    count_a = 0;
    count_b = 0;
    count_c = 0;
    count_d = 0;
    context_switches = 0;
    task_a_cpu_time = 0;
    task_b_cpu_time = 0;
    task_c_cpu_time = 0;
    task_d_cpu_time = 0;
    last_task_id = 0;

    daos_gfx_clear(COLOR_BLACK);
    draw_header();
    draw_task_stats();

    daos_uart_puts("[DEMO] Reset complete\r\n");
}

void demo_scheduler_rr_cleanup(void) {
    daos_gfx_clear(COLOR_BLACK);
    count_a = 0;
    count_b = 0;
    count_c = 0;
    count_d = 0;
    context_switches = 0;
    task_a_cpu_time = 0;
    task_b_cpu_time = 0;
    task_c_cpu_time = 0;
    task_d_cpu_time = 0;
    last_task_id = 0;

    daos_uart_puts("\r\n[DEMO] Cleanup complete\r\n");
}

void demo_scheduler_rr_input(void) {
    daos_sleep_ms(1000);
}

void demo_scheduler_rr_logic(void) {
    daos_sleep_ms(1000);
}

void demo_scheduler_rr_render(void) {
    daos_sleep_ms(1000);
}

// Getters (para compatibilidad)
uint32_t demo_rr_get_count_a(void) {
    return count_a;
}

uint32_t demo_rr_get_count_b(void) {
    return count_b;
}

uint8_t demo_rr_get_active_task(void) {
    return daos_get_current_task_id();
}

uint32_t demo_rr_get_context_switches(void) {
    return context_switches;
}

uint32_t demo_rr_get_cpu_time_a(void) {
    return task_a_cpu_time;
}

uint32_t demo_rr_get_cpu_time_b(void) {
    return task_b_cpu_time;
}

// Nuevos getters para Task C y D
uint32_t demo_rr_get_count_c(void) {
    return count_c;
}

uint32_t demo_rr_get_count_d(void) {
    return count_d;
}

uint32_t demo_rr_get_cpu_time_c(void) {
    return task_c_cpu_time;
}

uint32_t demo_rr_get_cpu_time_d(void) {
    return task_d_cpu_time;
}
