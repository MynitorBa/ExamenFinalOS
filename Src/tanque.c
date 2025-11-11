#include "tanque.h"
#include "api.h"
#include "sync.h"

// ========================================================================
// CONSTANTES - ESCALA Y DIMENSIÓN
// ========================================================================
#define TILE_SIZE 16
#define TILE_DATA_SIZE 10

#define MAP_WIDTH 19
#define MAP_HEIGHT 14

#define MAP_OFFSET_X 8
#define MAP_OFFSET_Y 8

// ========================================================================
// DIRECCIONES
// ========================================================================
typedef enum {
    DIR_UP = 0,
    DIR_RIGHT = 1,
    DIR_DOWN = 2,
    DIR_LEFT = 3
} Direccion;

// Sprites (sin cambios)
static const uint32_t sprite_bloque_destructible[100] = {
    0xff9d3312, 0xff8c2d10, 0xff8c2d10, 0xff8c2d10, 0xff8c2d10, 0xff9d3312, 0xff8c2d10, 0xff8c2d10, 0xff9d3312, 0xff9d3312,
    0xff8c2d10, 0xffffffff, 0xffffffff, 0xffcecece, 0xffcecece, 0xff8c2d10, 0xffffffff, 0xffcecece, 0xff8c2d10, 0xff9d3312,
    0xff9d3312, 0xff8c2d10, 0xff8c2d10, 0xff8c2d10, 0xff8c2d10, 0xff9d3312, 0xff8c2d10, 0xff8c2d10, 0xff9d3312, 0xff9d3312,
    0xff8c2d10, 0xffffffff, 0xffcecece, 0xff8c2d10, 0xffffffff, 0xffcecece, 0xffcecece, 0xff8c2d10, 0xffffffff, 0xff8c2d10,
    0xff9d3312, 0xff8c2d10, 0xff8c2d10, 0xff9d3312, 0xff8c2d10, 0xff8c2d10, 0xff8c2d10, 0xff9d3312, 0xff8c2d10, 0xff9d3312,
    0xff9d3312, 0xff8c2d10, 0xff8c2d10, 0xff8c2d10, 0xff8c2d10, 0xff9d3312, 0xff8c2d10, 0xff8c2d10, 0xff8c2d10, 0xff9d3312,
    0xff8c2d10, 0xffffffff, 0xffffffff, 0xffcecece, 0xffcecece, 0xff8c2d10, 0xffffffff, 0xffcecece, 0xffcecece, 0xff8c2d10,
    0xff9d3312, 0xff8c2d10, 0xff8c2d10, 0xff8c2d10, 0xff8c2d10, 0xff9d3312, 0xff8c2d10, 0xff8c2d10, 0xff8c2d10, 0xff9d3312,
    0xff8c2d10, 0xffffffff, 0xffcecece, 0xff8c2d10, 0xffffffff, 0xffcecece, 0xff8c2d10, 0xffffffff, 0xffcecece, 0xff8c2d10,
    0xff9d3312, 0xff8c2d10, 0xff8c2d10, 0xff9d3312, 0xff8c2d10, 0xff8c2d10, 0xff9d3312, 0xff8c2d10, 0xff8c2d10, 0xff9d3312
};

static const uint32_t sprite_bloque_acero[100] = {
    0xff7b7b7b, 0xff7b7b7b, 0xff7b7b7b, 0xff7b7b7b, 0xff7b7b7b, 0xff7b7b7b, 0xff7b7b7b, 0xff7b7b7b, 0xff7b7b7b, 0xff7b7b7b,
    0xff7b7b7b, 0xffaeaeae, 0xff494847, 0xff494847, 0xff7b7b7b, 0xff7b7b7b, 0xffaeaeae, 0xff494847, 0xff494847, 0xff7b7b7b,
    0xff7b7b7b, 0xff494847, 0xffaeaeae, 0xff494847, 0xff7b7b7b, 0xff7b7b7b, 0xff494847, 0xffaeaeae, 0xff494847, 0xff7b7b7b,
    0xff7b7b7b, 0xff494847, 0xffaeaeae, 0xff494847, 0xff7b7b7b, 0xff7b7b7b, 0xff494847, 0xffaeaeae, 0xff494847, 0xff7b7b7b,
    0xff7b7b7b, 0xff494847, 0xff494847, 0xffaeaeae, 0xff7b7b7b, 0xff7b7b7b, 0xff494847, 0xff494847, 0xffaeaeae, 0xff7b7b7b,
    0xff7b7b7b, 0xff7b7b7b, 0xff7b7b7b, 0xff7b7b7b, 0xff7b7b7b, 0xff7b7b7b, 0xff7b7b7b, 0xff7b7b7b, 0xff7b7b7b, 0xff7b7b7b,
    0xff7b7b7b, 0xffaeaeae, 0xff494847, 0xff494847, 0xff7b7b7b, 0xff7b7b7b, 0xffaeaeae, 0xff494847, 0xff494847, 0xff7b7b7b,
    0xff7b7b7b, 0xff494847, 0xffaeaeae, 0xff494847, 0xff7b7b7b, 0xff7b7b7b, 0xff494847, 0xffaeaeae, 0xff494847, 0xff7b7b7b,
    0xff7b7b7b, 0xff494847, 0xff494847, 0xffaeaeae, 0xff7b7b7b, 0xff7b7b7b, 0xff494847, 0xff494847, 0xffaeaeae, 0xff7b7b7b,
    0xff7b7b7b, 0xff7b7b7b, 0xff7b7b7b, 0xff7b7b7b, 0xff7b7b7b, 0xff7b7b7b, 0xff7b7b7b, 0xff7b7b7b, 0xff7b7b7b, 0xff7b7b7b
};

static const uint32_t sprite_planta[100] = {
    0x00000000, 0xff1f882d, 0xff29ac3a, 0x00000000, 0x00000000, 0xff1f882d, 0x00000000, 0xff1f882d, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xff29ac3a, 0xff74cc7f, 0x00000000, 0xff74cc7f, 0xff29ac3a, 0xff29ac3a, 0xff74cc7f, 0xff1f882d,
    0xff29ac3a, 0x00000000, 0x00000000, 0xff1f882d, 0xff29ac3a, 0xff29ac3a, 0x00000000, 0xff1f882d, 0x00000000, 0x00000000,
    0x00000000, 0xff74cc7f, 0xff1f882d, 0xff29ac3a, 0x00000000, 0x00000000, 0xff29ac3a, 0x00000000, 0xff74cc7f, 0xff1f882d,
    0xff74cc7f, 0x00000000, 0x00000000, 0xff29ac3a, 0xff29ac3a, 0xff29ac3a, 0x00000000, 0xff74cc7f, 0x00000000, 0x00000000,
    0xff1f882d, 0xff29ac3a, 0xff1f882d, 0x00000000, 0x00000000, 0xff29ac3a, 0x00000000, 0xff1f882d, 0x00000000, 0xff1f882d,
    0x00000000, 0xff1f882d, 0x00000000, 0x00000000, 0xff29ac3a, 0xff29ac3a, 0xff29ac3a, 0x00000000, 0xff29ac3a, 0x00000000,
    0xff29ac3a, 0x00000000, 0xff29ac3a, 0xff29ac3a, 0x00000000, 0xff74cc7f, 0x00000000, 0x00000000, 0xff74cc7f, 0x00000000,
    0xff29ac3a, 0xff29ac3a, 0x00000000, 0xff29ac3a, 0x00000000, 0xff1f882d, 0xff29ac3a, 0xff74cc7f, 0xff1f882d, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xff74cc7f, 0xff1f882d, 0x00000000, 0xff29ac3a, 0x00000000, 0x00000000, 0xff1f882d
};

// Tanque Azul
static const uint32_t sprite_tanque_azul[100] = {
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff42489f, 0xff42489f, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xff585fcc, 0xff585fcc, 0x00000000, 0x00000000, 0xff42489f, 0xff42489f, 0x00000000, 0x00000000, 0xff585fcc, 0xff585fcc,
    0xff19229b, 0xff19229b, 0xff171c63, 0x00000000, 0xff42489f, 0xff42489f, 0x00000000, 0xff171c63, 0xff19229b, 0xff19229b,
    0xff585fcc, 0xff585fcc, 0xff171c63, 0xff171c63, 0xff171c63, 0xff171c63, 0xff171c63, 0xff171c63, 0xff585fcc, 0xff585fcc,
    0xff19229b, 0xff19229b, 0xff171c63, 0xff171c63, 0xff2d3387, 0xff2d3387, 0xff171c63, 0xff171c63, 0xff19229b, 0xff19229b,
    0xff585fcc, 0xff585fcc, 0xff171c63, 0xff2d3387, 0xff2d3387, 0xff2d3387, 0xff2d3387, 0xff171c63, 0xff585fcc, 0xff585fcc,
    0xff19229b, 0xff19229b, 0xff171c63, 0xff171c63, 0xff2d3387, 0xff2d3387, 0xff171c63, 0xff171c63, 0xff19229b, 0xff19229b,
    0xff585fcc, 0xff585fcc, 0xff42434f, 0xff171c63, 0xff171c63, 0xff171c63, 0xff171c63, 0xff42434f, 0xff585fcc, 0xff585fcc,
    0xff19229b, 0xff19229b, 0xff42434f, 0xff42434f, 0xff171c63, 0xff171c63, 0xff42434f, 0xff42434f, 0xff19229b, 0xff19229b,
    0x00000000, 0x00000000, 0xff42434f, 0xff42434f, 0xff42434f, 0xff42434f, 0xff42434f, 0xff42434f, 0x00000000, 0x00000000
};

// Tanque Rojo
static const uint32_t sprite_tanque_rojo[100] = {
    0x00000000, 0x00000000, 0x00000000, 0xffb31c1c, 0x00000000, 0x00000000, 0xffb31c1c, 0x00000000, 0x00000000, 0x00000000,
    0xffe82323, 0x00000000, 0x00000000, 0x00000000, 0xffb31c1c, 0xffb31c1c, 0x00000000, 0x00000000, 0x00000000, 0xffe82323,
    0xffe82323, 0xffe82323, 0x00000000, 0x00000000, 0xffb31c1c, 0xffb31c1c, 0x00000000, 0x00000000, 0xffe82323, 0xffe82323,
    0xffbb2f2f, 0xffbb2f2f, 0xffa84444, 0x00000000, 0xffb31c1c, 0xffb31c1c, 0x00000000, 0xffa84444, 0xffbb2f2f, 0xffbb2f2f,
    0xffe82323, 0xffe82323, 0xffa84444, 0xffbb2f2f, 0xffb31c1c, 0xffb31c1c, 0xffa84444, 0xffa84444, 0xffe82323, 0xffe82323,
    0xffbb2f2f, 0xffbb2f2f, 0xffa84444, 0xffbb2f2f, 0xff9f6d6d, 0xff9f6d6d, 0xffa84444, 0xffa84444, 0xffbb2f2f, 0xffbb2f2f,
    0xffe82323, 0xffe82323, 0xffa84444, 0xff9f6d6d, 0xff9f6d6d, 0xff9f6d6d, 0xff9f6d6d, 0xffa84444, 0xffe82323, 0xffe82323,
    0xffbb2f2f, 0xffbb2f2f, 0xffa84444, 0xffa84444, 0xff9f6d6d, 0xff9f6d6d, 0xffbb2f2f, 0xffa84444, 0xffbb2f2f, 0xffbb2f2f,
    0xffe82323, 0xffe82323, 0xff565656, 0xffa84444, 0xffbb2f2f, 0xffbb2f2f, 0xffbb2f2f, 0xff565656, 0xffe82323, 0xffe82323,
    0x00000000, 0x00000000, 0xff565656, 0xff565656, 0xff565656, 0xff565656, 0xff565656, 0xff565656, 0x00000000, 0x00000000
};

// ========================================================================
// TIPOS
// ========================================================================
typedef enum {
    TILE_VACIO = 0,
    TILE_DESTRUCTIBLE,
    TILE_ACERO,
    TILE_PLANTA
} TipoTile;

typedef struct {
    uint8_t num_destructibles;
    uint8_t num_acero;
    uint8_t num_plantas;
    uint8_t densidad_pasillos;
    uint8_t num_conectores;
    uint8_t prob_agrupacion;
} ConfigMapa;

typedef struct {
    uint8_t x;
    uint8_t y;
    Direccion dir;
    uint8_t alive;
} Tanque;

typedef struct {
    int8_t x;
    int8_t y;
    Direccion dir;
    uint8_t active;
    uint8_t move_counter;
    uint8_t is_player_one;
} Proyectil;

typedef enum {
    GAME_STATE_RUNNING = 0,
    GAME_STATE_OVER
} GameState;

typedef enum {
    WINNER_NONE = 0,
    WINNER_P1_AZUL,
    WINNER_P2_ROJO,
    WINNER_DRAW
} GameWinner;

// ========================================================================
// VARIABLES GLOBALES
// ========================================================================
static GameState game_state = GAME_STATE_RUNNING;
static GameWinner winner = WINNER_NONE;

static uint8_t game_running = 0;
static TipoTile mapa[MAP_HEIGHT][MAP_WIDTH];
static uint8_t first_draw = 1;
static ConfigMapa config_actual;

static Tanque tanque_azul;
static Tanque tanque_rojo;

#define MAX_PROJECTILES 3
static Proyectil proyectiles[MAX_PROJECTILES];

#define TANK_MOVE_CYCLES 3
#define PROJECTILE_MOVE_CYCLES 1
#define BUTTON_REPEAT_DELAY 8

static uint8_t move_counter_azul = 0;
static uint8_t move_counter_rojo = 0;

static uint8_t move_button_hold_p1 = 0;
static uint8_t move_button_hold_p2 = 0;

#define COLOR_BLACK  DAOS_COLOR_BLACK
#define COLOR_WHITE  DAOS_COLOR_WHITE
#define COLOR_YELLOW DAOS_COLOR_YELLOW
#define COLOR_AZUL   DAOS_COLOR_BLUE
#define COLOR_ROJO   DAOS_COLOR_RED

/* ============================================================ */
/*          MUTEXES Y SEMÁFOROS PARA SINCRONIZACIÓN            */
/* ============================================================ */

// Mutexes para protección de recursos compartidos
mutex_t tanque_game_state_mutex;
mutex_t tanque_map_mutex;
mutex_t tanque_tank_mutex;
mutex_t tanque_projectile_mutex;

// Semáforos para sincronización de tareas
sem_t tanque_input_ready_sem;
sem_t tanque_logic_ready_sem;
sem_t tanque_render_ready_sem;

// ========================================================================
// FUNCIONES HELPER GLOBALES (Declaraciones)
// ========================================================================
static void clear_tank_position(uint8_t x, uint8_t y);
static void redraw_tile_content(uint8_t x, uint8_t y);
static uint8_t can_move_to(uint8_t x, uint8_t y);
static uint8_t try_move_tank(Tanque* tank, uint8_t* move_counter);
static void check_game_over(void);

// ========================================================================
// FUNCIONES HELPER DE ALEATORIEDAD
// ========================================================================
static inline uint8_t random_range(uint8_t min, uint8_t max) {
    return (uint8_t)daos_random_range(min, max);
}

// ========================================================================
// FUNCIONES DE DIBUJO Y MANEJO DE TILES
// ========================================================================
static void draw_horizontal_line(int16_t x0, int16_t y, int16_t x1, uint16_t color) {
    if (x0 > x1) { int16_t temp = x0; x0 = x1; x1 = temp; }
    if (y < 0 || y >= 240) return;
    x0 = (x0 < 0) ? 0 : x0;
    x1 = (x1 >= 320) ? 319 : x1;
    for (int16_t x = x0; x <= x1; x++) {
        daos_gfx_draw_pixel(x, y, color);
    }
}

static void draw_vertical_line(int16_t x, int16_t y0, int16_t y1, uint16_t color) {
    if (y0 > y1) { int16_t temp = y0; y0 = y1; y1 = temp; }
    if (x < 0 || x >= 320) return;
    y0 = (y0 < 0) ? 0 : y0;
    y1 = (y1 >= 240) ? 239 : y1;
    for (int16_t y = y0; y <= y1; y++) {
        daos_gfx_draw_pixel(x, y, color);
    }
}

static void draw_tile_rotated(uint8_t x, uint8_t y, const uint32_t* sprite_data, Direccion dir) {
    int16_t screen_x = (x * TILE_SIZE) + MAP_OFFSET_X;
    int16_t screen_y = (y * TILE_SIZE) + MAP_OFFSET_Y;

    for (uint8_t row = 0; row < TILE_SIZE; row++) {
        for (uint8_t col = 0; col < TILE_SIZE; col++) {
            uint8_t data_row, data_col;
            uint8_t mapped_row = (row * TILE_DATA_SIZE) / TILE_SIZE;
            uint8_t mapped_col = (col * TILE_DATA_SIZE) / TILE_SIZE;

            if (mapped_row >= TILE_DATA_SIZE) mapped_row = TILE_DATA_SIZE - 1;
            if (mapped_col >= TILE_DATA_SIZE) mapped_col = TILE_DATA_SIZE - 1;

            switch(dir) {
                case DIR_UP:
                    data_row = mapped_row;
                    data_col = mapped_col;
                    break;
                case DIR_RIGHT:
                    data_row = (TILE_DATA_SIZE - 1) - mapped_col;
                    data_col = mapped_row;
                    break;
                case DIR_DOWN:
                    data_row = (TILE_DATA_SIZE - 1) - mapped_row;
                    data_col = (TILE_DATA_SIZE - 1) - mapped_col;
                    break;
                case DIR_LEFT:
                    data_row = mapped_col;
                    data_col = (TILE_DATA_SIZE - 1) - mapped_row;
                    break;
            }

            uint32_t color_32bit = sprite_data[data_row * TILE_DATA_SIZE + data_col];

            if ((color_32bit >> 24) != 0x00) {
                uint8_t r = (color_32bit >> 16) & 0xFF;
                uint8_t g = (color_32bit >> 8) & 0xFF;
                uint8_t b = color_32bit & 0xFF;
                uint16_t color_16bit = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);

                int16_t pixel_x = screen_x + col;
                int16_t pixel_y = screen_y + row;

                if (pixel_x >= 0 && pixel_x < 320 && pixel_y >= 0 && pixel_y < 240) {
                    daos_gfx_draw_pixel(pixel_x, pixel_y, color_16bit);
                }
            }
        }
    }
}

static void draw_tile(uint8_t x, uint8_t y, const uint32_t* sprite_data) {
    draw_tile_rotated(x, y, sprite_data, DIR_UP);
}

static void draw_projectile(uint8_t x, uint8_t y) {
    int16_t screen_x = (x * TILE_SIZE) + MAP_OFFSET_X;
    int16_t screen_y = (y * TILE_SIZE) + MAP_OFFSET_Y;

    uint8_t size = 4;
    int16_t start_x = screen_x + (TILE_SIZE / 2) - (size / 2);
    int16_t start_y = screen_y + (TILE_SIZE / 2) - (size / 2);

    for (int16_t i = 0; i < size; i++) {
        for (int16_t j = 0; j < size; j++) {
            daos_gfx_draw_pixel(start_x + i, start_y + j, COLOR_YELLOW);
        }
    }
}

static void redraw_tile_content(uint8_t x, uint8_t y) {
    int16_t screen_x = (x * TILE_SIZE) + MAP_OFFSET_X;
    int16_t screen_y = (y * TILE_SIZE) + MAP_OFFSET_Y;

    for (uint8_t row = 0; row < TILE_SIZE; row++) {
        for (uint8_t col = 0; col < TILE_SIZE; col++) {
             daos_gfx_draw_pixel(screen_x + col, screen_y + row, COLOR_BLACK);
        }
    }

    // PROTECCIÓN: Acceso al mapa con mutex
    mutex_lock(&tanque_map_mutex);
    TipoTile tile = mapa[y][x];
    mutex_unlock(&tanque_map_mutex);

    switch(tile) {
        case TILE_DESTRUCTIBLE:
            draw_tile(x, y, sprite_bloque_destructible);
            break;
        case TILE_ACERO:
            draw_tile(x, y, sprite_bloque_acero);
            break;
        case TILE_PLANTA:
            draw_tile(x, y, sprite_planta);
            break;
        default:
            break;
    }

    int16_t map_end_x = MAP_OFFSET_X + (MAP_WIDTH * TILE_SIZE) - 1;
    int16_t map_end_y = MAP_OFFSET_Y + (MAP_HEIGHT * TILE_SIZE) - 1;

    if (x == 0) draw_vertical_line(MAP_OFFSET_X, MAP_OFFSET_Y, map_end_y, COLOR_WHITE);
    if (x == MAP_WIDTH - 1) draw_vertical_line(map_end_x, MAP_OFFSET_Y, map_end_y, COLOR_WHITE);
    if (y == 0) draw_horizontal_line(MAP_OFFSET_X, MAP_OFFSET_Y, map_end_x, COLOR_WHITE);
    if (y == MAP_HEIGHT - 1) draw_horizontal_line(MAP_OFFSET_X, map_end_y, map_end_x, COLOR_WHITE);
}

static void clear_tank_position(uint8_t x, uint8_t y) {
    redraw_tile_content(x, y);
}

// ========================================================================
// VERIFICAR SI SE PUEDE MOVER
// ========================================================================
static uint8_t can_move_to(uint8_t x, uint8_t y) {
    if (x >= MAP_WIDTH || y >= MAP_HEIGHT) return 0;

    // PROTECCIÓN: Acceso al mapa con mutex
    mutex_lock(&tanque_map_mutex);
    TipoTile tile = mapa[y][x];
    mutex_unlock(&tanque_map_mutex);

    return (tile == TILE_VACIO || tile == TILE_PLANTA);
}

// ========================================================================
// MOVER TANQUE
// ========================================================================
static uint8_t try_move_tank(Tanque* tank, uint8_t* move_counter) {
    // PROTECCIÓN: Acceso a tanques con mutex
    mutex_lock(&tanque_tank_mutex);

    if (!tank->alive) {
        mutex_unlock(&tanque_tank_mutex);
        return 0;
    }

    (*move_counter)++;
    if (*move_counter < TANK_MOVE_CYCLES) {
        mutex_unlock(&tanque_tank_mutex);
        return 0;
    }

    *move_counter = 0;

    uint8_t old_x = tank->x;
    uint8_t old_y = tank->y;
    uint8_t new_x = tank->x;
    uint8_t new_y = tank->y;

    switch(tank->dir) {
        case DIR_UP:
            if (new_y > 0) new_y--;
            break;
        case DIR_DOWN:
        	if (new_y < MAP_HEIGHT - 1) new_y++;
        	            break;
        	        case DIR_LEFT:
        	            if (new_x > 0) new_x--;
        	            break;
        	        case DIR_RIGHT:
        	            if (new_x < MAP_WIDTH - 1) new_x++;
        	            break;
        	    }

        	    // Verificar colisión con otros tanques
        	    if (tanque_azul.alive && tanque_rojo.alive) {
        	        if (new_x == tanque_azul.x && new_y == tanque_azul.y && tank != &tanque_azul) {
        	            mutex_unlock(&tanque_tank_mutex);
        	            return 0;
        	        }
        	        if (new_x == tanque_rojo.x && new_y == tanque_rojo.y && tank != &tanque_rojo) {
        	            mutex_unlock(&tanque_tank_mutex);
        	            return 0;
        	        }
        	    }

        	    mutex_unlock(&tanque_tank_mutex);

        	    if (can_move_to(new_x, new_y)) {
        	        clear_tank_position(old_x, old_y);

        	        mutex_lock(&tanque_tank_mutex);
        	        tank->x = new_x;
        	        tank->y = new_y;
        	        mutex_unlock(&tanque_tank_mutex);

        	        return 1;
        	    }

        	    return 0;
        	}

        	// ========================================================================
        	// MANEJO DEL ESTADO DEL JUEGO
        	// ========================================================================
        	static void check_game_over(void) {
        	    // PROTECCIÓN: Verificar estado del juego
        	    mutex_lock(&tanque_game_state_mutex);
        	    if (game_state == GAME_STATE_OVER) {
        	        mutex_unlock(&tanque_game_state_mutex);
        	        return;
        	    }
        	    mutex_unlock(&tanque_game_state_mutex);

        	    // PROTECCIÓN: Leer estado de tanques
        	    mutex_lock(&tanque_tank_mutex);
        	    uint8_t p1_alive = tanque_azul.alive;
        	    uint8_t p2_alive = tanque_rojo.alive;
        	    mutex_unlock(&tanque_tank_mutex);

        	    if (!p1_alive && !p2_alive) {
        	        mutex_lock(&tanque_game_state_mutex);
        	        winner = WINNER_DRAW;
        	        game_state = GAME_STATE_OVER;
        	        mutex_unlock(&tanque_game_state_mutex);
        	    } else if (!p1_alive && p2_alive) {
        	        mutex_lock(&tanque_game_state_mutex);
        	        winner = WINNER_P2_ROJO;
        	        game_state = GAME_STATE_OVER;
        	        mutex_unlock(&tanque_game_state_mutex);
        	    } else if (p1_alive && !p2_alive) {
        	        mutex_lock(&tanque_game_state_mutex);
        	        winner = WINNER_P1_AZUL;
        	        game_state = GAME_STATE_OVER;
        	        mutex_unlock(&tanque_game_state_mutex);
        	    }

        	    mutex_lock(&tanque_game_state_mutex);
        	    if (game_state == GAME_STATE_OVER) {
        	        mutex_unlock(&tanque_game_state_mutex);
        	        daos_gfx_clear(COLOR_BLACK);
        	    } else {
        	        mutex_unlock(&tanque_game_state_mutex);
        	    }
        	}

        	// ========================================================================
        	// PROYECTILES
        	// ========================================================================
        	static void shoot_projectile(uint8_t start_x, uint8_t start_y, Direccion dir, uint8_t is_player_one) {
        	    // PROTECCIÓN: Verificar estado del juego
        	    mutex_lock(&tanque_game_state_mutex);
        	    if (game_state == GAME_STATE_OVER) {
        	        mutex_unlock(&tanque_game_state_mutex);
        	        return;
        	    }
        	    mutex_unlock(&tanque_game_state_mutex);

        	    int8_t px = start_x;
        	    int8_t py = start_y;

        	    switch (dir) {
        	        case DIR_UP: py--; break;
        	        case DIR_DOWN: py++; break;
        	        case DIR_LEFT: px--; break;
        	        case DIR_RIGHT: px++; break;
        	    }

        	    if (px < 0 || px >= MAP_WIDTH || py < 0 || py >= MAP_HEIGHT) {
        	        return;
        	    }

        	    // PROTECCIÓN: Acceso al mapa
        	    mutex_lock(&tanque_map_mutex);
        	    TipoTile target_tile = mapa[py][px];

        	    if (target_tile == TILE_ACERO) {
        	        mutex_unlock(&tanque_map_mutex);
        	        return;
        	    }

        	    if (target_tile == TILE_DESTRUCTIBLE) {
        	        mapa[py][px] = TILE_VACIO;
        	        mutex_unlock(&tanque_map_mutex);
        	        redraw_tile_content(px, py);
        	        return;
        	    }
        	    mutex_unlock(&tanque_map_mutex);

        	    // PROTECCIÓN: Crear proyectil
        	    mutex_lock(&tanque_projectile_mutex);
        	    for (uint8_t i = 0; i < MAX_PROJECTILES; i++) {
        	        if (!proyectiles[i].active) {
        	            proyectiles[i].x = px;
        	            proyectiles[i].y = py;
        	            proyectiles[i].dir = dir;
        	            proyectiles[i].active = 1;
        	            proyectiles[i].move_counter = 0;
        	            proyectiles[i].is_player_one = is_player_one;
        	            mutex_unlock(&tanque_projectile_mutex);
        	            return;
        	        }
        	    }
        	    mutex_unlock(&tanque_projectile_mutex);
        	}

        	static uint8_t update_projectile_movement(Proyectil* p) {
        	    p->move_counter++;
        	    if (p->move_counter < PROJECTILE_MOVE_CYCLES) {
        	        return 0;
        	    }
        	    p->move_counter = 0;

        	    int8_t old_x = p->x;
        	    int8_t old_y = p->y;
        	    int8_t new_x = p->x;
        	    int8_t new_y = p->y;

        	    switch (p->dir) {
        	        case DIR_UP: new_y--; break;
        	        case DIR_DOWN: new_y++; break;
        	        case DIR_LEFT: new_x--; break;
        	        case DIR_RIGHT: new_x++; break;
        	    }

        	    if (new_x < 0 || new_x >= MAP_WIDTH || new_y < 0 || new_y >= MAP_HEIGHT) {
        	        return 1;
        	    }

        	    // PROTECCIÓN: Acceso al mapa
        	    mutex_lock(&tanque_map_mutex);
        	    TipoTile target_tile = mapa[new_y][new_x];

        	    if (target_tile == TILE_ACERO) {
        	        mutex_unlock(&tanque_map_mutex);
        	        return 1;
        	    }

        	    if (target_tile == TILE_DESTRUCTIBLE) {
        	        mapa[new_y][new_x] = TILE_VACIO;
        	        mutex_unlock(&tanque_map_mutex);
        	        redraw_tile_content(new_x, new_y);
        	        return 1;
        	    }
        	    mutex_unlock(&tanque_map_mutex);

        	    // PROTECCIÓN: Verificar colisión con tanques
        	    mutex_lock(&tanque_tank_mutex);
        	    if (p->is_player_one) {
        	        if (tanque_rojo.alive && new_x == tanque_rojo.x && new_y == tanque_rojo.y) {
        	            tanque_rojo.alive = 0;
        	            uint8_t tx = tanque_rojo.x;
        	            uint8_t ty = tanque_rojo.y;
        	            mutex_unlock(&tanque_tank_mutex);
        	            clear_tank_position(tx, ty);
        	            check_game_over();
        	            return 1;
        	        }
        	    } else {
        	        if (tanque_azul.alive && new_x == tanque_azul.x && new_y == tanque_azul.y) {
        	            tanque_azul.alive = 0;
        	            uint8_t tx = tanque_azul.x;
        	            uint8_t ty = tanque_azul.y;
        	            mutex_unlock(&tanque_tank_mutex);
        	            clear_tank_position(tx, ty);
        	            check_game_over();
        	            return 1;
        	        }
        	    }
        	    mutex_unlock(&tanque_tank_mutex);

        	    p->x = new_x;
        	    p->y = new_y;

        	    redraw_tile_content(old_x, old_y);

        	    return 0;
        	}

        	// ========================================================================
        	// GENERACIÓN DE MAPA
        	// ========================================================================
        	static void generar_config_aleatoria(ConfigMapa* cfg) {
        	    cfg->num_destructibles = random_range(20, 35);
        	    cfg->num_acero = random_range(8, 15);
        	    cfg->num_plantas = random_range(4, 10);
        	    cfg->densidad_pasillos = random_range(50, 90);
        	    cfg->num_conectores = random_range(3, 8);
        	    cfg->prob_agrupacion = random_range(30, 70);
        	}

        	static void generar_mapa_con_config(const ConfigMapa* cfg) {
        	    // PROTECCIÓN: Modificar mapa con mutex
        	    mutex_lock(&tanque_map_mutex);

        	    for (uint8_t y = 0; y < MAP_HEIGHT; y++) {
        	        for (uint8_t x = 0; x < MAP_WIDTH; x++) {
        	            mapa[y][x] = TILE_VACIO;
        	        }
        	    }

        	    uint8_t count = 0;
        	    uint16_t intentos = 0;
        	    while (count < cfg->num_destructibles && intentos < 200) {
        	        uint8_t x = random_range(0, MAP_WIDTH - 1);
        	        uint8_t y = random_range(0, MAP_HEIGHT - 1);

        	        if (mapa[y][x] == TILE_VACIO) {
        	            mapa[y][x] = TILE_DESTRUCTIBLE;
        	            count++;

        	            if (random_range(0, 99) < cfg->prob_agrupacion) {
        	                int8_t dx[] = {0, 1, 0, -1};
        	                int8_t dy[] = {-1, 0, 1, 0};
        	                uint8_t dir = random_range(0, 3);
        	                int8_t nx = x + dx[dir];
        	                int8_t ny = y + dy[dir];

        	                if (nx >= 0 && nx < MAP_WIDTH && ny >= 0 && ny < MAP_HEIGHT &&
        	                    mapa[ny][nx] == TILE_VACIO && count < cfg->num_destructibles) {
        	                    mapa[ny][nx] = TILE_DESTRUCTIBLE;
        	                    count++;
        	                }
        	            }
        	        }
        	        intentos++;
        	    }

        	    count = 0;
        	    intentos = 0;
        	    while (count < cfg->num_acero && intentos < 150) {
        	        uint8_t x = random_range(0, MAP_WIDTH - 1);
        	        uint8_t y = random_range(0, MAP_HEIGHT - 1);

        	        if (mapa[y][x] == TILE_VACIO) {
        	            mapa[y][x] = TILE_ACERO;
        	            count++;
        	        }
        	        intentos++;
        	    }

        	    uint8_t espaciado_h = random_range(4, 6);
        	    for (uint8_t y = 2; y < MAP_HEIGHT; y += espaciado_h) {
        	        for (uint8_t x = 0; x < MAP_WIDTH; x++) {
        	            mapa[y][x] = TILE_VACIO;
        	            if (y + 1 < MAP_HEIGHT && random_range(0, 99) < cfg->densidad_pasillos) {
        	                mapa[y + 1][x] = TILE_VACIO;
        	            }
        	        }
        	    }

        	    uint8_t espaciado_v = random_range(4, 6);
        	    for (uint8_t x = 2; x < MAP_WIDTH; x += espaciado_v) {
        	        for (uint8_t y = 0; y < MAP_HEIGHT; y++) {
        	            mapa[y][x] = TILE_VACIO;
        	            if (x + 1 < MAP_WIDTH && random_range(0, 99) < cfg->densidad_pasillos) {
        	                mapa[y][x + 1] = TILE_VACIO;
        	            }
        	        }
        	    }

        	    for (uint8_t i = 0; i < cfg->num_conectores; i++) {
        	        uint8_t x = random_range(0, MAP_WIDTH - 1);
        	        uint8_t y = random_range(0, MAP_HEIGHT - 1);
        	        uint8_t dir = random_range(0, 1);
        	        uint8_t longitud = random_range(3, 8);

        	        if (dir == 0) {
        	            for (uint8_t j = 0; j < longitud && x + j < MAP_WIDTH; j++) {
        	                mapa[y][x + j] = TILE_VACIO;
        	            }
        	        } else {
        	            for (uint8_t j = 0; j < longitud && y + j < MAP_HEIGHT; j++) {
        	                mapa[y + j][x] = TILE_VACIO;
        	            }
        	        }
        	    }

        	    count = 0;
        	    intentos = 0;
        	    while (count < cfg->num_plantas && intentos < 200) {
        	        uint8_t x = random_range(0, MAP_WIDTH - 1);
        	        uint8_t y = random_range(0, MAP_HEIGHT - 1);

        	        if (random_range(0, 99) < 60) {
        	            if (x + 1 < MAP_WIDTH && y + 1 < MAP_HEIGHT &&
        	                mapa[y][x] == TILE_VACIO &&
        	                mapa[y][x+1] == TILE_VACIO &&
        	                mapa[y+1][x] == TILE_VACIO &&
        	                mapa[y+1][x+1] == TILE_VACIO) {

        	                mapa[y][x] = TILE_PLANTA;
        	                mapa[y][x+1] = TILE_PLANTA;
        	                mapa[y+1][x] = TILE_PLANTA;
        	                mapa[y+1][x+1] = TILE_PLANTA;
        	                count += 4;
        	            }
        	        } else if (mapa[y][x] == TILE_VACIO) {
        	            mapa[y][x] = TILE_PLANTA;
        	            count++;
        	        }
        	        intentos++;
        	    }

        	    mutex_unlock(&tanque_map_mutex);

        	    // PROTECCIÓN: Posicionar tanques
        	    mutex_lock(&tanque_tank_mutex);

        	    while (1) {
        	        uint8_t x = random_range(0, (MAP_WIDTH / 2) - 1);
        	        uint8_t y = random_range(0, (MAP_HEIGHT / 2) - 1);

        	        mutex_lock(&tanque_map_mutex);
        	        uint8_t is_empty = (mapa[y][x] == TILE_VACIO);
        	        mutex_unlock(&tanque_map_mutex);

        	        if (is_empty) {
        	            tanque_azul.x = x;
        	            tanque_azul.y = y;
        	            tanque_azul.dir = DIR_RIGHT;
        	            tanque_azul.alive = 1;
        	            break;
        	        }
        	    }

        	    while (1) {
        	        uint8_t x = random_range(MAP_WIDTH / 2, MAP_WIDTH - 1);
        	        uint8_t y = random_range(MAP_HEIGHT / 2, MAP_HEIGHT - 1);

        	        int16_t dist_x = (int16_t)x - (int16_t)tanque_azul.x;
        	        int16_t dist_y = (int16_t)y - (int16_t)tanque_azul.y;
        	        uint16_t distancia = (dist_x * dist_x) + (dist_y * dist_y);

        	        mutex_lock(&tanque_map_mutex);
        	        uint8_t is_empty = (mapa[y][x] == TILE_VACIO);
        	        mutex_unlock(&tanque_map_mutex);

        	        if (is_empty && distancia >= 36) {
        	            tanque_rojo.x = x;
        	            tanque_rojo.y = y;
        	            tanque_rojo.dir = DIR_LEFT;
        	            tanque_rojo.alive = 1;
        	            break;
        	        }
        	    }

        	    mutex_unlock(&tanque_tank_mutex);
        	}

        	void tanque_regenerar_mapa(void) {
        	    daos_random_reseed();
        	    generar_config_aleatoria(&config_actual);
        	    generar_mapa_con_config(&config_actual);
        	    first_draw = 1;
        	}

        	// ========================================================================
        	// TAREAS
        	// ========================================================================
        	void tanque_input_task(void) {
        	    static uint8_t initialized = 0;
        	    static uint8_t last_state_p1_left = 0;
        	    static uint8_t last_state_p1_right = 0;
        	    static uint8_t last_state_p1_forward = 0;
        	    static uint32_t last_count_p1_fire = 0;
        	    static uint8_t last_state_p2_left = 0;
        	    static uint8_t last_state_p2_right = 0;
        	    static uint8_t last_state_p2_forward = 0;
        	    static uint32_t last_count_p2_fire = 0;

        	    // PROTECCIÓN: Verificar estado del juego
        	    mutex_lock(&tanque_game_state_mutex);
        	    GameState current_state = game_state;
        	    mutex_unlock(&tanque_game_state_mutex);

        	    if (current_state == GAME_STATE_OVER) {
        	        if (daos_btn_read_by_index(13)) {
        	            tanque_init();
        	            return;
        	        }
        	        daos_sleep_ms(100);
        	        return;
        	    }

        	    if (!game_running || current_state == GAME_STATE_OVER) {
        	        daos_sleep_ms(100);
        	        return;
        	    }

        	    if (!initialized) {
        	        last_state_p1_left = daos_btn_read_by_index(0);
        	        last_state_p1_right = daos_btn_read_by_index(2);
        	        last_count_p1_fire = daos_btn_get_count_by_index(3);
        	        last_state_p2_left = daos_btn_read_by_index(4);
        	        last_state_p2_right = daos_btn_read_by_index(6);
        	        last_count_p2_fire = daos_btn_get_count_by_index(7);
        	        initialized = 1;
        	        daos_sleep_ms(50);
        	        return;
        	    }

        	    // PROTECCIÓN: Leer estado de tanques
        	    mutex_lock(&tanque_tank_mutex);
        	    uint8_t azul_alive = tanque_azul.alive;
        	    uint8_t rojo_alive = tanque_rojo.alive;
        	    Direccion azul_dir = tanque_azul.dir;
        	    Direccion rojo_dir = tanque_rojo.dir;
        	    uint8_t azul_x = tanque_azul.x;
        	    uint8_t azul_y = tanque_azul.y;
        	    uint8_t rojo_x = tanque_rojo.x;
        	    uint8_t rojo_y = tanque_rojo.y;
        	    mutex_unlock(&tanque_tank_mutex);

        	    // JUGADOR 1 (TANQUE AZUL)
        	    if (azul_alive) {
        	        uint8_t current_p1_left = daos_btn_read_by_index(0);
        	        uint8_t current_p1_forward = daos_btn_read_by_index(1);
        	        uint8_t current_p1_right = daos_btn_read_by_index(2);
        	        uint32_t current_p1_fire = daos_btn_get_count_by_index(3);

        	        if (current_p1_left && !last_state_p1_left) {
        	            mutex_lock(&tanque_tank_mutex);
        	            tanque_azul.dir = (tanque_azul.dir + 1) % 4;
        	            uint8_t tx = tanque_azul.x;
        	            uint8_t ty = tanque_azul.y;
        	            mutex_unlock(&tanque_tank_mutex);
        	            clear_tank_position(tx, ty);
        	        }
        	        last_state_p1_left = current_p1_left;

        	        if (current_p1_forward) {
        	            if (!last_state_p1_forward) {
        	                uint8_t old_x = azul_x;
        	                uint8_t old_y = azul_y;
        	                uint8_t new_x = azul_x;
        	                uint8_t new_y = azul_y;

        	                switch(azul_dir) {
        	                    case DIR_UP: if (new_y > 0) new_y--; break;
        	                    case DIR_DOWN: if (new_y < MAP_HEIGHT - 1) new_y++; break;
        	                    case DIR_LEFT: if (new_x > 0) new_x--; break;
        	                    case DIR_RIGHT: if (new_x < MAP_WIDTH - 1) new_x++; break;
        	                }

        	                uint8_t tank_collision = (rojo_alive && new_x == rojo_x && new_y == rojo_y);

        	                if (can_move_to(new_x, new_y) && !tank_collision) {
        	                    clear_tank_position(old_x, old_y);
        	                    mutex_lock(&tanque_tank_mutex);
        	                    tanque_azul.x = new_x;
        	                    tanque_azul.y = new_y;
        	                    mutex_unlock(&tanque_tank_mutex);
        	                }
        	                move_button_hold_p1 = 0;
        	            } else if (move_button_hold_p1 >= BUTTON_REPEAT_DELAY) {
        	                if (try_move_tank(&tanque_azul, &move_counter_azul)) {
        	                    move_button_hold_p1 = BUTTON_REPEAT_DELAY - 2;
        	                }
        	            }
        	            move_button_hold_p1++;
        	        } else {
        	            move_button_hold_p1 = 0;
        	            move_counter_azul = 0;
        	        }
        	        last_state_p1_forward = current_p1_forward;

        	        if (current_p1_right && !last_state_p1_right) {
        	            mutex_lock(&tanque_tank_mutex);
        	            tanque_azul.dir = (tanque_azul.dir + 3) % 4;
        	            uint8_t tx = tanque_azul.x;
        	            uint8_t ty = tanque_azul.y;
        	            mutex_unlock(&tanque_tank_mutex);
        	            clear_tank_position(tx, ty);
        	        }
        	        last_state_p1_right = current_p1_right;

        	        if (current_p1_fire != last_count_p1_fire) {
        	            mutex_lock(&tanque_tank_mutex);
        	            uint8_t tx = tanque_azul.x;
        	            uint8_t ty = tanque_azul.y;
        	            Direccion tdir = tanque_azul.dir;
        	            mutex_unlock(&tanque_tank_mutex);
        	            shoot_projectile(tx, ty, tdir, 1);
        	            last_count_p1_fire = current_p1_fire;
        	        }
        	    }

        	    // JUGADOR 2 (TANQUE ROJO)
        	    if (rojo_alive) {
        	        uint8_t current_p2_left = daos_btn_read_by_index(4);
        	        uint8_t current_p2_forward = daos_btn_read_by_index(5);
        	        uint8_t current_p2_right = daos_btn_read_by_index(6);
        	        uint32_t current_p2_fire = daos_btn_get_count_by_index(7);

        	        if (current_p2_left && !last_state_p2_left) {
        	            mutex_lock(&tanque_tank_mutex);
        	            tanque_rojo.dir = (tanque_rojo.dir + 1) % 4;
        	            uint8_t tx = tanque_rojo.x;
        	            uint8_t ty = tanque_rojo.y;
        	            mutex_unlock(&tanque_tank_mutex);
        	            clear_tank_position(tx, ty);
        	        }
        	        last_state_p2_left = current_p2_left;

        	        if (current_p2_forward) {
        	            if (!last_state_p2_forward) {
        	                uint8_t old_x = rojo_x;
        	                uint8_t old_y = rojo_y;
        	                uint8_t new_x = rojo_x;
        	                uint8_t new_y = rojo_y;

        	                switch(rojo_dir) {
        	                    case DIR_UP: if (new_y > 0) new_y--; break;
        	                    case DIR_DOWN: if (new_y < MAP_HEIGHT - 1) new_y++; break;
        	                    case DIR_LEFT: if (new_x > 0) new_x--; break;
        	                    case DIR_RIGHT: if (new_x < MAP_WIDTH - 1) new_x++; break;
        	                }

        	                uint8_t tank_collision = (azul_alive && new_x == azul_x && new_y == azul_y);

        	                if (can_move_to(new_x, new_y) && !tank_collision) {
        	                    clear_tank_position(old_x, old_y);
        	                    mutex_lock(&tanque_tank_mutex);
        	                    tanque_rojo.x = new_x;
        	                    tanque_rojo.y = new_y;
        	                    mutex_unlock(&tanque_tank_mutex);
        	                }
        	                move_button_hold_p2 = 0;
        	            } else if (move_button_hold_p2 >= BUTTON_REPEAT_DELAY) {
        	                if (try_move_tank(&tanque_rojo, &move_counter_rojo)) {
        	                    move_button_hold_p2 = BUTTON_REPEAT_DELAY - 2;
        	                }
        	            }
        	            move_button_hold_p2++;
        	        } else {
        	            move_button_hold_p2 = 0;
        	            move_counter_rojo = 0;
        	        }
        	        last_state_p2_forward = current_p2_forward;

        	        if (current_p2_right && !last_state_p2_right) {
        	            mutex_lock(&tanque_tank_mutex);
        	            tanque_rojo.dir = (tanque_rojo.dir + 3) % 4;
        	            uint8_t tx = tanque_rojo.x;
        	            uint8_t ty = tanque_rojo.y;
        	            mutex_unlock(&tanque_tank_mutex);
        	            clear_tank_position(tx, ty);
        	        }
        	        last_state_p2_right = current_p2_right;

        	        if (current_p2_fire != last_count_p2_fire) {
        	            mutex_lock(&tanque_tank_mutex);
        	            uint8_t tx = tanque_rojo.x;
        	            uint8_t ty = tanque_rojo.y;
        	            Direccion tdir = tanque_rojo.dir;
        	            mutex_unlock(&tanque_tank_mutex);
        	            shoot_projectile(tx, ty, tdir, 0);
        	            last_count_p2_fire = current_p2_fire;
        	        }
        	    }

        	    daos_sleep_ms(50);
        	}

        	void tanque_logic_task(void) {
        	    // PROTECCIÓN: Verificar estado del juego
        	    mutex_lock(&tanque_game_state_mutex);
        	    GameState current_state = game_state;
        	    mutex_unlock(&tanque_game_state_mutex);

        	    if (!game_running || current_state == GAME_STATE_OVER) {
        	        daos_sleep_ms(100);
        	        return;
        	    }

        	    // PROTECCIÓN: Actualizar proyectiles
        	    mutex_lock(&tanque_projectile_mutex);
        	    for (uint8_t i = 0; i < MAX_PROJECTILES; i++) {
        	        if (proyectiles[i].active) {
        	            uint8_t deactivate = update_projectile_movement(&proyectiles[i]);
        	            if (deactivate) {
        	                int8_t px = proyectiles[i].x;
        	                int8_t py = proyectiles[i].y;
        	                proyectiles[i].active = 0;
        	                mutex_unlock(&tanque_projectile_mutex);
        	                redraw_tile_content(px, py);
        	                mutex_lock(&tanque_projectile_mutex);
        	            }
        	        }
        	    }
        	    mutex_unlock(&tanque_projectile_mutex);

        	    check_game_over();

        	    daos_sleep_ms(50);
        	}

        	void tanque_render_task(void) {
        	    if (!game_running) {
        	        daos_sleep_ms(50);
        	        return;
        	    }

        	    // PROTECCIÓN: Verificar estado del juego
        	    mutex_lock(&tanque_game_state_mutex);
        	    GameState current_state = game_state;
        	    GameWinner current_winner = winner;
        	    mutex_unlock(&tanque_game_state_mutex);

        	    if (current_state == GAME_STATE_OVER) {
        	        daos_gfx_fill_rect(60, 90, 200, 80, COLOR_BLACK);
        	        daos_gfx_fill_rect(58, 88, 204, 84, COLOR_WHITE);
        	        daos_gfx_fill_rect(60, 90, 200, 80, COLOR_BLACK);

        	        daos_gfx_draw_text_large(75, 100, "GAME OVER", COLOR_WHITE, COLOR_BLACK, 2);

        	        uint16_t result_color = COLOR_WHITE;
        	        const char *result_text = "";

        	        switch(current_winner) {
        	            case WINNER_P1_AZUL:
        	                result_text = "P1 AZUL WINS!";
        	                result_color = COLOR_AZUL;
        	                break;
        	            case WINNER_P2_ROJO:
        	                result_text = "P2 ROJO WINS!";
        	                result_color = COLOR_ROJO;
        	                break;
        	            case WINNER_DRAW:
        	                result_text = "EMPATE";
        	                result_color = COLOR_WHITE;
        	                break;
        	            default:
        	                result_text = "ERROR";
        	                break;
        	        }

        	        daos_gfx_draw_text_large(75, 125, result_text, result_color, COLOR_BLACK, 2);
        	        daos_gfx_draw_text(65, 155, "PRESIONA D15 PARA REINICIAR", COLOR_WHITE, COLOR_BLACK);

        	        daos_sleep_ms(100);
        	        return;
        	    }

        	    if (first_draw) {
        	        daos_gfx_clear(COLOR_BLACK);

        	        mutex_lock(&tanque_map_mutex);
        	        for (uint8_t y = 0; y < MAP_HEIGHT; y++) {
        	            for (uint8_t x = 0; x < MAP_WIDTH; x++) {
        	            	switch (mapa[y][x]) {
        	            	                    case TILE_DESTRUCTIBLE:
        	            	                        draw_tile(x, y, sprite_bloque_destructible);
        	            	                        break;
        	            	                    case TILE_ACERO:
        	            	                        draw_tile(x, y, sprite_bloque_acero);
        	            	                        break;
        	            	                    case TILE_PLANTA:
        	            	                        draw_tile(x, y, sprite_planta);
        	            	                        break;
        	            	                    default:
        	            	                        break;
        	            	                }
        	            	            }
        	            	        }
        	            	        mutex_unlock(&tanque_map_mutex);

        	            	        int16_t map_end_x = MAP_OFFSET_X + (MAP_WIDTH * TILE_SIZE) - 1;
        	            	        int16_t map_end_y = MAP_OFFSET_Y + (MAP_HEIGHT * TILE_SIZE) - 1;

        	            	        draw_horizontal_line(MAP_OFFSET_X, MAP_OFFSET_Y, map_end_x, COLOR_WHITE);
        	            	        draw_horizontal_line(MAP_OFFSET_X, map_end_y, map_end_x, COLOR_WHITE);
        	            	        draw_vertical_line(MAP_OFFSET_X, MAP_OFFSET_Y, map_end_y, COLOR_WHITE);
        	            	        draw_vertical_line(map_end_x, MAP_OFFSET_Y, map_end_y, COLOR_WHITE);

        	            	        first_draw = 0;
        	            	    }

        	            	    // PROTECCIÓN: Dibujar proyectiles
        	            	    mutex_lock(&tanque_projectile_mutex);
        	            	    for (uint8_t i = 0; i < MAX_PROJECTILES; i++) {
        	            	        if (proyectiles[i].active) {
        	            	            draw_projectile(proyectiles[i].x, proyectiles[i].y);
        	            	        }
        	            	    }
        	            	    mutex_unlock(&tanque_projectile_mutex);

        	            	    // PROTECCIÓN: Dibujar tanques
        	            	    mutex_lock(&tanque_tank_mutex);
        	            	    uint8_t azul_alive = tanque_azul.alive;
        	            	    uint8_t azul_x = tanque_azul.x;
        	            	    uint8_t azul_y = tanque_azul.y;
        	            	    Direccion azul_dir = tanque_azul.dir;
        	            	    uint8_t rojo_alive = tanque_rojo.alive;
        	            	    uint8_t rojo_x = tanque_rojo.x;
        	            	    uint8_t rojo_y = tanque_rojo.y;
        	            	    Direccion rojo_dir = tanque_rojo.dir;
        	            	    mutex_unlock(&tanque_tank_mutex);

        	            	    if (azul_alive) {
        	            	        mutex_lock(&tanque_map_mutex);
        	            	        uint8_t is_planta = (mapa[azul_y][azul_x] != TILE_PLANTA);
        	            	        mutex_unlock(&tanque_map_mutex);

        	            	        if (is_planta) {
        	            	            draw_tile_rotated(azul_x, azul_y, sprite_tanque_azul, azul_dir);
        	            	        }
        	            	    }

        	            	    if (rojo_alive) {
        	            	        mutex_lock(&tanque_map_mutex);
        	            	        uint8_t is_planta = (mapa[rojo_y][rojo_x] != TILE_PLANTA);
        	            	        mutex_unlock(&tanque_map_mutex);

        	            	        if (is_planta) {
        	            	            draw_tile_rotated(rojo_x, rojo_y, sprite_tanque_rojo, rojo_dir);
        	            	        }
        	            	    }

        	            	    // Dibujar plantas sobre tanques si es necesario
        	            	    if (azul_alive) {
        	            	        mutex_lock(&tanque_map_mutex);
        	            	        if (mapa[azul_y][azul_x] == TILE_PLANTA) {
        	            	            mutex_unlock(&tanque_map_mutex);
        	            	            draw_tile(azul_x, azul_y, sprite_planta);
        	            	        } else {
        	            	            mutex_unlock(&tanque_map_mutex);
        	            	        }
        	            	    }

        	            	    if (rojo_alive) {
        	            	        mutex_lock(&tanque_map_mutex);
        	            	        if (mapa[rojo_y][rojo_x] == TILE_PLANTA) {
        	            	            mutex_unlock(&tanque_map_mutex);
        	            	            draw_tile(rojo_x, rojo_y, sprite_planta);
        	            	        } else {
        	            	            mutex_unlock(&tanque_map_mutex);
        	            	        }
        	            	    }

        	            	    // Redibujar bordes
        	            	    int16_t map_end_x = MAP_OFFSET_X + (MAP_WIDTH * TILE_SIZE) - 1;
        	            	    int16_t map_end_y = MAP_OFFSET_Y + (MAP_HEIGHT * TILE_SIZE) - 1;

        	            	    draw_horizontal_line(MAP_OFFSET_X, MAP_OFFSET_Y, map_end_x, COLOR_WHITE);
        	            	    draw_horizontal_line(MAP_OFFSET_X, map_end_y, map_end_x, COLOR_WHITE);
        	            	    draw_vertical_line(MAP_OFFSET_X, MAP_OFFSET_Y, map_end_y, COLOR_WHITE);
        	            	    draw_vertical_line(map_end_x, MAP_OFFSET_Y, map_end_y, COLOR_WHITE);

        	            	    daos_sleep_ms(30);
        	            	}

        	            	// ========================================================================
        	            	// INICIALIZACIÓN
        	            	// ========================================================================
        	            	void tanque_init(void) {
        	            	    game_running = 0;
        	            	    daos_sleep_ms(100);

        	            	    // INICIALIZACIÓN: Configurar mutexes y semáforos
        	            	    mutex_init(&tanque_game_state_mutex);
        	            	    mutex_init(&tanque_map_mutex);
        	            	    mutex_init(&tanque_tank_mutex);
        	            	    mutex_init(&tanque_projectile_mutex);

        	            	    // Inicializar semáforos (NO SE USAN - se eliminaron las sincronizaciones bloqueantes)
        	            	    // Los dejamos inicializados por si se necesitan en el futuro
        	            	    sem_init(&tanque_input_ready_sem, 0, 1);
        	            	    sem_init(&tanque_logic_ready_sem, 0, 1);
        	            	    sem_init(&tanque_render_ready_sem, 0, 1);

        	            	    daos_gfx_clear(COLOR_BLACK);
        	            	    daos_random_init();

        	            	    config_actual.num_destructibles = 25;
        	            	    config_actual.num_acero = 12;
        	            	    config_actual.num_plantas = 6;
        	            	    config_actual.densidad_pasillos = 70;
        	            	    config_actual.num_conectores = 5;
        	            	    config_actual.prob_agrupacion = 50;

        	            	    generar_mapa_con_config(&config_actual);

        	            	    // PROTECCIÓN: Inicializar proyectiles
        	            	    mutex_lock(&tanque_projectile_mutex);
        	            	    for (uint8_t i = 0; i < MAX_PROJECTILES; i++) {
        	            	        proyectiles[i].active = 0;
        	            	        proyectiles[i].move_counter = 0;
        	            	    }
        	            	    mutex_unlock(&tanque_projectile_mutex);

        	            	    // PROTECCIÓN: Inicializar estado del juego
        	            	    mutex_lock(&tanque_game_state_mutex);
        	            	    game_state = GAME_STATE_RUNNING;
        	            	    winner = WINNER_NONE;
        	            	    mutex_unlock(&tanque_game_state_mutex);

        	            	    move_counter_azul = 0;
        	            	    move_counter_rojo = 0;
        	            	    move_button_hold_p1 = 0;
        	            	    move_button_hold_p2 = 0;

        	            	    first_draw = 1;
        	            	    game_running = 1;
        	            	}

        	            	void tanque_cleanup(void) {
        	            	    // PROTECCIÓN: Limpiar de forma segura
        	            	    game_running = 0;

        	            	    mutex_lock(&tanque_game_state_mutex);
        	            	    daos_gfx_clear(COLOR_BLACK);
        	            	    mutex_unlock(&tanque_game_state_mutex);

        	            	    // Nota: Los mutexes y semáforos no necesitan destrucción explícita
        	            	    // en este sistema embebido, pero podrían agregarse funciones
        	            	    // mutex_destroy() y sem_destroy() si fuera necesario
        	            	}

        	            	// ========================================================================
        	            	// API PÚBLICA
        	            	// ========================================================================
        	            	tanque_state_t tanque_get_state(void) {
        	            	    // PROTECCIÓN: Lectura segura del estado
        	            	    mutex_lock(&tanque_game_state_mutex);
        	            	    GameState state = game_state;
        	            	    mutex_unlock(&tanque_game_state_mutex);

        	            	    return (state == GAME_STATE_RUNNING) ? TANQUE_RUNNING : TANQUE_GAME_OVER;
        	            	}

        	            	tanque_winner_t tanque_get_winner(void) {
        	            	    // PROTECCIÓN: Lectura segura del ganador
        	            	    mutex_lock(&tanque_game_state_mutex);
        	            	    GameWinner w = winner;
        	            	    mutex_unlock(&tanque_game_state_mutex);

        	            	    switch(w) {
        	            	        case WINNER_P1_AZUL: return TANQUE_WINNER_P1_AZUL;
        	            	        case WINNER_P2_ROJO: return TANQUE_WINNER_P2_ROJO;
        	            	        case WINNER_DRAW: return TANQUE_WINNER_DRAW;
        	            	        default: return TANQUE_WINNER_NONE;
        	            	    }
        	            	}
