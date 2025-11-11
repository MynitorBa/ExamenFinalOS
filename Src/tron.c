#include "tron.h"
#include "api.h"
#include "sync.h"

#define BOARD_WIDTH 60
#define BOARD_HEIGHT 40
#define PIXEL_SIZE 5

// Estructura para representar una posición
typedef struct {
    uint8_t x;
    uint8_t y;
} Position;

// Rastro de luz (trail) para cada jugador
#define MAX_TRAIL_LENGTH 1800
static Position light_trail_p1[MAX_TRAIL_LENGTH];
static Position light_trail_p2[MAX_TRAIL_LENGTH];
static Position light_trail_p3[MAX_TRAIL_LENGTH];
static Position light_trail_p4[MAX_TRAIL_LENGTH];
static uint16_t trail_length_p1 = 0;
static uint16_t trail_length_p2 = 0;
static uint16_t trail_length_p3 = 0;
static uint16_t trail_length_p4 = 0;

// Motos de los 4 jugadores
static Position bike_p1;
static Position bike_p2;
static Position bike_p3;
static Position bike_p4;
static tron_direction_t dir_p1 = TRON_RIGHT;
static tron_direction_t dir_p2 = TRON_LEFT;
static tron_direction_t dir_p3 = TRON_DOWN;
static tron_direction_t dir_p4 = TRON_UP;

// Estado del juego
static tron_state_t game_state = TRON_RUNNING;
static uint32_t distance_p1 = 0;
static uint32_t distance_p2 = 0;
static uint32_t distance_p3 = 0;
static uint32_t distance_p4 = 0;
static uint8_t alive_p1 = 1;
static uint8_t alive_p2 = 1;
static uint8_t alive_p3 = 1;
static uint8_t alive_p4 = 1;
static tron_winner_t winner = TRON_NO_WINNER;

// Colores de los jugadores
#define COLOR_P1 DAOS_COLOR_CYAN
#define COLOR_P2 DAOS_COLOR_MAGENTA
#define COLOR_P3 DAOS_COLOR_YELLOW
#define COLOR_P4 DAOS_COLOR_GREEN

/* ============================================================ */
/*          MUTEXES Y SEMÁFOROS PARA SINCRONIZACIÓN            */
/* ============================================================ */

// Mutexes para protección de recursos compartidos
mutex_t tron_game_state_mutex;
mutex_t tron_trail_mutex;
mutex_t tron_bike_mutex;

// Semáforos para sincronización de tareas
sem_t tron_input_ready_sem;
sem_t tron_logic_ready_sem;
sem_t tron_render_ready_sem;

/* ============================================================ */
/*          FUNCIONES AUXILIARES                               */
/* ============================================================ */

static uint8_t is_trail_at(uint8_t x, uint8_t y) {
    uint8_t result = 0;

    // PROTECCIÓN: Acceso al trail con mutex
    mutex_lock(&tron_trail_mutex);

    for (uint16_t i = 0; i < trail_length_p1; i++) {
        if (light_trail_p1[i].x == x && light_trail_p1[i].y == y) {
            result = 1;
            break;
        }
    }

    if (!result) {
        for (uint16_t i = 0; i < trail_length_p2; i++) {
            if (light_trail_p2[i].x == x && light_trail_p2[i].y == y) {
                result = 1;
                break;
            }
        }
    }

    if (!result) {
        for (uint16_t i = 0; i < trail_length_p3; i++) {
            if (light_trail_p3[i].x == x && light_trail_p3[i].y == y) {
                result = 1;
                break;
            }
        }
    }

    if (!result) {
        for (uint16_t i = 0; i < trail_length_p4; i++) {
            if (light_trail_p4[i].x == x && light_trail_p4[i].y == y) {
                result = 1;
                break;
            }
        }
    }

    mutex_unlock(&tron_trail_mutex);
    return result;
}

static void add_to_trail_p1(uint8_t x, uint8_t y) {
    // PROTECCIÓN: Modificación del trail con mutex
    mutex_lock(&tron_trail_mutex);

    if (trail_length_p1 < MAX_TRAIL_LENGTH) {
        light_trail_p1[trail_length_p1].x = x;
        light_trail_p1[trail_length_p1].y = y;
        trail_length_p1++;
    }

    mutex_unlock(&tron_trail_mutex);
}

static void add_to_trail_p2(uint8_t x, uint8_t y) {
    // PROTECCIÓN: Modificación del trail con mutex
    mutex_lock(&tron_trail_mutex);

    if (trail_length_p2 < MAX_TRAIL_LENGTH) {
        light_trail_p2[trail_length_p2].x = x;
        light_trail_p2[trail_length_p2].y = y;
        trail_length_p2++;
    }

    mutex_unlock(&tron_trail_mutex);
}

static void add_to_trail_p3(uint8_t x, uint8_t y) {
    // PROTECCIÓN: Modificación del trail con mutex
    mutex_lock(&tron_trail_mutex);

    if (trail_length_p3 < MAX_TRAIL_LENGTH) {
        light_trail_p3[trail_length_p3].x = x;
        light_trail_p3[trail_length_p3].y = y;
        trail_length_p3++;
    }

    mutex_unlock(&tron_trail_mutex);
}

static void add_to_trail_p4(uint8_t x, uint8_t y) {
    // PROTECCIÓN: Modificación del trail con mutex
    mutex_lock(&tron_trail_mutex);

    if (trail_length_p4 < MAX_TRAIL_LENGTH) {
        light_trail_p4[trail_length_p4].x = x;
        light_trail_p4[trail_length_p4].y = y;
        trail_length_p4++;
    }

    mutex_unlock(&tron_trail_mutex);
}

static void draw_game_pixel(uint8_t x, uint8_t y, uint16_t color) {
    int screen_x = 10 + x * PIXEL_SIZE;
    int screen_y = 30 + y * PIXEL_SIZE;
    daos_gfx_fill_rect(screen_x, screen_y, PIXEL_SIZE, PIXEL_SIZE, color);
}

/* ============================================================ */
/*          TAREAS DEL JUEGO                                   */
/* ============================================================ */

void tron_input_task(void) {
    static uint32_t last_count_p1_left = 0;
    static uint32_t last_count_p1_right = 0;
    static uint32_t last_count_p2_left = 0;
    static uint32_t last_count_p2_right = 0;
    static uint32_t last_count_p3_left = 0;
    static uint32_t last_count_p3_right = 0;
    static uint32_t last_count_p4_left = 0;
    static uint32_t last_count_p4_right = 0;
    static uint8_t initialized = 0;

    // Sincronizar contadores en la primera ejecución
    if (!initialized) {
        last_count_p1_left = daos_btn_get_count_by_index(0);
        last_count_p1_right = daos_btn_get_count_by_index(1);
        last_count_p2_left = daos_btn_get_count_by_index(2);
        last_count_p2_right = daos_btn_get_count_by_index(3);
        last_count_p3_left = daos_btn_get_count_by_index(4);
        last_count_p3_right = daos_btn_get_count_by_index(5);
        last_count_p4_left = daos_btn_get_count_by_index(6);
        last_count_p4_right = daos_btn_get_count_by_index(7);
        initialized = 1;
        daos_sleep_ms(50);
        return;
    }

    // PROTECCIÓN: Verificar estado del juego de forma segura
    mutex_lock(&tron_game_state_mutex);
    tron_state_t current_state = game_state;
    mutex_unlock(&tron_game_state_mutex);

    if (current_state == TRON_GAME_OVER) {
        daos_sleep_ms(100);
        return;
    }

    // PROTECCIÓN: Lectura y escritura de direcciones con mutex
    mutex_lock(&tron_bike_mutex);
    uint8_t local_alive_p1 = alive_p1;
    uint8_t local_alive_p2 = alive_p2;
    uint8_t local_alive_p3 = alive_p3;
    uint8_t local_alive_p4 = alive_p4;
    tron_direction_t local_dir_p1 = dir_p1;
    tron_direction_t local_dir_p2 = dir_p2;
    tron_direction_t local_dir_p3 = dir_p3;
    tron_direction_t local_dir_p4 = dir_p4;
    mutex_unlock(&tron_bike_mutex);

    // ===== JUGADOR 1: D2 (índice 0), D3 (índice 1) =====
    if (local_alive_p1) {
        uint32_t current_count_p1_left = daos_btn_get_count_by_index(0);
        uint32_t current_count_p1_right = daos_btn_get_count_by_index(1);

        if (current_count_p1_left != last_count_p1_left) {
            mutex_lock(&tron_bike_mutex);
            dir_p1 = (dir_p1 + 3) % 4;
            mutex_unlock(&tron_bike_mutex);
            last_count_p1_left = current_count_p1_left;
        }

        if (current_count_p1_right != last_count_p1_right) {
            mutex_lock(&tron_bike_mutex);
            dir_p1 = (dir_p1 + 1) % 4;
            mutex_unlock(&tron_bike_mutex);
            last_count_p1_right = current_count_p1_right;
        }
    }

    // ===== JUGADOR 2: D4 (índice 2), D5 (índice 3) =====
    if (local_alive_p2) {
        uint32_t current_count_p2_left = daos_btn_get_count_by_index(2);
        uint32_t current_count_p2_right = daos_btn_get_count_by_index(3);

        if (current_count_p2_left != last_count_p2_left) {
            mutex_lock(&tron_bike_mutex);
            dir_p2 = (dir_p2 + 3) % 4;
            mutex_unlock(&tron_bike_mutex);
            last_count_p2_left = current_count_p2_left;
        }

        if (current_count_p2_right != last_count_p2_right) {
            mutex_lock(&tron_bike_mutex);
            dir_p2 = (dir_p2 + 1) % 4;
            mutex_unlock(&tron_bike_mutex);
            last_count_p2_right = current_count_p2_right;
        }
    }

    // ===== JUGADOR 3: D6 (índice 4), D7 (índice 5) =====
    if (local_alive_p3) {
        uint32_t current_count_p3_left = daos_btn_get_count_by_index(4);
        uint32_t current_count_p3_right = daos_btn_get_count_by_index(5);

        if (current_count_p3_left != last_count_p3_left) {
            mutex_lock(&tron_bike_mutex);
            dir_p3 = (dir_p3 + 3) % 4;
            mutex_unlock(&tron_bike_mutex);
            last_count_p3_left = current_count_p3_left;
        }

        if (current_count_p3_right != last_count_p3_right) {
            mutex_lock(&tron_bike_mutex);
            dir_p3 = (dir_p3 + 1) % 4;
            mutex_unlock(&tron_bike_mutex);
            last_count_p3_right = current_count_p3_right;
        }
    }

    // ===== JUGADOR 4: D8 (índice 6), D9 (índice 7) =====
    if (local_alive_p4) {
        uint32_t current_count_p4_left = daos_btn_get_count_by_index(6);
        uint32_t current_count_p4_right = daos_btn_get_count_by_index(7);

        if (current_count_p4_left != last_count_p4_left) {
            mutex_lock(&tron_bike_mutex);
            dir_p4 = (dir_p4 + 3) % 4;
            mutex_unlock(&tron_bike_mutex);
            last_count_p4_left = current_count_p4_left;
        }

        if (current_count_p4_right != last_count_p4_right) {
            mutex_lock(&tron_bike_mutex);
            dir_p4 = (dir_p4 + 1) % 4;
            mutex_unlock(&tron_bike_mutex);
            last_count_p4_right = current_count_p4_right;
        }
    }

    daos_sleep_ms(50);
}

void tron_logic_task(void) {
    // PROTECCIÓN: Verificar estado del juego
    mutex_lock(&tron_game_state_mutex);
    tron_state_t current_state = game_state;
    mutex_unlock(&tron_game_state_mutex);

    if (current_state == TRON_GAME_OVER) {
        daos_sleep_ms(200);
        return;
    }

    // PROTECCIÓN: Leer estado actual de forma segura
    mutex_lock(&tron_bike_mutex);
    uint8_t local_alive_p1 = alive_p1;
    uint8_t local_alive_p2 = alive_p2;
    uint8_t local_alive_p3 = alive_p3;
    uint8_t local_alive_p4 = alive_p4;
    Position local_bike_p1 = bike_p1;
    Position local_bike_p2 = bike_p2;
    Position local_bike_p3 = bike_p3;
    Position local_bike_p4 = bike_p4;
    tron_direction_t local_dir_p1 = dir_p1;
    tron_direction_t local_dir_p2 = dir_p2;
    tron_direction_t local_dir_p3 = dir_p3;
    tron_direction_t local_dir_p4 = dir_p4;
    mutex_unlock(&tron_bike_mutex);

    // Guardar posiciones en rastros
    if (local_alive_p1) add_to_trail_p1(local_bike_p1.x, local_bike_p1.y);
    if (local_alive_p2) add_to_trail_p2(local_bike_p2.x, local_bike_p2.y);
    if (local_alive_p3) add_to_trail_p3(local_bike_p3.x, local_bike_p3.y);
    if (local_alive_p4) add_to_trail_p4(local_bike_p4.x, local_bike_p4.y);

    // Calcular nuevas posiciones
    Position new_p1 = local_bike_p1;
    Position new_p2 = local_bike_p2;
    Position new_p3 = local_bike_p3;
    Position new_p4 = local_bike_p4;

    // Mover P1
    if (local_alive_p1) {
        switch(local_dir_p1) {
            case TRON_UP:    if (new_p1.y > 0) new_p1.y--; else new_p1.y = 255; break;
            case TRON_DOWN:  new_p1.y++; break;
            case TRON_LEFT:  if (new_p1.x > 0) new_p1.x--; else new_p1.x = 255; break;
            case TRON_RIGHT: new_p1.x++; break;
        }
    }

    // Mover P2
    if (local_alive_p2) {
        switch(local_dir_p2) {
            case TRON_UP:    if (new_p2.y > 0) new_p2.y--; else new_p2.y = 255; break;
            case TRON_DOWN:  new_p2.y++; break;
            case TRON_LEFT:  if (new_p2.x > 0) new_p2.x--; else new_p2.x = 255; break;
            case TRON_RIGHT: new_p2.x++; break;
        }
    }

    // Mover P3
    if (local_alive_p3) {
        switch(local_dir_p3) {
            case TRON_UP:    if (new_p3.y > 0) new_p3.y--; else new_p3.y = 255; break;
            case TRON_DOWN:  new_p3.y++; break;
            case TRON_LEFT:  if (new_p3.x > 0) new_p3.x--; else new_p3.x = 255; break;
            case TRON_RIGHT: new_p3.x++; break;
        }
    }

    // Mover P4
    if (local_alive_p4) {
        switch(local_dir_p4) {
            case TRON_UP:    if (new_p4.y > 0) new_p4.y--; else new_p4.y = 255; break;
            case TRON_DOWN:  new_p4.y++; break;
            case TRON_LEFT:  if (new_p4.x > 0) new_p4.x--; else new_p4.x = 255; break;
            case TRON_RIGHT: new_p4.x++; break;
        }
    }

    // Detectar colisiones
    if (local_alive_p1) {
        if (new_p1.x >= BOARD_WIDTH || new_p1.y >= BOARD_HEIGHT || is_trail_at(new_p1.x, new_p1.y)) {
            local_alive_p1 = 0;
        } else {
            local_bike_p1 = new_p1;
            distance_p1++;
        }
    }

    if (local_alive_p2) {
        if (new_p2.x >= BOARD_WIDTH || new_p2.y >= BOARD_HEIGHT || is_trail_at(new_p2.x, new_p2.y)) {
            local_alive_p2 = 0;
        } else {
            local_bike_p2 = new_p2;
            distance_p2++;
        }
    }

    if (local_alive_p3) {
        if (new_p3.x >= BOARD_WIDTH || new_p3.y >= BOARD_HEIGHT || is_trail_at(new_p3.x, new_p3.y)) {
            local_alive_p3 = 0;
        } else {
            local_bike_p3 = new_p3;
            distance_p3++;
        }
    }

    if (local_alive_p4) {
        if (new_p4.x >= BOARD_WIDTH || new_p4.y >= BOARD_HEIGHT || is_trail_at(new_p4.x, new_p4.y)) {
            local_alive_p4 = 0;
        } else {
            local_bike_p4 = new_p4;
            distance_p4++;
        }
    }

    // PROTECCIÓN: Actualizar estado global de forma segura
    mutex_lock(&tron_bike_mutex);
    bike_p1 = local_bike_p1;
    bike_p2 = local_bike_p2;
    bike_p3 = local_bike_p3;
    bike_p4 = local_bike_p4;
    alive_p1 = local_alive_p1;
    alive_p2 = local_alive_p2;
    alive_p3 = local_alive_p3;
    alive_p4 = local_alive_p4;
    mutex_unlock(&tron_bike_mutex);

    // Verificar ganador
    uint8_t alive_count = local_alive_p1 + local_alive_p2 + local_alive_p3 + local_alive_p4;

    if (alive_count <= 1) {
        mutex_lock(&tron_game_state_mutex);
        game_state = TRON_GAME_OVER;

        if (alive_count == 0) {
            winner = TRON_DRAW;
        } else {
            if (local_alive_p1) winner = TRON_PLAYER1_WINS;
            else if (local_alive_p2) winner = TRON_PLAYER2_WINS;
            else if (local_alive_p3) winner = TRON_PLAYER3_WINS;
            else if (local_alive_p4) winner = TRON_PLAYER4_WINS;
        }
        mutex_unlock(&tron_game_state_mutex);
    }

    daos_sleep_ms(300);
}

void tron_render_task(void) {
    static uint8_t first_frame = 1;

    if (first_frame) {
        daos_gfx_clear(DAOS_COLOR_BLACK);
        first_frame = 0;
    }

    // Dibujar marcadores superiores
    char buffer[32];

    // P1
    buffer[0] = 'P'; buffer[1] = '1'; buffer[2] = ':'; buffer[3] = '\0';
    daos_gfx_draw_text(5, 5, buffer, COLOR_P1, DAOS_COLOR_BLACK);
    buffer[0] = '0' + (distance_p1 / 100) % 10;
    buffer[1] = '0' + (distance_p1 / 10) % 10;
    buffer[2] = '0' + distance_p1 % 10;
    buffer[3] = '\0';
    daos_gfx_draw_text(25, 5, buffer, DAOS_COLOR_WHITE, DAOS_COLOR_BLACK);

    // P2
    buffer[0] = 'P'; buffer[1] = '2'; buffer[2] = ':'; buffer[3] = '\0';
    daos_gfx_draw_text(80, 5, buffer, COLOR_P2, DAOS_COLOR_BLACK);
    buffer[0] = '0' + (distance_p2 / 100) % 10;
    buffer[1] = '0' + (distance_p2 / 10) % 10;
    buffer[2] = '0' + distance_p2 % 10;
    buffer[3] = '\0';
    daos_gfx_draw_text(100, 5, buffer, DAOS_COLOR_WHITE, DAOS_COLOR_BLACK);

    // P3
    buffer[0] = 'P'; buffer[1] = '3'; buffer[2] = ':'; buffer[3] = '\0';
    daos_gfx_draw_text(155, 5, buffer, COLOR_P3, DAOS_COLOR_BLACK);
    buffer[0] = '0' + (distance_p3 / 100) % 10;
    buffer[1] = '0' + (distance_p3 / 10) % 10;
    buffer[2] = '0' + distance_p3 % 10;
    buffer[3] = '\0';
    daos_gfx_draw_text(175, 5, buffer, DAOS_COLOR_WHITE, DAOS_COLOR_BLACK);

    // P4
    buffer[0] = 'P'; buffer[1] = '4'; buffer[2] = ':'; buffer[3] = '\0';
    daos_gfx_draw_text(230, 5, buffer, COLOR_P4, DAOS_COLOR_BLACK);
    buffer[0] = '0' + (distance_p4 / 100) % 10;
    buffer[1] = '0' + (distance_p4 / 10) % 10;
    buffer[2] = '0' + distance_p4 % 10;
    buffer[3] = '\0';
    daos_gfx_draw_text(250, 5, buffer, DAOS_COLOR_WHITE, DAOS_COLOR_BLACK);

    // Dibujar bordes del tablero
    daos_gfx_fill_rect(8, 28, BOARD_WIDTH * PIXEL_SIZE + 4, 2, DAOS_COLOR_WHITE);
    daos_gfx_fill_rect(8, 30 + BOARD_HEIGHT * PIXEL_SIZE, BOARD_WIDTH * PIXEL_SIZE + 4, 2, DAOS_COLOR_WHITE);
    daos_gfx_fill_rect(8, 28, 2, BOARD_HEIGHT * PIXEL_SIZE + 4, DAOS_COLOR_WHITE);
    daos_gfx_fill_rect(10 + BOARD_WIDTH * PIXEL_SIZE, 28, 2, BOARD_HEIGHT * PIXEL_SIZE + 4, DAOS_COLOR_WHITE);

    // PROTECCIÓN: Verificar estado del juego
    mutex_lock(&tron_game_state_mutex);
    tron_state_t current_state = game_state;
    tron_winner_t current_winner = winner;
    mutex_unlock(&tron_game_state_mutex);

    // Game Over
    if (current_state == TRON_GAME_OVER) {
        daos_gfx_fill_rect(60, 90, 200, 60, DAOS_COLOR_BLACK);
        daos_gfx_fill_rect(58, 88, 204, 64, DAOS_COLOR_WHITE);
        daos_gfx_fill_rect(60, 90, 200, 60, DAOS_COLOR_BLACK);

        daos_gfx_draw_text_large(80, 100, "GAME OVER", DAOS_COLOR_WHITE, DAOS_COLOR_BLACK, 2);

        switch(current_winner) {
            case TRON_PLAYER1_WINS:
                daos_gfx_draw_text_large(90, 125, "P1 WINS", COLOR_P1, DAOS_COLOR_BLACK, 2);
                break;
            case TRON_PLAYER2_WINS:
                daos_gfx_draw_text_large(90, 125, "P2 WINS", COLOR_P2, DAOS_COLOR_BLACK, 2);
                break;
            case TRON_PLAYER3_WINS:
                daos_gfx_draw_text_large(90, 125, "P3 WINS", COLOR_P3, DAOS_COLOR_BLACK, 2);
                break;
            case TRON_PLAYER4_WINS:
                daos_gfx_draw_text_large(90, 125, "P4 WINS", COLOR_P4, DAOS_COLOR_BLACK, 2);
                break;
            case TRON_DRAW:
                daos_gfx_draw_text_large(100, 125, "DRAW", DAOS_COLOR_WHITE, DAOS_COLOR_BLACK, 2);
                breakdefault:
                break;
        }

        daos_sleep_ms(5000);
        return;
    }

    // PROTECCIÓN: Leer posiciones de motos de forma segura
    mutex_lock(&tron_bike_mutex);
    Position current_bike_p1 = bike_p1;
    Position current_bike_p2 = bike_p2;
    Position current_bike_p3 = bike_p3;
    Position current_bike_p4 = bike_p4;
    uint8_t local_alive_p1 = alive_p1;
    uint8_t local_alive_p2 = alive_p2;
    uint8_t local_alive_p3 = alive_p3;
    uint8_t local_alive_p4 = alive_p4;
    mutex_unlock(&tron_bike_mutex);

    // PROTECCIÓN: Dibujar trails de forma segura
    mutex_lock(&tron_trail_mutex);

    for (uint16_t i = 0; i < trail_length_p1; i++) {
        draw_game_pixel(light_trail_p1[i].x, light_trail_p1[i].y, COLOR_P1);
    }
    for (uint16_t i = 0; i < trail_length_p2; i++) {
        draw_game_pixel(light_trail_p2[i].x, light_trail_p2[i].y, COLOR_P2);
    }
    for (uint16_t i = 0; i < trail_length_p3; i++) {
        draw_game_pixel(light_trail_p3[i].x, light_trail_p3[i].y, COLOR_P3);
    }
    for (uint16_t i = 0; i < trail_length_p4; i++) {
        draw_game_pixel(light_trail_p4[i].x, light_trail_p4[i].y, COLOR_P4);
    }

    mutex_unlock(&tron_trail_mutex);

    // Dibujar motos
    if (local_alive_p1) draw_game_pixel(current_bike_p1.x, current_bike_p1.y, DAOS_COLOR_WHITE);
    if (local_alive_p2) draw_game_pixel(current_bike_p2.x, current_bike_p2.y, DAOS_COLOR_WHITE);
    if (local_alive_p3) draw_game_pixel(current_bike_p3.x, current_bike_p3.y, DAOS_COLOR_WHITE);
    if (local_alive_p4) draw_game_pixel(current_bike_p4.x, current_bike_p4.y, DAOS_COLOR_WHITE);

    daos_sleep_ms(100);
}

/* ============================================================ */
/*          INICIALIZACIÓN Y CLEANUP                           */
/* ============================================================ */

void tron_reset(void) {
    // PROTECCIÓN: Resetear el juego de forma segura
    mutex_lock(&tron_bike_mutex);
    bike_p1.x = 10;  bike_p1.y = 10;  dir_p1 = TRON_RIGHT;
    bike_p2.x = 49;  bike_p2.y = 10;  dir_p2 = TRON_LEFT;
    bike_p3.x = 10;  bike_p3.y = 29;  dir_p3 = TRON_RIGHT;
    bike_p4.x = 49;  bike_p4.y = 29;  dir_p4 = TRON_LEFT;

    distance_p1 = distance_p2 = distance_p3 = distance_p4 = 0;
    alive_p1 = alive_p2 = alive_p3 = alive_p4 = 1;
    mutex_unlock(&tron_bike_mutex);

    mutex_lock(&tron_game_state_mutex);
    game_state = TRON_RUNNING;
    winner = TRON_NO_WINNER;
    mutex_unlock(&tron_game_state_mutex);

    mutex_lock(&tron_trail_mutex);
    trail_length_p1 = trail_length_p2 = trail_length_p3 = trail_length_p4 = 0;
    mutex_unlock(&tron_trail_mutex);
}

void tron_init(void) {
    // INICIALIZACIÓN: Configurar mutexes y semáforos
    mutex_init(&tron_game_state_mutex);
    mutex_init(&tron_trail_mutex);
    mutex_init(&tron_bike_mutex);

    // Inicializar semáforos (NO SE USAN - se eliminaron las sincronizaciones bloqueantes)
    // Los dejamos inicializados por si se necesitan en el futuro
    sem_init(&tron_input_ready_sem, 0, 1);
    sem_init(&tron_logic_ready_sem, 0, 1);
    sem_init(&tron_render_ready_sem, 0, 1);

    // Resetear el estado del juego
    tron_reset();
}

void tron_cleanup(void) {
    // PROTECCIÓN: Limpiar de forma segura
    mutex_lock(&tron_game_state_mutex);
    daos_gfx_clear(DAOS_COLOR_BLACK);
    game_state = TRON_GAME_OVER;
    mutex_unlock(&tron_game_state_mutex);

    // Nota: Los mutexes y semáforos no necesitan destrucción explícita
    // en este sistema embebido, pero podrían agregarse funciones
    // mutex_destroy() y sem_destroy() si fuera necesario
}

/* ============================================================ */
/*          API PÚBLICA                                        */
/* ============================================================ */

tron_state_t tron_get_state(void) {
    // PROTECCIÓN: Lectura segura del estado
    mutex_lock(&tron_game_state_mutex);
    tron_state_t state = game_state;
    mutex_unlock(&tron_game_state_mutex);
    return state;
}

uint32_t tron_get_score_p1(void) {
    // PROTECCIÓN: Lectura segura del score
    // distance_p1 es de tipo uint32_t, la lectura es atómica en ARM Cortex-M4
    // pero por consistencia usamos mutex
    mutex_lock(&tron_bike_mutex);
    uint32_t score = distance_p1;
    mutex_unlock(&tron_bike_mutex);
    return score;
}

uint32_t tron_get_score_p2(void) {
    // PROTECCIÓN: Lectura segura del score
    mutex_lock(&tron_bike_mutex);
    uint32_t score = distance_p2;
    mutex_unlock(&tron_bike_mutex);
    return score;
}

uint32_t tron_get_score_p3(void) {
    // PROTECCIÓN: Lectura segura del score
    mutex_lock(&tron_bike_mutex);
    uint32_t score = distance_p3;
    mutex_unlock(&tron_bike_mutex);
    return score;
}

uint32_t tron_get_score_p4(void) {
    // PROTECCIÓN: Lectura segura del score
    mutex_lock(&tron_bike_mutex);
    uint32_t score = distance_p4;
    mutex_unlock(&tron_bike_mutex);
    return score;
}

tron_winner_t tron_get_winner(void) {
    // PROTECCIÓN: Lectura segura del ganador
    mutex_lock(&tron_game_state_mutex);
    tron_winner_t w = winner;
    mutex_unlock(&tron_game_state_mutex);
    return w;
}
