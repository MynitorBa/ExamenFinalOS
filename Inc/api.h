#ifndef DAOS_API_H // Guarda de inclusión para evitar inclusiones dobles
#define DAOS_API_H

#pragma once // Directiva alternativa de guarda (dependiente del compilador)
#include <stdint.h> // Incluye tipos de enteros fijos
#include <stddef.h> // Incluye la definición de size_t y NULL

#ifdef __cplusplus
extern "C" { // Bloque para permitir la invocación desde C++
#endif

// ========================================================================
// SISTEMA DE TAREAS
// ========================================================================

/** Enumeración para las prioridades de las tareas. */
typedef enum {
    DAOS_PRIO_IDLE     = 0, // Prioridad de inactividad
    DAOS_PRIO_LOW      = 1, // Prioridad baja
    DAOS_PRIO_NORMAL   = 2, // Prioridad normal (por defecto)
    DAOS_PRIO_HIGH     = 3, // Prioridad alta
    DAOS_PRIO_CRITICAL = 4  // Prioridad crítica
} daos_priority_t;

/**
 * Crea una nueva tarea.
 * @param entry Función de entrada de la tarea.
 * @param prio Prioridad inicial de la tarea.
 * @return ID de la tarea creada o un error (< 0).
 */
int daos_task_create(void (*entry)(void), daos_priority_t prio);

/**
 * Suspende la tarea actual por un número de milisegundos.
 * @param ms Milisegundos a dormir.
 */
void daos_sleep_ms(uint32_t ms);

/**
 * Cede el control de la CPU a otra tarea de igual o menor prioridad.
 */
void daos_task_yield(void);

/**
 * Termina la tarea actual.
 */
void daos_task_exit(void);

/**
 * Establece la prioridad de una tarea específica.
 * @param task_id ID de la tarea.
 * @param priority Nueva prioridad.
 */
void daos_task_set_priority(uint8_t task_id, daos_priority_t priority);

/**
 * Obtiene el tiempo transcurrido del sistema en milisegundos.
 * @return Tiempo en ms.
 */
uint32_t daos_millis(void);

/**
 * Obtiene el ID de la tarea actualmente en ejecución.
 * @return ID de la tarea.
 */
uint8_t daos_get_current_task_id(void);

/**
 * Obtiene la prioridad real (efectiva) de una tarea.
 * @param task_id ID de la tarea.
 * @return Prioridad.
 */
daos_priority_t daos_get_task_real_priority(uint8_t task_id);

/** Estructura con información de una tarea. */
typedef struct {
    uint8_t id;
    uint8_t state;
    uint8_t priority;
    uint32_t cpu_time;
} daos_task_info_t;

/**
 * Rellena una lista con información de las tareas.
 * @param list Puntero a la lista de estructuras.
 * @param max_tasks Tamaño máximo de la lista.
 * @return Número de tareas listadas.
 */
int daos_get_task_list(daos_task_info_t* list, int max_tasks);

// ========================================================================
// SINCRONIZACIÓN
// ========================================================================

/** Tipos opacos para Mutex y Semáforo. */
typedef void* daos_mutex_t;
typedef void* daos_sem_t;

// MUTEX
/** Inicializa un Mutex. */
void daos_mutex_init(daos_mutex_t m);
/** Bloquea el Mutex (bloqueante). */
void daos_mutex_lock(daos_mutex_t m);
/** Desbloquea el Mutex. */
void daos_mutex_unlock(daos_mutex_t m);
/** Intenta bloquear el Mutex sin esperar. @return 1 si bloqueó, 0 si falló. */
int daos_mutex_trylock(daos_mutex_t m);
/** Bloquea el Mutex y aplica herencia de prioridad. @return 1 si bloqueó, 0 si falló. */
int daos_mutex_lock_ex(daos_mutex_t m);
/** Obtiene el contador de herencia de prioridad del Mutex. */
uint32_t daos_mutex_get_inheritance_count(daos_mutex_t m);
/** Resetea el contador de herencia de prioridad. */
void daos_mutex_reset_inheritance_count(daos_mutex_t m);
/** Obtiene el ID de la tarea dueña del Mutex. */
uint8_t daos_mutex_get_owner(daos_mutex_t m);
/** Incrementa el contador de herencia de prioridad. */
void daos_mutex_increment_inheritance(daos_mutex_t m);

// 🔥 SEMÁFOROS CON HERENCIA DE PRIORIDAD
/** Inicializa un Semáforo. @param initial Conteo inicial. @param max Conteo máximo. */
void daos_sem_init(daos_sem_t s, int initial, int max);

/**
 * Intenta adquirir recurso del semáforo.
 * @return 1 si adquirió, 0 si debe reintentar (comportamiento no-bloqueante especial).
 */
int daos_sem_wait(daos_sem_t s);

/** Libera un recurso del semáforo (incrementa el contador). */
void daos_sem_post(daos_sem_t s);
/** Intenta adquirir recurso sin bloquear. @return 1 si adquirió, 0 si falló. */
int daos_sem_trywait(daos_sem_t s);

// 🔥 NUEVAS: Estadísticas de semáforos
/** Obtiene el contador de herencia de prioridad del Semáforo. */
uint32_t daos_sem_get_inheritance_count(daos_sem_t s);
/** Resetea el contador de herencia de prioridad del Semáforo. */
void daos_sem_reset_inheritance_count(daos_sem_t s);
/** Obtiene el ID del poseedor del Semáforo. */
int daos_sem_get_holder(daos_sem_t s);
/** Obtiene el conteo actual de recursos disponibles. */
int daos_sem_get_count(daos_sem_t s);

// ========================================================================
// SISTEMA DE ARCHIVOS
// ========================================================================

/** Modos de búsqueda para `daos_seek`. */
#define DAOS_SEEK_SET 0
#define DAOS_SEEK_CUR 1
#define DAOS_SEEK_END 2

/** Abre un archivo. @param path Ruta del archivo. @return Descriptor de archivo (fd) o error. */
int daos_open(const char* path);
/** Lee de un archivo. @param fd Descriptor. @param buf Buffer de destino. @param n Bytes a leer. @return Bytes leídos. */
int daos_read(int fd, void* buf, int n);
/** Escribe en un archivo. @param fd Descriptor. @param buf Buffer de origen. @param n Bytes a escribir. @return Bytes escritos. */
int daos_write(int fd, const void* buf, int n);
/** Cierra un archivo. */
int daos_close(int fd);
/** Mueve el puntero de lectura/escritura. */
int daos_seek(int fd, int offset, int whence);
/** Lista los archivos del directorio raíz. @param out Buffer de salida. @param max Tamaño. @return Número de archivos. */
int daos_listdir(char* out, int max);
/** Verifica si un archivo existe. @return 1 si existe, 0 si no. */
int daos_exists(const char* path);
/** Obtiene el tamaño de un archivo. @return Tamaño en bytes o error. */
int daos_get_file_size(const char* path);
/** Crea un nuevo archivo con datos iniciales. */
int daos_create_file(const char* name, const void* data, uint32_t size);
/** Elimina un archivo. */
int daos_delete_file(const char* name);
/** Añade datos al final de un archivo existente. */
int daos_append(const char* name, const void* data, uint32_t size);

// ========================================================================
// GRÁFICOS
// ========================================================================

/** Definiciones de resolución de pantalla. */
#define DAOS_SCREEN_WIDTH  320
#define DAOS_SCREEN_HEIGHT 240

/** Paleta de colores estándar (formato RGB565). */
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

/** Macro para convertir RGB 8-bits a RGB565. */
#define DAOS_RGB565(r,g,b) ((((r)>>3)<<11) | (((g)>>2)<<5) | ((b)>>3))

/** Inicializa el subsistema gráfico. */
void daos_gfx_init(void);
/** Limpia la pantalla con un color. */
void daos_gfx_clear(uint16_t color);
/** Rellena un rectángulo. */
void daos_gfx_fill_rect(int x, int y, int w, int h, uint16_t color);
/** Dibuja un píxel. */
void daos_gfx_draw_pixel(int x, int y, uint16_t color);
/** Dibuja texto con fondo. */
void daos_gfx_draw_text(int x, int y, const char* text, uint16_t color, uint16_t bg);
/** Dibuja texto a una escala mayor. */
void daos_gfx_draw_text_large(int x, int y, const char* text, uint16_t color, uint16_t bg, uint8_t scale);
/** Dibuja el contorno de un círculo. */
void daos_gfx_draw_circle(int x0, int y0, int radius, uint16_t color);
/** Dibuja un círculo relleno. */
void daos_gfx_draw_circle_filled(int x0, int y0, int radius, uint16_t color);
/** Copia un bitmap de 16 bits (blit). */
void daos_gfx_blit16(int x, int y, const uint16_t* src, int w, int h);
/** Dibuja el contorno de un rectángulo. */
void daos_gfx_draw_rect(int x, int y, int w, int h, uint16_t color);
/** Dibuja una línea horizontal. */
void daos_gfx_hline(int x, int y, int w, uint16_t color);
/** Dibuja una línea vertical. */
void daos_gfx_vline(int x, int y, int h, uint16_t color);

// ========================================================================
// AUDIO
// ========================================================================

/** Inicializa el subsistema de audio. */
void daos_audio_init(void);
/** Reproduce una melodía navideña pregrabada. */
void daos_audio_play_christmas(void);
/** Reproduce un tono de frecuencia y duración específicas. */
void daos_audio_play_tone(uint32_t frequency, uint32_t duration_ms);
/** Detiene cualquier reproducción de audio. */
void daos_audio_stop(void);

// ========================================================================
// BOTONES
// ========================================================================

/** Enumeración para identificar los botones. */
typedef enum {
    DAOS_BTN_A = 0,
    DAOS_BTN_B = 1,
    DAOS_BTN_C = 2,
    DAOS_BTN_D = 3
} daos_btn_t;

/**
 * Consulta el estado actual de un botón.
 * @return 1 si presionado, 0 si no.
 */
int daos_btn_poll(daos_btn_t btn);

/**
 * Obtiene el evento de botón más reciente (FIFO).
 * @return 1 si hay evento, 0 si no.
 */
int daos_btn_get_event(daos_btn_t* out_btn, int* out_pressed);

/**
 * Obtiene el conteo total de presiones detectadas.
 * @return Contador.
 */
uint32_t daos_btn_get_press_count(void);

/** Lee el estado del botón azul especial. */
int daos_btn_read_blue(void);

/** Verifica si el botón azul fue presionado (basado en eventos). */
int daos_btn_was_blue_pressed(void);

/** Actualiza el estado de todos los botones. */
void daos_btn_update_all(void);

/** Lee el estado de un botón por su índice interno. */
int daos_btn_read_by_index(int index);

/** Obtiene el conteo de presiones por índice interno. */
uint32_t daos_btn_get_count_by_index(int index);

/** Resetea todos los contadores de presión. */
void daos_btn_reset_all_counts(void);

/** Obtiene el nombre estático del botón (ej: "A"). */
const char* daos_btn_get_name(int index);

/** Obtiene la etiqueta del botón (ej: "Start"). */
const char* daos_btn_get_label(int index);

// ========================================================================
// UART / DEBUG
// ========================================================================

/** Envía una cadena a la UART (serial). */
void daos_uart_puts(const char* str);
/** Envía un solo carácter a la UART. */
void daos_uart_putc(char c);
/** Envía un entero sin signo a la UART. */
void daos_uart_putint(uint32_t num);
/** Envía un carácter de nueva línea a la UART. */
void daos_uart_newline(void);
/** Causa un error fatal en el sistema con un mensaje. */
void daos_panic(const char* msg) __attribute__((noreturn));

// ========================================================================
// LOADER
// ========================================================================

/**
 * Ejecuta una aplicación cargada.
 * @return 0 en éxito, < 0 en error.
 */
int daos_loader_exec(const char* app_name, daos_priority_t priority);

/** Lista las aplicaciones disponibles. */
void daos_loader_list_apps(void);

/** Obtiene el número total de aplicaciones. */
int daos_loader_get_app_count(void);

// ========================================================================
// SHELL
// ========================================================================

/** Inicializa el intérprete de comandos (Shell). */
void daos_shell_init(void);
/** Función de tarea para el Shell. */
void daos_shell_task(void);

// ========================================================================
// INFORMACIÓN DEL SISTEMA
// ========================================================================

/** Obtiene la cadena de versión del sistema operativo. */
const char* daos_get_version(void);
/** Obtiene la arquitectura de CPU. */
const char* daos_get_arch(void);

/** Estructura con información de uso de memoria. */
typedef struct {
    int total_files;
    int used_blocks;
    int free_blocks;
    int total_kb;
} daos_memory_info_t;

/** Rellena la estructura con la información de memoria. */
void daos_get_memory_info(daos_memory_info_t* info);

/** Obtiene el tiempo total de funcionamiento en segundos. */
uint32_t daos_get_uptime_seconds(void);

// ========================================================================
// INICIALIZACIÓN Y CONTROL
// ========================================================================

/** Inicializa todos los subsistemas del DAOS. */
void daos_init(void);
/** Inicia el planificador de tareas (no retorna). */
void daos_start(void) __attribute__((noreturn));
/** Retorna la ejecución al menú principal/loader. */
void daos_return_to_menu(void);
/** Realiza la limpieza del juego/aplicación actual. */
void daos_cleanup_current_game(void);

// ========================================================================
// SISTEMA DE ALEATORIEDAD
// ========================================================================

/** Inicializa el generador aleatorio. */
void daos_random_init(void);
/** Obtiene un número aleatorio de 32 bits. */
uint32_t daos_random(void);
/** Obtiene un aleatorio en el rango [min, max]. */
uint32_t daos_random_range(uint32_t min, uint32_t max);
/** Obtiene un byte aleatorio. */
uint8_t daos_random_byte(void);
/** Refresca la semilla del generador. */
void daos_random_reseed(void);

// ========================================================================
// SISTEMA DE BINARIOS
// ========================================================================

/** Número mágico para identificar archivos binarios DAOS. */
#define DAOS_BINARIO_MAGIC 0x44414F53
/** Versión actual del formato binario. */
#define DAOS_BINARIO_VERSION 1

/** Tipos de binarios. */
typedef enum {
    DAOS_BINARIO_TIPO_JUEGO = 1,
    DAOS_BINARIO_TIPO_APP = 2,
    DAOS_BINARIO_TIPO_SERVICIO = 3
} daos_binario_tipo_t;

/** Estructura del encabezado del binario. */
typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t tipo;
    char nombre[32];
    uint32_t entry_point;
    uint32_t size;
    uint32_t checksum;
} daos_binario_header_t;

/** Estructura de un binario ejecutable completo (con punteros a funciones). */
typedef struct {
    daos_binario_header_t header;
    void (*init)(void);
    void (*reset)(void);
    void (*input_task)(void);
    void (*logic_task)(void);
    void (*render_task)(void);
    void (*cleanup)(void);
    uint32_t (*get_state)(void);
} daos_binario_ejecutable_t;

/**
 * Carga un binario desde el sistema de archivos a la estructura.
 * @return 0 si cargó, < 0 si error.
 */
int daos_binario_cargar(const char *nombre, daos_binario_ejecutable_t *binario);

/**
 * Valida el encabezado y checksum de un binario.
 * @return 0 si es válido, < 0 si no.
 */
int daos_binario_validar(daos_binario_ejecutable_t *binario);

/** Ejecuta un binario cargado (crea tareas). */
int daos_binario_ejecutar(daos_binario_ejecutable_t *binario);

/** Detiene y limpia las tareas del binario. */
void daos_binario_detener(daos_binario_ejecutable_t *binario);

/** Calcula el checksum de un bloque de datos. */
uint32_t daos_binario_calcular_checksum(void *data, uint32_t size);

/**
 * Crea la estructura binaria en memoria a partir de funciones.
 * @return Puntero a la estructura o NULL si error.
 */
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

/** Función auxiliar interna para establecer la prioridad. */
void daos_set_task_priority(uint8_t task_id, daos_priority_t priority);

#ifdef __cplusplus
}
#endif

#endif // DAOS_API_H
