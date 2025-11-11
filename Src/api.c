#include "api.h"
#include "sched.h"
#include "sync.h"
#include "ramfs.h"
#include "uart.h"
#include "button.h"
#include "loader.h"
#include "shell.h"
#include "pantalla.h"
#include "buzzer.h"
#include "aleatorio.h"
#include <string.h>

// ========================================================================
// SISTEMA DE TAREAS
// ========================================================================

int daos_task_create(void (*entry)(void), daos_priority_t prio) {
    TaskPriority kernel_prio;
    switch(prio) {
        case DAOS_PRIO_IDLE:     kernel_prio = PRIO_IDLE; break;
        case DAOS_PRIO_LOW:      kernel_prio = PRIO_LOW; break;
        case DAOS_PRIO_NORMAL:   kernel_prio = PRIO_NORMAL; break;
        case DAOS_PRIO_HIGH:     kernel_prio = PRIO_HIGH; break;
        case DAOS_PRIO_CRITICAL: kernel_prio = PRIO_CRITICAL; break;
        default:                 kernel_prio = PRIO_NORMAL; break;
    }
    task_create(entry, kernel_prio);
    return 0;
}

void daos_sleep_ms(uint32_t ms) {
    task_delay(ms);
}

void daos_task_yield(void) {
    task_yield();
}

void daos_task_exit(void) {
    task_exit();
}

uint32_t daos_millis(void) {
    return millis();
}

uint8_t daos_get_current_task_id(void) {
    return get_current_task_id();
}

// 🔥 NUEVA FUNCIÓN: Obtiene la prioridad real (efectiva) de una tarea.
// La prioridad real puede ser mayor que la prioridad base debido a la Herencia de Prioridad.
daos_priority_t daos_get_task_real_priority(uint8_t task_id) {
    TaskPriority kernel_prio = get_task_priority(task_id);
    switch(kernel_prio) {
        case PRIO_IDLE:      return DAOS_PRIO_IDLE;
        case PRIO_LOW:       return DAOS_PRIO_LOW;
        case PRIO_NORMAL:    return DAOS_PRIO_NORMAL;
        case PRIO_HIGH:      return DAOS_PRIO_HIGH;
        case PRIO_CRITICAL:  return DAOS_PRIO_CRITICAL;
        default:             return DAOS_PRIO_NORMAL;
    }
}

int daos_get_task_list(daos_task_info_t* list, int max_tasks) {
    TaskInfo temp[16];
    int count = get_task_list(temp, max_tasks);
    for (int i = 0; i < count; i++) {
        list[i].id = temp[i].id;
        list[i].state = temp[i].state;
        list[i].priority = temp[i].priority;
        list[i].cpu_time = temp[i].cpu_time;
    }
    return count;
}

// ========================================================================
// SINCRONIZACIÓN
// ========================================================================

void daos_mutex_init(daos_mutex_t m) {
    if (m != NULL) mutex_init((mutex_t*)m);
}

void daos_mutex_lock(daos_mutex_t m) {
    if (m != NULL) mutex_lock((mutex_t*)m);
}

void daos_mutex_unlock(daos_mutex_t m) {
    if (m != NULL) mutex_unlock((mutex_t*)m);
}

int daos_mutex_trylock(daos_mutex_t m) {
    if (m != NULL) return mutex_trylock((mutex_t*)m);
    return 0;
}

int daos_mutex_lock_ex(daos_mutex_t m) {
    if (m != NULL) {
        return mutex_lock((mutex_t*)m);  // Llama al mutex_lock() simplificado
    }
    return 0;
}

void daos_sem_init(daos_sem_t s, int initial, int max) {
    if (s != NULL) sem_init((sem_t*)s, initial, max);
}

// ========================================================================
// SISTEMA DE ARCHIVOS
// ========================================================================

int daos_open(const char* path) {
    return ramfs_open(path, RAMFS_O_RDONLY);
}

int daos_read(int fd, void* buf, int n) {
    return ramfs_read(fd, buf, n);
}

int daos_write(int fd, const void* buf, int n) {
    return ramfs_write(fd, buf, n);
}

int daos_close(int fd) {
    return ramfs_close(fd);
}

int daos_seek(int fd, int offset, int whence) {
    return ramfs_seek(fd, offset, whence);
}

int daos_listdir(char* out, int max) {
    return ramfs_listdir(out, max);
}

int daos_exists(const char* path) {
    return ramfs_exists(path);
}

int daos_get_file_size(const char* path) {
    return ramfs_get_size(path);
}

int daos_create_file(const char* name, const void* data, uint32_t size) {
    return ramfs_create(name, data, size);
}

int daos_delete_file(const char* name) {
    return ramfs_delete(name);
}

int daos_append(const char* name, const void* data, uint32_t size) {
    return ramfs_append(name, data, size);
}

// ========================================================================
// GRÁFICOS
// ========================================================================

void daos_gfx_init(void) {
    pantalla_init();
}

void daos_gfx_clear(uint16_t color) {
    pantalla_clear(color);
}

void daos_gfx_fill_rect(int x, int y, int w, int h, uint16_t color) {
    pantalla_fill_rect((uint16_t)x, (uint16_t)y, (uint16_t)w, (uint16_t)h, color);
}

void daos_gfx_draw_pixel(int x, int y, uint16_t color) {
    pantalla_draw_pixel((uint16_t)x, (uint16_t)y, color);
}

void daos_gfx_draw_text(int x, int y, const char* text, uint16_t color, uint16_t bg) {
    pantalla_draw_string((uint16_t)x, (uint16_t)y, text, color, bg);
}

void daos_gfx_draw_text_large(int x, int y, const char* text, uint16_t color, uint16_t bg, uint8_t scale) {
    pantalla_draw_string_large((uint16_t)x, (uint16_t)y, text, color, bg, scale);
}

void daos_gfx_draw_circle(int x0, int y0, int radius, uint16_t color) {
    pantalla_draw_circle((uint16_t)x0, (uint16_t)y0, (uint16_t)radius, color);
}

void daos_gfx_draw_circle_filled(int x0, int y0, int radius, uint16_t color) {
    pantalla_draw_circle_filled((uint16_t)x0, (uint16_t)y0, (uint16_t)radius, color);
}

void daos_gfx_blit16(int x, int y, const uint16_t* src, int w, int h) {
    (void)x; (void)y; (void)src; (void)w; (void)h;
}

void daos_gfx_draw_rect(int x, int y, int w, int h, uint16_t color) {
    daos_gfx_hline(x, y, w, color);
    daos_gfx_hline(x, y + h - 1, w, color);
    daos_gfx_vline(x, y, h, color);
    daos_gfx_vline(x + w - 1, y, h, color);
}

void daos_gfx_hline(int x, int y, int w, uint16_t color) {
    daos_gfx_fill_rect(x, y, w, 1, color);
}

void daos_gfx_vline(int x, int y, int h, uint16_t color) {
    daos_gfx_fill_rect(x, y, 1, h, color);
}

// ========================================================================
// AUDIO
// ========================================================================

void daos_audio_init(void) {
    Buzzer_Init();
}

void daos_audio_play_christmas(void) {
    buzzer_play_christmas();
}

void daos_audio_play_tone(uint32_t frequency, uint32_t duration_ms) {
    buzzer_play_tone(frequency, duration_ms);
}

void daos_audio_stop(void) {
    buzzer_stop_all();
}

// ========================================================================
// BOTONES
// ========================================================================

int daos_btn_poll(daos_btn_t btn) {
    int index = -1;
    switch(btn) {
        case DAOS_BTN_A: index = 0; break;
        case DAOS_BTN_B: index = 3; break;
        case DAOS_BTN_C: index = 1; break;
        case DAOS_BTN_D: index = 2; break;
        default: return 0;
    }
    return button_read_index(index);
}

int daos_btn_get_event(daos_btn_t* out_btn, int* out_pressed) {
    (void)out_btn; (void)out_pressed;
    return 0;
}

uint32_t daos_btn_get_press_count(void) {
    static uint32_t count = 0;
    if (button_was_pressed()) count++;
    return count;
}

int daos_btn_read_blue(void) {
    return button_read();
}

int daos_btn_was_blue_pressed(void) {
    return button_was_pressed();
}

void daos_btn_update_all(void) {
    button_update();
    buttons_update();
}

int daos_btn_read_by_index(int index) {
    return button_read_index(index);
}

uint32_t daos_btn_get_count_by_index(int index) {
    return button_get_press_count_index(index);
}

void daos_btn_reset_all_counts(void) {
    buttons_reset_counts();
}

const char* daos_btn_get_name(int index) {
    return button_get_name(index);
}

const char* daos_btn_get_label(int index) {
    return button_get_label(index);
}

void daos_return_to_menu(void) {
    extern void sched_kill_all_tasks(void);
    daos_uart_puts("\r\n[SYSTEM] Killing tasks\r\n");
    sched_kill_all_tasks();
}

// ========================================================================
// UART / DEBUG
// ========================================================================

void daos_uart_puts(const char* str) {
    uart_puts(str);
}

void daos_uart_putc(char c) {
    uart_putc(c);
}

void daos_uart_putint(uint32_t num) {
    uart_putint(num);
}

void daos_uart_newline(void) {
    uart_newline();
}

void daos_panic(const char* msg) {
    uart_puts("\r\n\r\n=================================\r\n");
    uart_puts("     KERNEL PANIC!\r\n");
    uart_puts("=================================\r\n");
    uart_puts("Error: ");
    uart_puts(msg);
    uart_puts("\r\nSystem halted.\r\n");
    uart_puts("=================================\r\n");
    __asm volatile("cpsid i");
    while(1) __asm volatile("wfi");
}

// ========================================================================
// LOADER
// ========================================================================

int daos_loader_exec(const char* app_name, daos_priority_t priority) {
    TaskPriority kernel_prio;
    switch(priority) {
        case DAOS_PRIO_IDLE:     kernel_prio = PRIO_IDLE; break;
        case DAOS_PRIO_LOW:      kernel_prio = PRIO_LOW; break;
        case DAOS_PRIO_NORMAL:   kernel_prio = PRIO_NORMAL; break;
        case DAOS_PRIO_HIGH:     kernel_prio = PRIO_HIGH; break;
        case DAOS_PRIO_CRITICAL: kernel_prio = PRIO_CRITICAL; break;
        default:                 kernel_prio = PRIO_NORMAL; break;
    }
    return loader_exec(app_name, kernel_prio);
}

void daos_loader_list_apps(void) {
    loader_list_apps();
}

int daos_loader_get_app_count(void) {
    return loader_get_app_count();
}

// ========================================================================
// SHELL
// ========================================================================

void daos_shell_init(void) {
    shell_init();
}

void daos_shell_task(void) {
    shell_task();
}

// ========================================================================
// INFORMACIÓN DEL SISTEMA
// ========================================================================

const char* daos_get_version(void) {
    return "2.0.0";
}

const char* daos_get_arch(void) {
    return "ARM Cortex-M4 (STM32F446)";
}

void daos_get_memory_info(daos_memory_info_t* info) {
    int files, used, free;
    ramfs_stats(&files, &used, &free);
    info->total_files = files;
    info->used_blocks = used;
    info->free_blocks = free;
    info->total_kb = (used * 256) / 1024;
}

uint32_t daos_get_uptime_seconds(void) {
    return millis() / 1000;
}

// ========================================================================
// INICIALIZACIÓN Y CONTROL
// ========================================================================

void daos_init(void) {
    uart_init();
    button_init();
    buttons_init();
    ramfs_init();
    loader_init();
    daos_gfx_init();
    daos_audio_init();
    daos_random_init();
}

void daos_start(void) {
    sched_start();
    while(1) __asm volatile("wfi");
}

// ========================================================================
// SISTEMA DE ALEATORIEDAD
// ========================================================================

void daos_random_init(void) {
    aleatorio_init();
}

uint32_t daos_random(void) {
    return aleatorio_obtener();
}

uint32_t daos_random_range(uint32_t min, uint32_t max) {
    return aleatorio_rango(min, max);
}

uint8_t daos_random_byte(void) {
    return aleatorio_byte();
}

void daos_random_reseed(void) {
    aleatorio_refrescar_semilla();
}

// ========================================================================
// SISTEMA DE BINARIOS
// ========================================================================

// Calcular checksum simple
uint32_t daos_binario_calcular_checksum(void *data, uint32_t size) {
    uint32_t checksum = 0;
    uint8_t *ptr = (uint8_t *)data;
    for (uint32_t i = 0; i < size; i++) {
        checksum += ptr[i];
    }
    return checksum;
}

// Validar un binario
int daos_binario_validar(daos_binario_ejecutable_t *binario) {
    if (binario == NULL) {
        daos_uart_puts("[BINARIO] Error: binario NULL\r\n");
        return -1;
    }

    // Verificar magic number
    if (binario->header.magic != DAOS_BINARIO_MAGIC) {
        daos_uart_puts("[BINARIO] Error: Magic number inválido\r\n");
        return -2;
    }

    // Verificar versión
    if (binario->header.version != DAOS_BINARIO_VERSION) {
        daos_uart_puts("[BINARIO] Error: Versión incompatible\r\n");
        return -3;
    }

    // Verificar que tenga al menos init
    if (binario->init == NULL) {
        daos_uart_puts("[BINARIO] Error: No tiene función init\r\n");
        return -4;
    }

    daos_uart_puts("[BINARIO] Validación OK: ");
    daos_uart_puts(binario->header.nombre);
    daos_uart_puts("\r\n");

    return 0;
}

// Cargar un binario desde memoria o filesystem
int daos_binario_cargar(const char *nombre, daos_binario_ejecutable_t *binario) {
    daos_uart_puts("[BINARIO] Cargando: ");
    daos_uart_puts(nombre);
    daos_uart_puts("\r\n");

    // Aquí podrías leer desde el filesystem
    // Por ahora, el binario ya está en memoria

    // Validar el binario
    return daos_binario_validar(binario);
}

// Ejecutar un binario
int daos_binario_ejecutar(daos_binario_ejecutable_t *binario) {
    if (daos_binario_validar(binario) != 0) {
        return -1;
    }

    daos_uart_puts("[BINARIO] Ejecutando: ");
    daos_uart_puts(binario->header.nombre);
    daos_uart_puts("\r\n");

    // Llamar a la función de inicialización
    if (binario->init != NULL) {
        binario->init();
    }

    // Crear las tareas del binario
    if (binario->input_task != NULL) {
        daos_task_create(binario->input_task, DAOS_PRIO_HIGH);
        daos_uart_puts("[BINARIO] Tarea input creada\r\n");
    }

    if (binario->logic_task != NULL) {
        daos_task_create(binario->logic_task, DAOS_PRIO_HIGH);
        daos_uart_puts("[BINARIO] Tarea logic creada\r\n");
    }

    if (binario->render_task != NULL) {
        daos_task_create(binario->render_task, DAOS_PRIO_NORMAL);
        daos_uart_puts("[BINARIO] Tarea render creada\r\n");
    }

    daos_uart_puts("[BINARIO] Binario ejecutándose\r\n");

    return 0;
}

// Detener un binario
void daos_binario_detener(daos_binario_ejecutable_t *binario) {
    if (binario == NULL) return;

    daos_uart_puts("[BINARIO] Deteniendo: ");
    daos_uart_puts(binario->header.nombre);
    daos_uart_puts("\r\n");

    // Llamar a cleanup si existe
    if (binario->cleanup != NULL) {
        binario->cleanup();
    }

    // Aquí podrías eliminar las tareas si el scheduler lo permite
    // Por ahora solo notificamos
    daos_uart_puts("[BINARIO] Binario detenido\r\n");
}

// Crear binario desde funciones
daos_binario_ejecutable_t* daos_binario_crear(
    const char *nombre,
    daos_binario_tipo_t tipo,
    void (*init)(void),
    void (*reset)(void),
    void (*input_task)(void),
    void (*logic_task)(void),
    void (*render_task)(void),
    void (*cleanup)(void),
    uint32_t (*get_state)(void)
) {
    static daos_binario_ejecutable_t binario;

    // Configurar header
    binario.header.magic = DAOS_BINARIO_MAGIC;
    binario.header.version = DAOS_BINARIO_VERSION;
    binario.header.tipo = tipo;
    binario.header.entry_point = 0;
    binario.header.size = sizeof(daos_binario_ejecutable_t);

    // Copiar nombre
    uint32_t i;
    for (i = 0; i < 31 && nombre[i] != '\0'; i++) {
        binario.header.nombre[i] = nombre[i];
    }
    binario.header.nombre[i] = '\0';

    // Asignar funciones
    binario.init = init;
    binario.reset = reset;
    binario.input_task = input_task;
    binario.logic_task = logic_task;
    binario.render_task = render_task;
    binario.cleanup = cleanup;
    binario.get_state = get_state;

    // Calcular checksum
    binario.header.checksum = daos_binario_calcular_checksum(
        &binario,
        sizeof(daos_binario_ejecutable_t)
    );

    return &binario;
}

// ========================================================================
// ESTADÍSTICAS DE MUTEX
// ========================================================================

uint32_t daos_mutex_get_inheritance_count(daos_mutex_t m) {
    if (m != NULL) {
        return mutex_get_inheritance_count((mutex_t*)m);
    }
    return 0;
}

void daos_mutex_reset_inheritance_count(daos_mutex_t m) {
    if (m != NULL) {
        mutex_reset_inheritance_count((mutex_t*)m);
    }
}

void daos_task_set_priority(uint8_t task_id, daos_priority_t priority) {
    TaskPriority kernel_prio;
    switch(priority) {
        case DAOS_PRIO_IDLE:     kernel_prio = PRIO_IDLE; break;
        case DAOS_PRIO_LOW:      kernel_prio = PRIO_LOW; break;
        case DAOS_PRIO_NORMAL:   kernel_prio = PRIO_NORMAL; break;
        case DAOS_PRIO_HIGH:     kernel_prio = PRIO_HIGH; break;
        case DAOS_PRIO_CRITICAL: kernel_prio = PRIO_CRITICAL; break;
        default:                 kernel_prio = PRIO_NORMAL; break;
    }
    task_set_priority(task_id, kernel_prio);
}

// ========================================================================
// IMPLEMENTACIONES DE SEMÁFOROS
// ========================================================================

int daos_sem_wait(daos_sem_t s) {
    if (s != NULL) return sem_wait((sem_t*)s);
    return 0;
}

void daos_sem_post(daos_sem_t s) {
    if (s != NULL) sem_post((sem_t*)s);
}

int daos_sem_trywait(daos_sem_t s) {
    if (s != NULL) return sem_trywait((sem_t*)s);
    return 0;
}

uint32_t daos_sem_get_inheritance_count(daos_sem_t s) {
    if (s != NULL) return sem_get_inheritance_count((sem_t*)s);
    return 0;
}

void daos_sem_reset_inheritance_count(daos_sem_t s) {
    if (s != NULL) sem_reset_inheritance_count((sem_t*)s);
}

int daos_sem_get_holder(daos_sem_t s) {
    if (s != NULL) return sem_get_holder((sem_t*)s);
    return -1;
}

int daos_sem_get_count(daos_sem_t s) {
    if (s != NULL) return sem_get_count((sem_t*)s);
    return 0;
}

uint8_t daos_mutex_get_owner(daos_mutex_t m) {
    if (m != NULL) {
        mutex_t* mutex = (mutex_t*)m;
        return (uint8_t)mutex->owner_task_id;
    }
    return 0xFF;
}

void daos_mutex_increment_inheritance(daos_mutex_t m) {
    if (m != NULL) {
        mutex_t* mutex = (mutex_t*)m;
        mutex->inheritance_count++;
    }
}

void daos_set_task_priority(uint8_t task_id, daos_priority_t priority) {
    daos_task_set_priority(task_id, priority);  // Llamar a la función ya existente
}
