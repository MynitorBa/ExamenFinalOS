#include "tron2.h"
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
static uint16_t trail_length_p1 = 0;
static uint16_t trail_length_p2 = 0;

// Motos de los 2 jugadores
static Position bike_p1;
static Position bike_p2;
static tron2_direction_t dir_p1 = TRON2_RIGHT;
static tron2_direction_t dir_p2 = TRON2_LEFT;

// Dirección pendiente (nueva dirección que se aplicará en el próximo frame)
static tron2_direction_t pending_dir_p1 = TRON2_RIGHT;
static tron2_direction_t pending_dir_p2 = TRON2_LEFT;
static uint8_t dir_changed_p1 = 0;
static uint8_t dir_changed_p2 = 0;

// Estado del juego
static tron2_state_t game_state = TRON2_RUNNING;
static uint32_t distance_p1 = 0;
static uint32_t distance_p2 = 0;
static uint8_t alive_p1 = 1;
static uint8_t alive_p2 = 1;
static tron2_winner_t winner = TRON2_NO_WINNER;

// Colores de los jugadores
#define COLOR_P1 DAOS_COLOR_CYAN
#define COLOR_P2 DAOS_COLOR_MAGENTA

#define SPRITE_SIZE 7
#define SPRITE_SCALE 2
#define SPRITE_OFFSET 2.5

static const uint32_t sprite_p1_data[49] = {
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff0066cc, 0x00000000,
    0x00000000, 0xff0066cc, 0x00000000, 0x00000000, 0xff0066cc, 0xff0066cc, 0xff003d99,
    0x00000000, 0xff003d99, 0xff0066cc, 0x00000000, 0xff003d99, 0xff003d99, 0xff003d99,
    0xff66b3ff, 0xff66b3ff, 0xff003d99, 0xff003d99, 0xff003d99, 0xff66b3ff, 0xff66b3ff,
    0x00000000, 0xff003d99, 0xff0066cc, 0x00000000, 0xff003d99, 0xff003d99, 0xff003d99,
    0x00000000, 0xff0066cc, 0x00000000, 0x00000000, 0xff0066cc, 0xff0066cc, 0xff003d99,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff0066cc, 0x00000000
};

static const uint32_t sprite_p2_data[49] = {
    0x00000000, 0xffcc0000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xff990000, 0xffcc0000, 0xffcc0000, 0x00000000, 0x00000000, 0xffcc0000, 0x00000000,
    0xff990000, 0xff990000, 0xff990000, 0x00000000, 0xffff6666, 0xff990000, 0x00000000,
    0xffff6666, 0xffff6666, 0xff990000, 0xff990000, 0xff990000, 0xff990000, 0xff990000,
    0xff990000, 0xff990000, 0xff990000, 0x00000000, 0xffff6666, 0xff990000, 0x00000000,
    0x00000000, 0xffcc0000, 0xffcc0000, 0x00000000, 0x00000000, 0xffcc0000, 0x00000000,
    0xffcc0000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
};

/* ============================================================ */
/*          MUTEXES Y SEMÁFOROS PARA SINCRONIZACIÓN            */
/* ============================================================ */

// Mutexes para protección de recursos compartidos
mutex_t tron2_game_state_mutex;
mutex_t tron2_trail_mutex;
mutex_t tron2_bike_mutex;

// Semáforos para sincronización de tareas
sem_t tron2_input_ready_sem;
sem_t tron2_logic_ready_sem;
sem_t tron2_render_ready_sem;

/* ============================================================ */
/*          FUNCIONES AUXILIARES                               */
/* ============================================================ */

static uint8_t is_opposite_direction(tron2_direction_t current, tron2_direction_t new_dir) {
    if (current == TRON2_UP && new_dir == TRON2_DOWN) return 1;
    if (current == TRON2_DOWN && new_dir == TRON2_UP) return 1;
    if (current == TRON2_LEFT && new_dir == TRON2_RIGHT) return 1;
    if (current == TRON2_RIGHT && new_dir == TRON2_LEFT) return 1;
    return 0;
}

static uint8_t is_trail_at(uint8_t x, uint8_t y) {
    uint8_t result = 0;

    // PROTECCIÓN: Acceso al trail con mutex
    mutex_lock(&tron2_trail_mutex);

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

    mutex_unlock(&tron2_trail_mutex);
    return result;
}

static void add_to_trail_p1(uint8_t x, uint8_t y) {
    // PROTECCIÓN: Modificación del trail con mutex
    mutex_lock(&tron2_trail_mutex);

    if (trail_length_p1 < MAX_TRAIL_LENGTH) {
        light_trail_p1[trail_length_p1].x = x;
        light_trail_p1[trail_length_p1].y = y;
        trail_length_p1++;
    }

    mutex_unlock(&tron2_trail_mutex);
}

static void add_to_trail_p2(uint8_t x, uint8_t y) {
    // PROTECCIÓN: Modificación del trail con mutex
    mutex_lock(&tron2_trail_mutex);

    if (trail_length_p2 < MAX_TRAIL_LENGTH) {
        light_trail_p2[trail_length_p2].x = x;
        light_trail_p2[trail_length_p2].y = y;
        trail_length_p2++;
    }

    mutex_unlock(&tron2_trail_mutex);
}

/* ============================================================ */
/*          TAREAS DEL JUEGO                                   */
/* ============================================================ */

void tron2_input_task(void) {
    static uint32_t last_count_p1_left = 0;
    static uint32_t last_count_p1_up = 0;
    static uint32_t last_count_p1_down = 0;
    static uint32_t last_count_p1_right = 0;
    static uint32_t last_count_p2_left = 0;
    static uint32_t last_count_p2_up = 0;
    static uint32_t last_count_p2_down = 0;
    static uint32_t last_count_p2_right = 0;
    static uint8_t initialized = 0;

    if (!initialized) {
        last_count_p1_right = daos_btn_get_count_by_index(0);
        last_count_p1_down = daos_btn_get_count_by_index(1);
        last_count_p1_up = daos_btn_get_count_by_index(2);
        last_count_p1_left = daos_btn_get_count_by_index(3);
        last_count_p2_left = daos_btn_get_count_by_index(4);
        last_count_p2_up = daos_btn_get_count_by_index(5);
        last_count_p2_down = daos_btn_get_count_by_index(6);
        last_count_p2_right = daos_btn_get_count_by_index(7);
        initialized = 1;
        daos_sleep_ms(50);
        return;
    }

    // PROTECCIÓN: Verificar estado del juego de forma segura
    mutex_lock(&tron2_game_state_mutex);
    tron2_state_t current_state = game_state;
    mutex_unlock(&tron2_game_state_mutex);

    if (current_state == TRON2_GAME_OVER) {
        daos_sleep_ms(100);
        return;
    }

    // PROTECCIÓN: Lectura y escritura de direcciones con mutex
    mutex_lock(&tron2_bike_mutex);
    uint8_t local_alive_p1 = alive_p1;
    uint8_t local_alive_p2 = alive_p2;
    uint8_t local_dir_changed_p1 = dir_changed_p1;
    uint8_t local_dir_changed_p2 = dir_changed_p2;
    tron2_direction_t local_dir_p1 = dir_p1;
    tron2_direction_t local_dir_p2 = dir_p2;
    mutex_unlock(&tron2_bike_mutex);

    // JUGADOR 1
    if (local_alive_p1 && !local_dir_changed_p1) {
        uint32_t current_count_p1_right = daos_btn_get_count_by_index(0);
        uint32_t current_count_p1_down = daos_btn_get_count_by_index(1);
        uint32_t current_count_p1_up = daos_btn_get_count_by_index(2);
        uint32_t current_count_p1_left = daos_btn_get_count_by_index(3);

        if (current_count_p1_left != last_count_p1_left) {
            if (!is_opposite_direction(local_dir_p1, TRON2_LEFT)) {
                mutex_lock(&tron2_bike_mutex);
                pending_dir_p1 = TRON2_LEFT;
                dir_changed_p1 = 1;
                mutex_unlock(&tron2_bike_mutex);
            }
            last_count_p1_left = current_count_p1_left;
        } else if (current_count_p1_up != last_count_p1_up) {
            if (!is_opposite_direction(local_dir_p1, TRON2_UP)) {
                mutex_lock(&tron2_bike_mutex);
                pending_dir_p1 = TRON2_UP;
                dir_changed_p1 = 1;
                mutex_unlock(&tron2_bike_mutex);
            }
            last_count_p1_up = current_count_p1_up;
        } else if (current_count_p1_down != last_count_p1_down) {
            if (!is_opposite_direction(local_dir_p1, TRON2_DOWN)) {
                mutex_lock(&tron2_bike_mutex);
                pending_dir_p1 = TRON2_DOWN;
                dir_changed_p1 = 1;
                mutex_unlock(&tron2_bike_mutex);
            }
            last_count_p1_down = current_count_p1_down;
        } else if (current_count_p1_right != last_count_p1_right) {
            if (!is_opposite_direction(local_dir_p1, TRON2_RIGHT)) {
                mutex_lock(&tron2_bike_mutex);
                pending_dir_p1 = TRON2_RIGHT;
                dir_changed_p1 = 1;
                mutex_unlock(&tron2_bike_mutex);
            }
            last_count_p1_right = current_count_p1_right;
        }
    }

    // JUGADOR 2
    if (local_alive_p2 && !local_dir_changed_p2) {
        uint32_t current_count_p2_left = daos_btn_get_count_by_index(4);
        uint32_t current_count_p2_up = daos_btn_get_count_by_index(5);
        uint32_t current_count_p2_down = daos_btn_get_count_by_index(6);
        uint32_t current_count_p2_right = daos_btn_get_count_by_index(7);

        if (current_count_p2_left != last_count_p2_left) {
            if (!is_opposite_direction(local_dir_p2, TRON2_LEFT)) {
                mutex_lock(&tron2_bike_mutex);
                pending_dir_p2 = TRON2_LEFT;
                dir_changed_p2 = 1;
                mutex_unlock(&tron2_bike_mutex);
            }
            last_count_p2_left = current_count_p2_left;
        } else if (current_count_p2_up != last_count_p2_up) {
            if (!is_opposite_direction(local_dir_p2, TRON2_UP)) {
                mutex_lock(&tron2_bike_mutex);
                pending_dir_p2 = TRON2_UP;
                dir_changed_p2 = 1;
                mutex_unlock(&tron2_bike_mutex);
            }
            last_count_p2_up = current_count_p2_up;
        } else if (current_count_p2_down != last_count_p2_down) {
            if (!is_opposite_direction(local_dir_p2, TRON2_DOWN)) {
                mutex_lock(&tron2_bike_mutex);
                pending_dir_p2 = TRON2_DOWN;
                dir_changed_p2 = 1;
                mutex_unlock(&tron2_bike_mutex);
            }
            last_count_p2_down = current_count_p2_down;
        } else if (current_count_p2_right != last_count_p2_right) {
            if (!is_opposite_direction(local_dir_p2, TRON2_RIGHT)) {
                mutex_lock(&tron2_bike_mutex);
                pending_dir_p2 = TRON2_RIGHT;
                dir_changed_p2 = 1;
                mutex_unlock(&tron2_bike_mutex);
            }
            last_count_p2_right = current_count_p2_right;
        }
    }

    daos_sleep_ms(50);
}

void tron2_logic_task(void) {
    // PROTECCIÓN: Verificar estado del juego
    mutex_lock(&tron2_game_state_mutex);
    tron2_state_t current_state = game_state;
    mutex_unlock(&tron2_game_state_mutex);

    if (current_state == TRON2_GAME_OVER) {
        daos_sleep_ms(200);
        return;
    }

    // PROTECCIÓN: Actualizar direcciones de forma segura
    mutex_lock(&tron2_bike_mutex);
    if (dir_changed_p1) {
        dir_p1 = pending_dir_p1;
        dir_changed_p1 = 0;
    }
    if (dir_changed_p2) {
        dir_p2 = pending_dir_p2;
        dir_changed_p2 = 0;
    }

    uint8_t local_alive_p1 = alive_p1;
    uint8_t local_alive_p2 = alive_p2;
    Position local_bike_p1 = bike_p1;
    Position local_bike_p2 = bike_p2;
    tron2_direction_t local_dir_p1 = dir_p1;
    tron2_direction_t local_dir_p2 = dir_p2;
    mutex_unlock(&tron2_bike_mutex);

    // Agregar posiciones actuales al trail
    if (local_alive_p1) add_to_trail_p1(local_bike_p1.x, local_bike_p1.y);
    if (local_alive_p2) add_to_trail_p2(local_bike_p2.x, local_bike_p2.y);

    Position new_p1 = local_bike_p1;
    Position new_p2 = local_bike_p2;

    // Calcular nuevas posiciones
    if (local_alive_p1) {
        switch(local_dir_p1) {
            case TRON2_UP:    if (new_p1.y > 0) new_p1.y--; else new_p1.y = 255; break;
            case TRON2_DOWN:  new_p1.y++; break;
            case TRON2_LEFT:  if (new_p1.x > 0) new_p1.x--; else new_p1.x = 255; break;
            case TRON2_RIGHT: new_p1.x++; break;
        }
    }

    if (local_alive_p2) {
        switch(local_dir_p2) {
            case TRON2_UP:    if (new_p2.y > 0) new_p2.y--; else new_p2.y = 255; break;
            case TRON2_DOWN:  new_p2.y++; break;
            case TRON2_LEFT:  if (new_p2.x > 0) new_p2.x--; else new_p2.x = 255; break;
            case TRON2_RIGHT: new_p2.x++; break;
        }
    }

    // Verificar colisiones
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

    // PROTECCIÓN: Actualizar estado global de forma segura
    mutex_lock(&tron2_bike_mutex);
    bike_p1 = local_bike_p1;
    bike_p2 = local_bike_p2;
    alive_p1 = local_alive_p1;
    alive_p2 = local_alive_p2;
    mutex_unlock(&tron2_bike_mutex);

    uint8_t alive_count = local_alive_p1 + local_alive_p2;

    if (alive_count <= 1) {
        mutex_lock(&tron2_game_state_mutex);
        game_state = TRON2_GAME_OVER;

        if (alive_count == 0) {
            winner = TRON2_DRAW;
        } else {
            if (local_alive_p1) winner = TRON2_PLAYER1_WINS;
            else if (local_alive_p2) winner = TRON2_PLAYER2_WINS;
        }
        mutex_unlock(&tron2_game_state_mutex);
    }

    daos_sleep_ms(150);
}

void tron2_render_task(void) {
    static uint8_t first_frame = 1;
    static Position last_bike_p1 = {0, 0};
    static Position last_bike_p2 = {0, 0};

    if (first_frame) {
        daos_gfx_clear(DAOS_COLOR_BLACK);

        mutex_lock(&tron2_bike_mutex);
        last_bike_p1 = bike_p1;
        last_bike_p2 = bike_p2;
        mutex_unlock(&tron2_bike_mutex);

        first_frame = 0;
    }

    char buffer[32];

    // Dibujar scores de forma segura
    buffer[0] = 'P'; buffer[1] = '1'; buffer[2] = ':'; buffer[3] = '\0';
    daos_gfx_draw_text(5, 5, buffer, COLOR_P1, DAOS_COLOR_BLACK);
    buffer[0] = '0' + (distance_p1 / 100) % 10;
    buffer[1] = '0' + (distance_p1 / 10) % 10;
    buffer[2] = '0' + distance_p1 % 10;
    buffer[3] = '\0';
    daos_gfx_draw_text(25, 5, buffer, DAOS_COLOR_WHITE, DAOS_COLOR_BLACK);

    buffer[0] = 'P'; buffer[1] = '2'; buffer[2] = ':'; buffer[3] = '\0';
    daos_gfx_draw_text(230, 5, buffer, COLOR_P2, DAOS_COLOR_BLACK);
    buffer[0] = '0' + (distance_p2 / 100) % 10;
    buffer[1] = '0' + (distance_p2 / 10) % 10;
    buffer[2] = '0' + distance_p2 % 10;
    buffer[3] = '\0';
    daos_gfx_draw_text(250, 5, buffer, DAOS_COLOR_WHITE, DAOS_COLOR_BLACK);

    // Dibujar bordes del tablero
    daos_gfx_fill_rect(8, 28, BOARD_WIDTH * PIXEL_SIZE + 4, 2, DAOS_COLOR_WHITE);
    daos_gfx_fill_rect(8, 30 + BOARD_HEIGHT * PIXEL_SIZE, BOARD_WIDTH * PIXEL_SIZE + 4, 2, DAOS_COLOR_WHITE);
    daos_gfx_fill_rect(8, 28, 2, BOARD_HEIGHT * PIXEL_SIZE + 4, DAOS_COLOR_WHITE);
    daos_gfx_fill_rect(10 + BOARD_WIDTH * PIXEL_SIZE, 28, 2, BOARD_HEIGHT * PIXEL_SIZE + 4, DAOS_COLOR_WHITE);

    // PROTECCIÓN: Verificar estado del juego
    mutex_lock(&tron2_game_state_mutex);
    tron2_state_t current_state = game_state;
    tron2_winner_t current_winner = winner;
    mutex_unlock(&tron2_game_state_mutex);

    if (current_state == TRON2_GAME_OVER) {
        daos_gfx_fill_rect(60, 90, 200, 60, DAOS_COLOR_BLACK);
        daos_gfx_fill_rect(58, 88, 204, 64, DAOS_COLOR_WHITE);
        daos_gfx_fill_rect(60, 90, 200, 60, DAOS_COLOR_BLACK);

        daos_gfx_draw_text_large(80, 100, "GAME OVER", DAOS_COLOR_WHITE, DAOS_COLOR_BLACK, 2);

        switch(current_winner) {
            case TRON2_PLAYER1_WINS:
                daos_gfx_draw_text_large(90, 125, "P1 WINS", COLOR_P1, DAOS_COLOR_BLACK, 2);
                break;
            case TRON2_PLAYER2_WINS:
                daos_gfx_draw_text_large(90, 125, "P2 WINS", COLOR_P2, DAOS_COLOR_BLACK, 2);
                break;
            case TRON2_DRAW:
                daos_gfx_draw_text_large(100, 125, "DRAW", DAOS_COLOR_WHITE, DAOS_COLOR_BLACK, 2);
                break;
            default:
                break;
        }

        daos_sleep_ms(5000);
        return;
    }

    // PROTECCIÓN: Leer posiciones de motos de forma segura
    mutex_lock(&tron2_bike_mutex);
    Position current_bike_p1 = bike_p1;
    Position current_bike_p2 = bike_p2;
    uint8_t local_alive_p1 = alive_p1;
    uint8_t local_alive_p2 = alive_p2;
    tron2_direction_t local_dir_p1 = dir_p1;
    tron2_direction_t local_dir_p2 = dir_p2;
    mutex_unlock(&tron2_bike_mutex);

    // Borrar posiciones antiguas de las motos
    if (local_alive_p1 && (last_bike_p1.x != current_bike_p1.x || last_bike_p1.y != current_bike_p1.y)) {
        int screen_x = 10 + last_bike_p1.x * PIXEL_SIZE - SPRITE_OFFSET * SPRITE_SCALE;
        int screen_y = 30 + last_bike_p1.y * PIXEL_SIZE - SPRITE_OFFSET * SPRITE_SCALE;
        daos_gfx_fill_rect(screen_x, screen_y, SPRITE_SIZE * SPRITE_SCALE, SPRITE_SIZE * SPRITE_SCALE, DAOS_COLOR_BLACK);
    }

    if (local_alive_p2 && (last_bike_p2.x != current_bike_p2.x || last_bike_p2.y != current_bike_p2.y)) {
        int screen_x = 10 + last_bike_p2.x * PIXEL_SIZE - SPRITE_OFFSET * SPRITE_SCALE;
        int screen_y = 30 + last_bike_p2.y * PIXEL_SIZE - SPRITE_OFFSET * SPRITE_SCALE;
        daos_gfx_fill_rect(screen_x, screen_y, SPRITE_SIZE * SPRITE_SCALE, SPRITE_SIZE * SPRITE_SCALE, DAOS_COLOR_BLACK);
    }

    // PROTECCIÓN: Dibujar trails de forma segura
    mutex_lock(&tron2_trail_mutex);

    uint16_t trail_color_p1 = 0x01F3;
    for (uint16_t i = 0; i < trail_length_p1; i++) {
        int screen_x = 10 + light_trail_p1[i].x * PIXEL_SIZE;
        int screen_y = 30 + light_trail_p1[i].y * PIXEL_SIZE;
        daos_gfx_fill_rect(screen_x, screen_y, PIXEL_SIZE, PIXEL_SIZE, trail_color_p1);
    }

    uint16_t trail_color_p2 = 0xC800;
    for (uint16_t i = 0; i < trail_length_p2; i++) {
        int screen_x = 10 + light_trail_p2[i].x * PIXEL_SIZE;
        int screen_y = 30 + light_trail_p2[i].y * PIXEL_SIZE;
        daos_gfx_fill_rect(screen_x, screen_y, PIXEL_SIZE, PIXEL_SIZE, trail_color_p2);
    }

    mutex_unlock(&tron2_trail_mutex);

    // Dibujar moto del jugador 1
    if (local_alive_p1) {
        int screen_x = 10 + current_bike_p1.x * PIXEL_SIZE - SPRITE_OFFSET * SPRITE_SCALE;
        int screen_y = 30 + current_bike_p1.y * PIXEL_SIZE - SPRITE_OFFSET * SPRITE_SCALE;

        for (uint8_t sy = 0; sy < SPRITE_SIZE; sy++) {
            for (uint8_t sx = 0; sx < SPRITE_SIZE; sx++) {
                uint8_t rot_x = sx, rot_y = sy;
                switch(local_dir_p1) {
                    case TRON2_RIGHT: rot_x = sx; rot_y = sy; break;
                    case TRON2_LEFT: rot_x = (SPRITE_SIZE-1) - sx; rot_y = (SPRITE_SIZE-1) - sy; break;
                    case TRON2_DOWN: rot_x = sy; rot_y = (SPRITE_SIZE-1) - sx; break;
                    case TRON2_UP: rot_x = (SPRITE_SIZE-1) - sy; rot_y = sx; break;
                }

                uint32_t color = sprite_p1_data[rot_y * SPRITE_SIZE + rot_x];

                if (color != 0x00000000) {
                    uint8_t r = (color >> 16) & 0xFF;
                    uint8_t g = (color >> 8) & 0xFF;
                    uint8_t b = color & 0xFF;
                    uint16_t rgb565 = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);

                    daos_gfx_fill_rect(screen_x + sx * SPRITE_SCALE,
                                     screen_y + sy * SPRITE_SCALE,
                                     SPRITE_SCALE,
                                     SPRITE_SCALE,
                                     rgb565);
                }
            }
        }
        last_bike_p1 = current_bike_p1;
    }

    // Dibujar moto del jugador 2
    if (local_alive_p2) {
        int screen_x = 10 + current_bike_p2.x * PIXEL_SIZE - SPRITE_OFFSET * SPRITE_SCALE;
        int screen_y = 30 + current_bike_p2.y * PIXEL_SIZE - SPRITE_OFFSET * SPRITE_SCALE;

        for (uint8_t sy = 0; sy < SPRITE_SIZE; sy++) {
            for (uint8_t sx = 0; sx < SPRITE_SIZE; sx++) {
                uint8_t rot_x = sx, rot_y = sy;
                switch(local_dir_p2) {
                    case TRON2_LEFT: rot_x = sx; rot_y = sy; break;
                    case TRON2_RIGHT: rot_x = (SPRITE_SIZE-1) - sx; rot_y = (SPRITE_SIZE-1) - sy; break;
                    case TRON2_UP: rot_x = sy; rot_y = (SPRITE_SIZE-1) - sx; break;
                    case TRON2_DOWN: rot_x = (SPRITE_SIZE-1) - sy; rot_y = sx; break;
                }

                uint32_t color = sprite_p2_data[rot_y * SPRITE_SIZE + rot_x];

                if (color != 0x00000000) {
                    uint8_t r = (color >> 16) & 0xFF;
                    uint8_t g = (color >> 8) & 0xFF;
                    uint8_t b = color & 0xFF;
                    uint16_t rgb565 = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);

                    daos_gfx_fill_rect(screen_x + sx * SPRITE_SCALE,
                                     screen_y + sy * SPRITE_SCALE,
                                     SPRITE_SCALE,
                                     SPRITE_SCALE,
                                     rgb565);
                }
            }
        }
        last_bike_p2 = current_bike_p2;
    }

    daos_sleep_ms(100);
}

									 /* ============================================================ */
									 /*          INICIALIZACIÓN Y CLEANUP                           */
									 /* ============================================================ */

									 void tron2_reset(void) {
									     // PROTECCIÓN: Resetear el juego de forma segura
									     mutex_lock(&tron2_bike_mutex);
									     bike_p1.x = 10;  bike_p1.y = 15;  dir_p1 = TRON2_RIGHT;
									     bike_p2.x = 49;  bike_p2.y = 15;  dir_p2 = TRON2_LEFT;

									     pending_dir_p1 = TRON2_RIGHT;
									     pending_dir_p2 = TRON2_LEFT;
									     dir_changed_p1 = 0;
									     dir_changed_p2 = 0;

									     distance_p1 = distance_p2 = 0;
									     alive_p1 = alive_p2 = 1;
									     mutex_unlock(&tron2_bike_mutex);

									     mutex_lock(&tron2_game_state_mutex);
									     game_state = TRON2_RUNNING;
									     winner = TRON2_NO_WINNER;
									     mutex_unlock(&tron2_game_state_mutex);

									     mutex_lock(&tron2_trail_mutex);
									     trail_length_p1 = trail_length_p2 = 0;
									     mutex_unlock(&tron2_trail_mutex);
									 }

									 void tron2_init(void) {
									     // INICIALIZACIÓN: Configurar mutexes y semáforos
									     mutex_init(&tron2_game_state_mutex);
									     mutex_init(&tron2_trail_mutex);
									     mutex_init(&tron2_bike_mutex);

									     // Inicializar semáforos (NO SE USAN - se eliminaron las sincronizaciones bloqueantes)
									     // Los dejamos inicializados por si se necesitan en el futuro
									     sem_init(&tron2_input_ready_sem, 0, 1);
									     sem_init(&tron2_logic_ready_sem, 0, 1);
									     sem_init(&tron2_render_ready_sem, 0, 1);

									     // Resetear el estado del juego
									     tron2_reset();
									 }

									 void tron2_cleanup(void) {
									     // PROTECCIÓN: Limpiar de forma segura
									     mutex_lock(&tron2_game_state_mutex);
									     daos_gfx_clear(DAOS_COLOR_BLACK);
									     game_state = TRON2_GAME_OVER;
									     mutex_unlock(&tron2_game_state_mutex);

									     // Nota: Los mutexes y semáforos no necesitan destrucción explícita
									     // en este sistema embebido, pero podrían agregarse funciones
									     // mutex_destroy() y sem_destroy() si fuera necesario
									 }

									 /* ============================================================ */
									 /*          API PÚBLICA                                        */
									 /* ============================================================ */

									 tron2_state_t tron2_get_state(void) {
									     // PROTECCIÓN: Lectura segura del estado
									     mutex_lock(&tron2_game_state_mutex);
									     tron2_state_t state = game_state;
									     mutex_unlock(&tron2_game_state_mutex);
									     return state;
									 }

									 uint32_t tron2_get_score_p1(void) {
									     // PROTECCIÓN: Lectura segura del score
									     // distance_p1 es de tipo uint32_t, la lectura es atómica en ARM Cortex-M4
									     // pero por consistencia usamos mutex
									     mutex_lock(&tron2_bike_mutex);
									     uint32_t score = distance_p1;
									     mutex_unlock(&tron2_bike_mutex);
									     return score;
									 }

									 uint32_t tron2_get_score_p2(void) {
									     // PROTECCIÓN: Lectura segura del score
									     mutex_lock(&tron2_bike_mutex);
									     uint32_t score = distance_p2;
									     mutex_unlock(&tron2_bike_mutex);
									     return score;
									 }

									 tron2_winner_t tron2_get_winner(void) {
									     // PROTECCIÓN: Lectura segura del ganador
									     mutex_lock(&tron2_game_state_mutex);
									     tron2_winner_t w = winner;
									     mutex_unlock(&tron2_game_state_mutex);
									     return w;
									 }
