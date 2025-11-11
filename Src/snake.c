/**
 * ============================================================================
 * SNAKE.C - Juego Snake (Movimiento 1x1 Corregido + Retraso Inicial + PAUSA)
 * ============================================================================
 * Correcciones: Inicialización de longitud a 1 (evita salto 2x2),
 * Renderizado correcto de Cabeza (Blanca) y Cuerpo (Verde).
 * RETRASO INICIAL: El snake no se mueve hasta 5 segundos después del inicio.
 * PAUSA: Botón D7 pausa/reanuda el juego con espera de 5 segundos al reanudar.
 * FIX: Limpieza correcta del mensaje de pausa incluyendo el borde blanco.
 * ============================================================================
 */

#include "snake.h"
#include "api.h"
#include "sync.h"
#include <string.h>

// --- MODIFICACIONES DE DIMENSIONES DEL MAPA ---
#define BOARD_WIDTH 40
#define BOARD_HEIGHT 25
#define PIXEL_SIZE 6

// CÁLCULO DE POSICIÓN DE CENTRADO EN PANTALLA (Asumiendo 320x240)
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define GAME_AREA_WIDTH (BOARD_WIDTH * PIXEL_SIZE)
#define GAME_AREA_HEIGHT (BOARD_HEIGHT * PIXEL_SIZE)
#define X_OFFSET ((SCREEN_WIDTH - GAME_AREA_WIDTH) / 2)
#define Y_OFFSET ((SCREEN_HEIGHT - GAME_AREA_HEIGHT) / 2)


// Estructura para representar una posición
typedef struct {
    uint8_t x;
    uint8_t y;
} Position;

// Cuerpo del gusano (array de posiciones)
#define MAX_SNAKE_LENGTH 1000
static Position snake_body[MAX_SNAKE_LENGTH];
static uint8_t snake_length = 1;

// Dirección actual
static snake_direction_t current_direction = SNAKE_RIGHT;

// Comida
static Position food_pos;
static uint8_t food_exists = 0;

// Estado del juego
static snake_state_t game_state = SNAKE_RUNNING;
static uint32_t score = 0;

// NUEVO: Control de tiempo de inicio
static uint32_t game_start_time = 0;
static uint8_t game_started = 0;

// NUEVO: Control de pausa
static uint8_t game_paused = 0;
static uint32_t pause_resume_time = 0;
static uint8_t waiting_to_resume = 0;
static uint8_t pause_message_cleared = 0;

// Colores
#define COLOR_SNAKE DAOS_COLOR_GREEN
#define COLOR_HEAD DAOS_COLOR_WHITE
#define COLOR_FOOD DAOS_COLOR_RED
#define COLOR_BG DAOS_COLOR_BLACK
#define COLOR_GRID DAOS_COLOR_GRAY

/* ============================================================ */
/* MUTEXES PARA SINCRONIZACIÓN                         */
/* ============================================================ */

mutex_t snake_sync_mutex;

/* ============================================================ */
/* FUNCIONES AUXILIARES                                */
/* ============================================================ */

static inline uint8_t random_range(uint8_t min, uint8_t max) {
    return (uint8_t)daos_random_range(min, max);
}

static uint8_t is_position_occupied(uint8_t x, uint8_t y) {
    uint8_t result = 0;

    mutex_lock(&snake_sync_mutex);
    for (uint8_t i = 0; i < snake_length; i++) {
        if (snake_body[i].x == x && snake_body[i].y == y) {
            result = 1;
            break;
        }
    }
    mutex_unlock(&snake_sync_mutex);
    return result;
}

static void place_food(void) {
    uint8_t found = 0;
    uint8_t attempts = 0;

    while (!found && attempts < 100) {
        uint8_t x = random_range(0, BOARD_WIDTH - 1);
        uint8_t y = random_range(0, BOARD_HEIGHT - 1);

        if (!is_position_occupied(x, y)) {
            mutex_lock(&snake_sync_mutex);
            food_pos.x = x;
            food_pos.y = y;
            food_exists = 1;
            mutex_unlock(&snake_sync_mutex);
            found = 1;
        }
        attempts++;
    }
}

static void draw_game_pixel(uint8_t x, uint8_t y, uint16_t color) {
    int screen_x = X_OFFSET + x * PIXEL_SIZE;
    int screen_y = Y_OFFSET + y * PIXEL_SIZE;

    daos_gfx_fill_rect(screen_x, screen_y, PIXEL_SIZE, PIXEL_SIZE, color);
}

/* ============================================================ */
/* TAREAS DEL JUEGO                                    */
/* ============================================================ */

void snake_input_task(void) {
    static uint32_t last_count_left = 0;
    static uint32_t last_count_right = 0;
    static uint32_t last_count_pause = 0;
    static uint8_t initialized = 0;

    if (!initialized) {
        last_count_left = daos_btn_get_count_by_index(0);
        last_count_right = daos_btn_get_count_by_index(1);
        last_count_pause = daos_btn_get_count_by_index(6); // D7 es índice 6
        initialized = 1;
        daos_sleep_ms(50);
        return;
    }

    mutex_lock(&snake_sync_mutex);
    snake_state_t current_state = game_state;
    uint8_t paused = game_paused;
    uint8_t waiting = waiting_to_resume;
    mutex_unlock(&snake_sync_mutex);

    if (current_state == SNAKE_GAME_OVER) {
        daos_sleep_ms(100);
        return;
    }

    // D7: Pausa/Reanuda
    uint32_t current_count_pause = daos_btn_get_count_by_index(6);
    if (current_count_pause != last_count_pause) {
        mutex_lock(&snake_sync_mutex);

        // Solo permitir pausar/reanudar si no está esperando
        if (!waiting_to_resume) {
            if (!game_paused) {
                // Pausar el juego
                game_paused = 1;
                pause_message_cleared = 0;
            } else {
                // Quitar pausa e iniciar espera de 5 segundos
                game_paused = 0;
                waiting_to_resume = 1;
                pause_resume_time = daos_millis();
                pause_message_cleared = 0;
            }
        }

        mutex_unlock(&snake_sync_mutex);
        last_count_pause = current_count_pause;
    }

    // Si está esperando reanudar, IGNORAR todos los inputs de movimiento
    if (waiting) {
        daos_sleep_ms(20);
        return;
    }

    // Si está pausado, no procesar inputs de movimiento
    if (paused) {
        daos_sleep_ms(20);
        return;
    }

    mutex_lock(&snake_sync_mutex);
    snake_direction_t current_dir = current_direction;
    mutex_unlock(&snake_sync_mutex);

    // D2: Rotar a la izquierda (puede ser múltiple)
    uint32_t current_count_left = daos_btn_get_count_by_index(0);
    if (current_count_left != last_count_left) {
        uint32_t presses = current_count_left - last_count_left;
        snake_direction_t new_dir = current_dir;

        // Rotar tantas veces como presiones
        for (uint32_t i = 0; i < presses; i++) {
            new_dir = (new_dir + 3) % 4;
        }

        mutex_lock(&snake_sync_mutex);
        current_direction = new_dir;
        mutex_unlock(&snake_sync_mutex);

        last_count_left = current_count_left;
    }

    // D3: Rotar a la derecha (puede ser múltiple)
    uint32_t current_count_right = daos_btn_get_count_by_index(1);
    if (current_count_right != last_count_right) {
        uint32_t presses = current_count_right - last_count_right;
        snake_direction_t new_dir = current_dir;

        // Rotar tantas veces como presiones
        for (uint32_t i = 0; i < presses; i++) {
            new_dir = (new_dir + 1) % 4;
        }

        mutex_lock(&snake_sync_mutex);
        current_direction = new_dir;
        mutex_unlock(&snake_sync_mutex);

        last_count_right = current_count_right;
    }

    daos_sleep_ms(20);
}

void snake_logic_task(void) {
    mutex_lock(&snake_sync_mutex);

    // Verificar si está esperando para reanudar (5 segundos silenciosos)
    if (waiting_to_resume) {
        uint32_t current_time = daos_millis();
        uint32_t elapsed = current_time - pause_resume_time;

        if (elapsed >= 5000) {
            waiting_to_resume = 0;
        }
        mutex_unlock(&snake_sync_mutex);
        daos_sleep_ms(100);
        return;
    }

    // Verificar si está pausado
    if (game_paused) {
        mutex_unlock(&snake_sync_mutex);
        daos_sleep_ms(100);
        return;
    }

    // VERIFICAR SI HA PASADO EL TIEMPO DE ESPERA INICIAL (5 SEGUNDOS)
    if (!game_started) {
        uint32_t current_time = daos_millis();
        uint32_t elapsed = current_time - game_start_time;

        if (elapsed < 5000) {
            mutex_unlock(&snake_sync_mutex);
            daos_sleep_ms(200);
            return;
        } else {
            game_started = 1;
        }
    }

    snake_state_t current_state = game_state;
    mutex_unlock(&snake_sync_mutex);

    if (current_state == SNAKE_GAME_OVER) {
        daos_sleep_ms(200);
        return;
    }

    mutex_lock(&snake_sync_mutex);
    uint8_t local_food_exists = food_exists;
    mutex_unlock(&snake_sync_mutex);

    if (!local_food_exists) {
        place_food();
    }

    mutex_lock(&snake_sync_mutex);
    Position current_head = snake_body[0];
    snake_direction_t current_dir = current_direction;
    Position current_food = food_pos;
    mutex_unlock(&snake_sync_mutex);

    Position new_head = current_head;

    switch(current_dir) {
        case SNAKE_UP:
            new_head.y = (new_head.y == 0) ? 255 : new_head.y - 1;
            break;
        case SNAKE_DOWN:
            new_head.y++;
            break;
        case SNAKE_LEFT:
            new_head.x = (new_head.x == 0) ? 255 : new_head.x - 1;
            break;
        case SNAKE_RIGHT:
            new_head.x++;
            break;
    }

    // Detectar colisiones con paredes
    if (new_head.x >= BOARD_WIDTH || new_head.y >= BOARD_HEIGHT ||
        new_head.x == 255 || new_head.y == 255) {
        mutex_lock(&snake_sync_mutex);
        game_state = SNAKE_GAME_OVER;
        mutex_unlock(&snake_sync_mutex);
        return;
    }

    // Detectar colisiones con el propio cuerpo
    mutex_lock(&snake_sync_mutex);
    uint8_t collision = 0;
    // Empezamos desde i=1 para evitar colisionar con la cabeza antigua
    for (uint8_t i = 1; i < snake_length; i++) {
        if (snake_body[i].x == new_head.x && snake_body[i].y == new_head.y) {
            collision = 1;
            break;
        }
    }
    mutex_unlock(&snake_sync_mutex);

    if (collision) {
        mutex_lock(&snake_sync_mutex);
        game_state = SNAKE_GAME_OVER;
        mutex_unlock(&snake_sync_mutex);
        return;
    }

    // Verificar si come la manzana
    uint8_t ate_food = (new_head.x == current_food.x && new_head.y == current_food.y);

    mutex_lock(&snake_sync_mutex);

    if (ate_food) {
        // CRECER
        if (snake_length < MAX_SNAKE_LENGTH) {
            for (int i = snake_length; i > 0; i--) {
                snake_body[i] = snake_body[i - 1];
            }
            snake_body[0] = new_head;
            snake_length++;
            score++;
            food_exists = 0;
        }
    } else {
        // MOVER: Desplazar la cola un paso hacia adelante
        for (int i = snake_length - 1; i > 0; i--) {
            snake_body[i] = snake_body[i - 1];
        }
        snake_body[0] = new_head;
    }

    mutex_unlock(&snake_sync_mutex);

    if (ate_food) {
        place_food();
    }

    // Velocidad del juego: 100 milisegundos
    daos_sleep_ms(100);
}

void snake_render_task(void) {
    // Dibujar marcador superior
    char buffer[32];
    buffer[0] = 'S'; buffer[1] = 'C'; buffer[2] = 'O'; buffer[3] = 'R';
    buffer[4] = 'E'; buffer[5] = ':'; buffer[6] = '\0';
    daos_gfx_draw_text(5, 5, buffer, DAOS_COLOR_WHITE, COLOR_BG);

    mutex_lock(&snake_sync_mutex);
    uint32_t current_score = score;
    uint8_t paused = game_paused;
    uint8_t waiting = waiting_to_resume;
    uint8_t message_cleared = pause_message_cleared;
    mutex_unlock(&snake_sync_mutex);

    buffer[0] = '0' + (current_score / 100) % 10;
    buffer[1] = '0' + (current_score / 10) % 10;
    buffer[2] = '0' + current_score % 10;
    buffer[3] = '\0';
    daos_gfx_draw_text(55, 5, buffer, DAOS_COLOR_GREEN, COLOR_BG);

    // Dibujar bordes del tablero
    int border_x = X_OFFSET - 2;
    int border_y = Y_OFFSET - 2;
    int border_w = GAME_AREA_WIDTH + 4;
    int border_h = GAME_AREA_HEIGHT + 4;

    daos_gfx_fill_rect(border_x, border_y, border_w, 2, DAOS_COLOR_WHITE);
    daos_gfx_fill_rect(border_x, border_y + border_h - 2, border_w, 2, DAOS_COLOR_WHITE);
    daos_gfx_fill_rect(border_x, border_y, 2, border_h, DAOS_COLOR_WHITE);
    daos_gfx_fill_rect(border_x + border_w - 2, border_y, 2, border_h, DAOS_COLOR_WHITE);

    mutex_lock(&snake_sync_mutex);
    snake_state_t current_state = game_state;
    mutex_unlock(&snake_sync_mutex);

    // SOLO MOSTRAR PAUSA cuando está pausado
    if (paused) {
        daos_gfx_fill_rect(60, 90, 200, 60, COLOR_BG);
        daos_gfx_fill_rect(58, 88, 204, 64, DAOS_COLOR_WHITE);
        daos_gfx_fill_rect(60, 90, 200, 60, COLOR_BG);

        daos_gfx_draw_text_large(95, 110, "PAUSA", DAOS_COLOR_YELLOW, COLOR_BG, 2);

        daos_sleep_ms(50);
        return;
    }

    // Limpiar mensaje de pausa una sola vez cuando se quita (INCLUYENDO EL BORDE BLANCO)
    if (waiting && !message_cleared) {
        // Limpiar el área COMPLETA del mensaje de pausa incluyendo el borde blanco
        daos_gfx_fill_rect(58, 88, 204, 64, COLOR_BG);

        mutex_lock(&snake_sync_mutex);
        pause_message_cleared = 1;
        mutex_unlock(&snake_sync_mutex);
    }

    // Game Over
    if (current_state == SNAKE_GAME_OVER) {
        daos_gfx_fill_rect(60, 90, 200, 60, COLOR_BG);
        daos_gfx_fill_rect(58, 88, 204, 64, DAOS_COLOR_WHITE);
        daos_gfx_fill_rect(60, 90, 200, 60, COLOR_BG);

        daos_gfx_draw_text_large(80, 100, "GAME OVER", DAOS_COLOR_WHITE, COLOR_BG, 2);

        buffer[0] = 'S'; buffer[1] = 'C'; buffer[2] = 'O'; buffer[3] = 'R';
        buffer[4] = 'E'; buffer[5] = ':'; buffer[6] = '\0';
        daos_gfx_draw_text_large(85, 125, buffer, DAOS_COLOR_WHITE, COLOR_BG, 1);

        buffer[0] = '0' + (current_score / 100) % 10;
        buffer[1] = '0' + (current_score / 10) % 10;
        buffer[2] = '0' + current_score % 10;
        buffer[3] = '\0';
        daos_gfx_draw_text_large(160, 125, buffer, DAOS_COLOR_GREEN, COLOR_BG, 2);

        daos_sleep_ms(5000);
        return;
    }

    // SOLO BORRAR LA ÚLTIMA POSICIÓN DE LA COLA (optimización)
    static Position last_tail = {0, 0};
    static uint8_t first_render = 1;

    if (!first_render) {
        draw_game_pixel(last_tail.x, last_tail.y, COLOR_BG);
    }

    mutex_lock(&snake_sync_mutex);
    uint8_t local_length = snake_length;
    Position local_food = food_pos;
    uint8_t local_food_exists = food_exists;

    Position local_body[MAX_SNAKE_LENGTH];
    for (uint8_t i = 0; i < local_length; i++) {
        local_body[i] = snake_body[i];
    }

    // Guardar la última posición de la cola para borrarla en el siguiente frame
    if (local_length > 0) {
        last_tail = local_body[local_length - 1];
    }
    mutex_unlock(&snake_sync_mutex);

    // Dibujar el CUERPO del gusano (verde)
    for (uint8_t i = 1; i < local_length; i++) {
        draw_game_pixel(local_body[i].x, local_body[i].y, COLOR_SNAKE);
    }

    // Dibujar la CABEZA del gusano (blanca)
    if (local_length > 0) {
        draw_game_pixel(local_body[0].x, local_body[0].y, COLOR_HEAD);
    }

    // Dibujar la comida (roja)
    if (local_food_exists) {
        draw_game_pixel(local_food.x, local_food.y, COLOR_FOOD);
    }

    first_render = 0;
    daos_sleep_ms(50);
}

/* ============================================================ */
/* INICIALIZACIÓN Y CLEANUP                             */
/* ============================================================ */

void snake_reset(void) {
    mutex_lock(&snake_sync_mutex);

    // Limpiar el array del cuerpo
    memset(snake_body, 0, sizeof(Position) * MAX_SNAKE_LENGTH);

    // Limpiar la pantalla completa SOLO EN RESET
    daos_gfx_clear(COLOR_BG);

    // Dibujar bordes iniciales
    int border_x = X_OFFSET - 2;
    int border_y = Y_OFFSET - 2;
    int border_w = GAME_AREA_WIDTH + 4;
    int border_h = GAME_AREA_HEIGHT + 4;

    daos_gfx_fill_rect(border_x, border_y, border_w, 2, DAOS_COLOR_WHITE);
    daos_gfx_fill_rect(border_x, border_y + border_h - 2, border_w, 2, DAOS_COLOR_WHITE);
    daos_gfx_fill_rect(border_x, border_y, 2, border_h, DAOS_COLOR_WHITE);
    daos_gfx_fill_rect(border_x + border_w - 2, border_y, 2, border_h, DAOS_COLOR_WHITE);

    // Posición central inicial (cabeza)
    snake_body[0].x = BOARD_WIDTH / 2;
    snake_body[0].y = BOARD_HEIGHT / 2;

    snake_length = 1;
    current_direction = SNAKE_RIGHT;

    score = 0;
    game_state = SNAKE_RUNNING;
    food_exists = 0;

    // REINICIAR TEMPORIZADOR DE INICIO
    game_start_time = daos_millis();
    game_started = 0;

    // REINICIAR ESTADO DE PAUSA
    game_paused = 0;
    waiting_to_resume = 0;
    pause_resume_time = 0;
    pause_message_cleared = 0;

    mutex_unlock(&snake_sync_mutex);

    place_food();
}

void snake_init(void) {
    mutex_init(&snake_sync_mutex);

    // INICIALIZAR TEMPORIZADOR
    game_start_time = daos_millis();
    game_started = 0;

    // INICIALIZAR PAUSA
    game_paused = 0;
    waiting_to_resume = 0;
    pause_resume_time = 0;
    pause_message_cleared = 0;

    // Llamamos a reset para asegurar la limpieza total
    snake_reset();
}

void snake_cleanup(void) {
    mutex_lock(&snake_sync_mutex);
    daos_gfx_clear(COLOR_BG);
    game_state = SNAKE_GAME_OVER;
    mutex_unlock(&snake_sync_mutex);
}

/* ============================================================ */
/* API PÚBLICA                                         */
/* ============================================================ */

snake_state_t snake_get_state(void) {
    mutex_lock(&snake_sync_mutex);
    snake_state_t state = game_state;
    mutex_unlock(&snake_sync_mutex);
    return state;
}

uint32_t snake_get_score(void) {
    mutex_lock(&snake_sync_mutex);
    uint32_t current_score = score;
    mutex_unlock(&snake_sync_mutex);
    return current_score;
}

daos_binario_ejecutable_t* snake_get_binario(void) {
    return daos_binario_crear(
        "SNAKE",
        DAOS_BINARIO_TIPO_JUEGO,
        snake_init,
        snake_reset,
        snake_input_task,
        snake_logic_task,
        snake_render_task,
        snake_cleanup,
        (uint32_t (*)(void))snake_get_state
    );
}
