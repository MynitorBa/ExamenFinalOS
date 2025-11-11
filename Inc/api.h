#ifndef DAOS_API_H
#define DAOS_API_H

#pragma once
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ========================================================================
// SISTEMA DE TAREAS
// ========================================================================

typedef enum {
    DAOS_PRIO_IDLE     = 0,
    DAOS_PRIO_LOW      = 1,
    DAOS_PRIO_NORMAL   = 2,
    DAOS_PRIO_HIGH     = 3,
    DAOS_PRIO_CRITICAL = 4
} daos_priority_t;

int daos_task_create(void (*entry)(void), daos_priority_t prio);
void daos_sleep_ms(uint32_t ms);
void daos_task_yield(void);
void daos_task_exit(void);
void daos_task_set_priority(uint8_t task_id, daos_priority_t priority);

uint32_t daos_millis(void);
uint8_t daos_get_current_task_id(void);
daos_priority_t daos_get_task_real_priority(uint8_t task_id);

typedef struct {
    uint8_t id;
    uint8_t state;
    uint8_t priority;
    uint32_t cpu_time;
} daos_task_info_t;

int daos_get_task_list(daos_task_info_t* list, int max_tasks);

// ========================================================================
// SINCRONIZACIÓN
// ========================================================================

typedef void* daos_mutex_t;
typedef void* daos_sem_t;

// MUTEX
void daos_mutex_init(daos_mutex_t m);
void daos_mutex_lock(daos_mutex_t m);
void daos_mutex_unlock(daos_mutex_t m);
int daos_mutex_trylock(daos_mutex_t m);
int daos_mutex_lock_ex(daos_mutex_t m);
uint32_t daos_mutex_get_inheritance_count(daos_mutex_t m);
void daos_mutex_reset_inheritance_count(daos_mutex_t m);
uint8_t daos_mutex_get_owner(daos_mutex_t m);
void daos_mutex_increment_inheritance(daos_mutex_t m);

// 🔥 SEMÁFOROS CON HERENCIA DE PRIORIDAD
void daos_sem_init(daos_sem_t s, int initial, int max);

/**
 * daos_sem_wait() - Intenta adquirir recurso del semáforo
 *
 * COMPORTAMIENTO NO-BLOQUEANTE:
 * - Si hay recursos: Decrementa y retorna 1
 * - Si NO hay: Aplica herencia y retorna 0 (tarea debe terminar)
 *
 * Retorna: 1 si adquirió, 0 si debe reintentar
 */
int daos_sem_wait(daos_sem_t s);

void daos_sem_post(daos_sem_t s);
int daos_sem_trywait(daos_sem_t s);

// 🔥 NUEVAS: Estadísticas de semáforos
uint32_t daos_sem_get_inheritance_count(daos_sem_t s);
void daos_sem_reset_inheritance_count(daos_sem_t s);
int daos_sem_get_holder(daos_sem_t s);
int daos_sem_get_count(daos_sem_t s);

// ========================================================================
// SISTEMA DE ARCHIVOS
// ========================================================================

#define DAOS_SEEK_SET 0
#define DAOS_SEEK_CUR 1
#define DAOS_SEEK_END 2

int daos_open(const char* path);
int daos_read(int fd, void* buf, int n);
int daos_write(int fd, const void* buf, int n);
int daos_close(int fd);
int daos_seek(int fd, int offset, int whence);
int daos_listdir(char* out, int max);
int daos_exists(const char* path);
int daos_get_file_size(const char* path);
int daos_create_file(const char* name, const void* data, uint32_t size);
int daos_delete_file(const char* name);
int daos_append(const char* name, const void* data, uint32_t size);

// ========================================================================
// GRÁFICOS
// ========================================================================

#define DAOS_SCREEN_WIDTH  320
#define DAOS_SCREEN_HEIGHT 240

#define DAOS_COLOR_BLACK   0x0000
#define DAOS_COLOR_WHITE   0xFFFF
#define DAOS_COLOR_RED     0xF800
#define DAOS_COLOR_GREEN   0x07E0
#define DAOS_COLOR_BLUE    0x001F
#define DAOS_COLOR_YELLOW  0xFFE0
#define DAOS_COLOR_CYAN    0x07FF
#define DAOS_COLOR_MAGENTA 0xF81F
#define DAOS_COLOR_ORANGE  0xFD20
#define DAOS_COLOR_GRAY    0xAD55

#define DAOS_RGB565(r,g,b) ((((r)>>3)<<11) | (((g)>>2)<<5) | ((b)>>3))

void daos_gfx_init(void);
void daos_gfx_clear(uint16_t color);
void daos_gfx_fill_rect(int x, int y, int w, int h, uint16_t color);
void daos_gfx_draw_pixel(int x, int y, uint16_t color);
void daos_gfx_draw_text(int x, int y, const char* text, uint16_t color, uint16_t bg);
void daos_gfx_draw_text_large(int x, int y, const char* text, uint16_t color, uint16_t bg, uint8_t scale);
void daos_gfx_draw_circle(int x0, int y0, int radius, uint16_t color);
void daos_gfx_draw_circle_filled(int x0, int y0, int radius, uint16_t color);
void daos_gfx_blit16(int x, int y, const uint16_t* src, int w, int h);
void daos_gfx_draw_rect(int x, int y, int w, int h, uint16_t color);
void daos_gfx_hline(int x, int y, int w, uint16_t color);
void daos_gfx_vline(int x, int y, int h, uint16_t color);

// ========================================================================
// AUDIO
// ========================================================================

void daos_audio_init(void);
void daos_audio_play_christmas(void);
void daos_audio_play_tone(uint32_t frequency, uint32_t duration_ms);
void daos_audio_stop(void);

// ========================================================================
// BOTONES
// ========================================================================

typedef enum {
    DAOS_BTN_A = 0,
    DAOS_BTN_B = 1,
    DAOS_BTN_C = 2,
    DAOS_BTN_D = 3
} daos_btn_t;

int daos_btn_poll(daos_btn_t btn);
int daos_btn_get_event(daos_btn_t* out_btn, int* out_pressed);
uint32_t daos_btn_get_press_count(void);
int daos_btn_read_blue(void);
int daos_btn_was_blue_pressed(void);
void daos_btn_update_all(void);
int daos_btn_read_by_index(int index);
uint32_t daos_btn_get_count_by_index(int index);
void daos_btn_reset_all_counts(void);
const char* daos_btn_get_name(int index);
const char* daos_btn_get_label(int index);

// ========================================================================
// UART / DEBUG
// ========================================================================

void daos_uart_puts(const char* str);
void daos_uart_putc(char c);
void daos_uart_putint(uint32_t num);
void daos_uart_newline(void);
void daos_panic(const char* msg) __attribute__((noreturn));

// ========================================================================
// LOADER
// ========================================================================

int daos_loader_exec(const char* app_name, daos_priority_t priority);
void daos_loader_list_apps(void);
int daos_loader_get_app_count(void);

// ========================================================================
// SHELL
// ========================================================================

void daos_shell_init(void);
void daos_shell_task(void);

// ========================================================================
// INFORMACIÓN DEL SISTEMA
// ========================================================================

const char* daos_get_version(void);
const char* daos_get_arch(void);

typedef struct {
    int total_files;
    int used_blocks;
    int free_blocks;
    int total_kb;
} daos_memory_info_t;

void daos_get_memory_info(daos_memory_info_t* info);
uint32_t daos_get_uptime_seconds(void);

// ========================================================================
// INICIALIZACIÓN Y CONTROL
// ========================================================================

void daos_init(void);
void daos_start(void) __attribute__((noreturn));
void daos_return_to_menu(void);
void daos_cleanup_current_game(void);

// ========================================================================
// SISTEMA DE ALEATORIEDAD
// ========================================================================

void daos_random_init(void);
uint32_t daos_random(void);
uint32_t daos_random_range(uint32_t min, uint32_t max);
uint8_t daos_random_byte(void);
void daos_random_reseed(void);

// ========================================================================
// SISTEMA DE BINARIOS
// ========================================================================

#define DAOS_BINARIO_MAGIC 0x44414F53  // "DAOS" en hex
#define DAOS_BINARIO_VERSION 1

// Tipos de binarios
typedef enum {
    DAOS_BINARIO_TIPO_JUEGO = 1,
    DAOS_BINARIO_TIPO_APP = 2,
    DAOS_BINARIO_TIPO_SERVICIO = 3
} daos_binario_tipo_t;

// Estructura del header del binario
typedef struct {
    uint32_t magic;              // Número mágico DAOS
    uint32_t version;            // Versión del formato
    uint32_t tipo;               // Tipo de binario
    char nombre[32];             // Nombre del binario
    uint32_t entry_point;        // Offset al punto de entrada
    uint32_t size;               // Tamaño total del binario
    uint32_t checksum;           // Checksum simple
} daos_binario_header_t;

// Estructura de un binario ejecutable completo
typedef struct {
    daos_binario_header_t header;
    void (*init)(void);          // Función de inicialización
    void (*reset)(void);         // Función de reset
    void (*input_task)(void);    // Tarea de input
    void (*logic_task)(void);    // Tarea de lógica
    void (*render_task)(void);   // Tarea de renderizado
    void (*cleanup)(void);       // Función de limpieza
    uint32_t (*get_state)(void); // Obtener estado
} daos_binario_ejecutable_t;

// API del cargador de binarios
int daos_binario_cargar(const char *nombre, daos_binario_ejecutable_t *binario);
int daos_binario_validar(daos_binario_ejecutable_t *binario);
int daos_binario_ejecutar(daos_binario_ejecutable_t *binario);
void daos_binario_detener(daos_binario_ejecutable_t *binario);
uint32_t daos_binario_calcular_checksum(void *data, uint32_t size);

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
);

// Función auxiliar para establecer prioridad interna
void daos_set_task_priority(uint8_t task_id, daos_priority_t priority);

#ifdef __cplusplus
}
#endif

#endif // DAOS_API_H
