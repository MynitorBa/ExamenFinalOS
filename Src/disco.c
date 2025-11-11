#include "disco.h"
#include "api.h"
#include "sync.h"

// ========================================================================
// CONSTANTES Y ARRAYS DE SPRITES - 25x25
// ========================================================================
#define SPRITE_WIDTH 25
#define SPRITE_HEIGHT 25
#define SPRITE_SIZE (10 * 10)

// Frame: Player 1 Idle 2 - AZUL
static const uint32_t sprite_p1_idle2[100] = {
    0x00000000, 0x00000000, 0xff74e3e4, 0xff74e3e4, 0xffc8e4e4, 0xff74e3e4, 0xff74e3e4, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0xff74e3e4, 0xffc8e4e4, 0xff74e3e4, 0xff74e3e4, 0xff74e3e4, 0xff74e3e4, 0xff74e3e4, 0x00000000, 0x00000000,
    0x00000000, 0xff74e3e4, 0xff74e3e4, 0xff74e3e4, 0xffc8e4e4, 0xffc8e4e4, 0xffc8e4e4, 0xff74e3e4, 0x00000000, 0x00000000,
    0x00000000, 0xffc8e4e4, 0xff74e3e4, 0xffc8e4e4, 0xff1c1c1c, 0xffc8e4e4, 0xff1c1c1c, 0xffc8e4e4, 0x00000000, 0x00000000,
    0x00000000, 0xff74e3e4, 0xffc8e4e4, 0xffc8e4e4, 0xffc8e4e4, 0xffc8e4e4, 0xffc8e4e4, 0xffc8e4e4, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xffc8e4e4, 0xffc8e4e4, 0xffc8e4e4, 0xffc8e4e4, 0xffc8e4e4, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0xff74e3e4, 0xff74e3e4, 0xff74e3e4, 0xffc8e4e4, 0xff74e3e4, 0xffc8e4e4, 0xff74e3e4, 0x00000000, 0x00000000,
    0x00000000, 0xff42bebf, 0xffc8e4e4, 0xff74e3e4, 0xff74e3e4, 0xffc8e4e4, 0xff74e3e4, 0xff42bebf, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xffc8e4e4, 0xff74e3e4, 0xff74e3e4, 0xff74e3e4, 0xff74e3e4, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xff42bebf, 0xff42bebf, 0x00000000, 0xff42bebf, 0xff42bebf, 0x00000000, 0x00000000, 0x00000000
};

static const uint32_t sprite_p1_idle1[100] = {
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xff74e3e4, 0xff74e3e4, 0xffc8e4e4, 0xff74e3e4, 0xff74e3e4, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0xff74e3e4, 0xffc8e4e4, 0xff74e3e4, 0xff74e3e4, 0xff74e3e4, 0xff74e3e4, 0xff74e3e4, 0x00000000, 0x00000000,
    0x00000000, 0xff74e3e4, 0xff74e3e4, 0xff74e3e4, 0xffc8e4e4, 0xffc8e4e4, 0xffc8e4e4, 0xff74e3e4, 0x00000000, 0x00000000,
    0x00000000, 0xffc8e4e4, 0xff74e3e4, 0xffc8e4e4, 0xff1c1c1c, 0xffc8e4e4, 0xff1c1c1c, 0xffc8e4e4, 0x00000000, 0x00000000,
    0x00000000, 0xff74e3e4, 0xffc8e4e4, 0xffc8e4e4, 0xffc8e4e4, 0xffc8e4e4, 0xffc8e4e4, 0xffc8e4e4, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xffc8e4e4, 0xffc8e4e4, 0xffc8e4e4, 0xffc8e4e4, 0xffc8e4e4, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0xff74e3e4, 0xff74e3e4, 0xff74e3e4, 0xffc8e4e4, 0xff74e3e4, 0xffc8e4e4, 0xff74e3e4, 0x00000000, 0x00000000,
    0x00000000, 0xff42bebf, 0xffc8e4e4, 0xff74e3e4, 0xff74e3e4, 0xffc8e4e4, 0xff74e3e4, 0xff42bebf, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xffc8e4e4, 0xff74e3e4, 0xff74e3e4, 0xff74e3e4, 0xff74e3e4, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xff42bebf, 0xff42bebf, 0x00000000, 0xff42bebf, 0xff42bebf, 0x00000000, 0x00000000, 0x00000000
};

// Frame: Player 2 Idle 2 - NEGRO con ROJO
static const uint32_t sprite_p2_idle2[100] = {
    0x00000000, 0x00000000, 0x00000000, 0xff110000, 0xffff0000, 0xffff0000, 0xff110000, 0xff110000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xff110000, 0xff222222, 0xff110000, 0xff110000, 0xff222222, 0xff110000, 0xff110000, 0x00000000,
    0x00000000, 0x00000000, 0xff110000, 0xff222222, 0xff110000, 0xff110000, 0xff222222, 0xff110000, 0xffff0000, 0x00000000,
    0x00000000, 0x00000000, 0xff110000, 0xff110000, 0xff110000, 0xff222222, 0xff110000, 0xff110000, 0xffff0000, 0x00000000,
    0x00000000, 0x00000000, 0xff110000, 0xff110000, 0xff110000, 0xff222222, 0xff110000, 0xff110000, 0xff110000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xff110000, 0xff110000, 0xff110000, 0xff110000, 0xff110000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xff110000, 0xff222222, 0xff110000, 0xff222222, 0xff222222, 0xff222222, 0xff110000, 0x00000000,
    0x00000000, 0x00000000, 0xff222222, 0xffff0000, 0xff222222, 0xffff0000, 0xff110000, 0xff110000, 0xff222222, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xff110000, 0xffff0000, 0xff110000, 0xff110000, 0xff110000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xff222222, 0xff222222, 0x00000000, 0xff222222, 0xff222222, 0x00000000, 0x00000000
};

static const uint32_t sprite_p2_idle1[100] = {
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xff110000, 0xffff0000, 0xffff0000, 0xff110000, 0xff110000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xff110000, 0xff222222, 0xff110000, 0xff110000, 0xff222222, 0xff110000, 0xff110000, 0x00000000,
    0x00000000, 0x00000000, 0xff110000, 0xff222222, 0xff110000, 0xff110000, 0xff222222, 0xff110000, 0xffff0000, 0x00000000,
    0x00000000, 0x00000000, 0xff110000, 0xff110000, 0xff110000, 0xff222222, 0xff110000, 0xff110000, 0xffff0000, 0x00000000,
    0x00000000, 0x00000000, 0xff110000, 0xff110000, 0xff110000, 0xff222222, 0xff110000, 0xff110000, 0xff110000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xff110000, 0xff110000, 0xff110000, 0xff110000, 0xff110000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xff110000, 0xff222222, 0xff110000, 0xff222222, 0xff222222, 0xff222222, 0xff110000, 0x00000000,
    0x00000000, 0x00000000, 0xff222222, 0xffff0000, 0xff222222, 0xffff0000, 0xff110000, 0xff110000, 0xff222222, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xff110000, 0xffff0000, 0xff110000, 0xff110000, 0xff110000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xff222222, 0xff222222, 0x00000000, 0xff222222, 0xff222222, 0x00000000, 0x00000000
};

// Frame: Player 1 Disparando
static const uint32_t sprite_p1_shoot[100] = {
    0x00000000, 0x00000000, 0xff74e3e4, 0xff74e3e4, 0xffc8e4e4, 0xff74e3e4, 0xff74e3e4, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0xff74e3e4, 0xffc8e4e4, 0xff74e3e4, 0xff74e3e4, 0xff74e3e4, 0xff74e3e4, 0xff74e3e4, 0x00000000, 0x00000000,
    0x00000000, 0xff74e3e4, 0xff74e3e4, 0xff74e3e4, 0xffc8e4e4, 0xffc8e4e4, 0xffc8e4e4, 0xff74e3e4, 0x00000000, 0x00000000,
    0x00000000, 0xffc8e4e4, 0xff74e3e4, 0xffc8e4e4, 0xff1c1c1c, 0xffc8e4e4, 0xff1c1c1c, 0xffc8e4e4, 0x00000000, 0x00000000,
    0x00000000, 0xff74e3e4, 0xffc8e4e4, 0xffc8e4e4, 0xffc8e4e4, 0xffc8e4e4, 0xffc8e4e4, 0xffc8e4e4, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xffc8e4e4, 0xffc8e4e4, 0xffc8e4e4, 0xffc8e4e4, 0xffc8e4e4, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xffc8e4e4, 0xffc8e4e4, 0xffc8e4e4, 0xffc8e4e4, 0xffc8e4e4, 0xffc8e4e4, 0xffc8e4e4, 0x00000000,
    0x00000000, 0xffc8e4e4, 0xffc8e4e4, 0xff74e3e4, 0xff74e3e4, 0xffc8e4e4, 0xffc8e4e4, 0xffc8e4e4, 0xffc8e4e4, 0x00000000,
    0x00000000, 0xffc8e4e4, 0xff74e3e4, 0xffc8e4e4, 0xffc8e4e4, 0xff74e3e4, 0xffc8e4e4, 0xff74e3e4, 0xffc8e4e4, 0x00000000,
    0x00000000, 0xffc8e4e4, 0xff74e3e4, 0xffc8e4e4, 0xffc8e4e4, 0xff74e3e4, 0xffc8e4e4, 0xff74e3e4, 0xffc8e4e4, 0x00000000
};

// Frame: Player 2 Disparando
static const uint32_t sprite_p2_shoot[100] = {
    0x00000000, 0x00000000, 0x00000000, 0xff110000, 0xffff0000, 0xffff0000, 0xff110000, 0xff110000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xff110000, 0xff222222, 0xff110000, 0xff110000, 0xff222222, 0xff110000, 0xff110000, 0x00000000,
    0x00000000, 0x00000000, 0xff110000, 0xff222222, 0xff110000, 0xff110000, 0xff222222, 0xff110000, 0xffff0000, 0x00000000,
    0x00000000, 0x00000000, 0xff110000, 0xff110000, 0xff110000, 0xff222222, 0xff110000, 0xff110000, 0xffff0000, 0x00000000,
    0x00000000, 0x00000000, 0xff110000, 0xff110000, 0xff110000, 0xff222222, 0xff110000, 0xff110000, 0xff110000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xff110000, 0xff110000, 0xff110000, 0xff110000, 0xff110000, 0x00000000, 0x00000000,
    0x00000000, 0xff110000, 0xff110000, 0xff110000, 0xff110000, 0xff222222, 0xff110000, 0xff222222, 0x00000000, 0x00000000,
    0x00000000, 0xff110000, 0xff110000, 0xffff0000, 0xffff0000, 0xff110000, 0xff110000, 0xff222222, 0x00000000, 0x00000000,
    0x00000000, 0xff110000, 0xffff0000, 0xff110000, 0xff110000, 0xffff0000, 0xff222222, 0xff110000, 0x00000000, 0x00000000,
    0x00000000, 0xff110000, 0xffff0000, 0xff110000, 0xff110000, 0xffff0000, 0xff222222, 0xff110000, 0x00000000, 0x00000000
};

// ========================================================================
// CONSTANTES DE JUEGO
// ========================================================================
#define MAX_GAME_TIME_S 15 // MODIFICADO: 30 -> 15 segundos
#define MAX_GAME_TIME_MS (MAX_GAME_TIME_S * 1000)
#define LOGIC_CYCLE_MS 50

// Colores
#define COLOR_YELLOW   0xFFE0
#define COLOR_ORANGE   0xFC00
#define COLOR_RED      0xF800
#define COLOR_P1       0x001F  // Azul
#define COLOR_P2       0x07E0  // Verde
#define COLOR_WHITE    0xFFFF
#define COLOR_BLACK    0x0000
#define COLOR_TIME     0xFF00  // Rojo para tiempo
#define COLOR_GRAY     0xAD55

// NUEVOS COLORES PARA LAS BOLAS DE TIEMPO
#define COLOR_TIME_BALL_A 0x07E0 // Bola 15s-10s (Verde/P2 Color)
#define COLOR_TIME_BALL_B 0xFFE0 // Bola 10s-5s (Amarillo)
#define COLOR_TIME_BALL_C 0xF800 // Bola 5s-0s (Rojo)

// Estado del juego
static uint8_t game_running = 0;

// Posiciones de los jugadores
static int8_t player1_offset_x = 0;
static int8_t player1_offset_y = 0;
static uint8_t player1_lane = 1;

static int8_t player2_offset_x = 0;
static int8_t player2_offset_y = 0;
static uint8_t player2_lane = 1;

// Estados de escudo
static uint8_t shield_p1 = 0;
static uint8_t shield_p2 = 0;

// Variables de tiempo y puntuación
static uint32_t start_time_ms = 0;
static uint32_t simulated_time_ms = 0;
static uint16_t destruction_score_p1 = 0;
static uint16_t destruction_score_p2 = 0;

// Plataformas
typedef struct {
    int16_t center_x;
    int16_t center_y;
    uint8_t radius_inner;
    uint8_t radius_middle;
    uint8_t radius_outer;
    uint8_t layer;
    uint8_t exists;
} Platform;

static Platform platforms[6];

// Sistema de disparos
#define MAX_SHOTS 3
typedef struct {
    int16_t x;
    int16_t y;
    int8_t vx;
    uint8_t active;
    uint8_t color_is_p1;
    uint8_t target_lane;
} Shot;

static Shot shots_p1[MAX_SHOTS];
static Shot shots_p2[MAX_SHOTS];
static uint8_t animation_frame_counter = 0;
static uint8_t p1_shooting_frame_timer = 0;
static uint8_t p2_shooting_frame_timer = 0;

// Variables estáticas de tarea
#define BTN_D15_INDEX 8

// Tarea de Input
static uint8_t initialized_input = 0;
static uint32_t last_p1_shield_count = 0;
static uint32_t last_p1_up_count = 0;
static uint32_t last_p1_down_count = 0;
static uint32_t last_p1_shoot_count = 0;
static uint32_t last_p2_shoot_count = 0;
static uint32_t last_p2_up_count = 0;
static uint32_t last_p2_down_count = 0;
static uint32_t last_p2_shield_count = 0;
static uint32_t last_d15_count = 0;

// Tarea de Renderizado
static uint8_t first_draw = 1;
static int16_t last_p1_x = 0, last_p1_y = 0;
static int16_t last_p2_x = 0, last_p2_y = 0;
static uint8_t last_p1_lane = 1;
static uint8_t last_p2_lane = 1;
static uint8_t last_shield_p1 = 0;
static uint8_t last_shield_p2 = 0;
static uint8_t last_layers[6] = {3, 3, 3, 3, 3, 3};
static int16_t last_shot_x_p1[MAX_SHOTS] = {-1, -1, -1};
static int16_t last_shot_y_p1[MAX_SHOTS] = {-1, -1, -1};
static int16_t last_shot_x_p2[MAX_SHOTS] = {-1, -1, -1};
static int16_t last_shot_y_p2[MAX_SHOTS] = {-1, -1, -1};
// VARIABLE PARA EL ESTADO DE LAS BOLAS DE TIEMPO
static uint8_t last_time_ball_state = 3;


/* ============================================================ */
/* MUTEXES Y SEMÁFOROS PARA SINCRONIZACIÓN            */
/* ============================================================ */

// Mutexes para protección de recursos compartidos
mutex_t disco_game_state_mutex;
mutex_t disco_platform_mutex;
mutex_t disco_player_mutex;
mutex_t disco_shot_mutex;

// Semáforos para sincronización de tareas
sem_t disco_input_ready_sem;
sem_t disco_logic_ready_sem;
sem_t disco_render_ready_sem;

/* ============================================================ */
/* FUNCIÓN AUXILIAR PARA CONVERSIÓN DE ENTERO A CADENA        */
/* ============================================================ */
static char* integer_to_string(int32_t value, char *buffer) {
    if (buffer == NULL) return NULL;
    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return buffer;
    }

    char *ptr = buffer;
    int32_t tmp_value = value;

    while (tmp_value > 0) {
        *ptr++ = (char)(tmp_value % 10) + '0';
        tmp_value /= 10;
    }
    *ptr = '\0';

    char *start = buffer;
    char *end = ptr - 1;
    char temp;

    while (start < end) {
        temp = *start;
        *start++ = *end;
        *end-- = temp;
    }

    return buffer;
}

/* ============================================================ */
/* FUNCIÓN DE TIEMPO SIMULADO                                  */
/* ============================================================ */
static uint32_t get_simulated_time(void) {
    return simulated_time_ms;
}

/* ============================================================ */
/* FUNCIONES AUXILIARES DE DIBUJO                              */
/* ============================================================ */

static void draw_sprite_at(int16_t x, int16_t y, const uint32_t* sprite_data) {
    for (uint8_t row = 0; row < SPRITE_HEIGHT; row++) {
        uint8_t data_row = (row * 10) / SPRITE_HEIGHT;
        if (data_row >= 10) data_row = 9;

        for (uint8_t col = 0; col < SPRITE_WIDTH; col++) {
            uint8_t data_col = (col * 10) / SPRITE_WIDTH;
            if (data_col >= 10) data_col = 9;

            uint32_t color_32bit = sprite_data[data_row * 10 + data_col];

            if ((color_32bit >> 24) != 0x00) {
                uint8_t r = (color_32bit >> 16) & 0xFF;
                uint8_t g = (color_32bit >> 8) & 0xFF;
                uint8_t b = color_32bit & 0xFF;
                uint16_t color_16bit = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
                daos_gfx_draw_pixel(x - 12 + col, y - 12 + row, color_16bit);
            }
        }
    }
}

static void draw_platform(Platform* p) {
    if (!p->exists) {
        daos_gfx_draw_circle_filled(p->center_x, p->center_y, p->radius_outer + 2, COLOR_BLACK);
        return;
    }

    if (p->layer >= 3) {
        daos_gfx_draw_circle_filled(p->center_x, p->center_y, p->radius_outer, COLOR_YELLOW);
    } else {
        daos_gfx_draw_circle_filled(p->center_x, p->center_y, p->radius_outer, COLOR_BLACK);
    }

    if (p->layer >= 2) {
        daos_gfx_draw_circle_filled(p->center_x, p->center_y, p->radius_middle, COLOR_ORANGE);
    } else if (p->layer < 3) {
        daos_gfx_draw_circle_filled(p->center_x, p->center_y, p->radius_middle, COLOR_BLACK);
    }

    if (p->layer >= 1) {
        daos_gfx_draw_circle_filled(p->center_x, p->center_y, p->radius_inner, COLOR_RED);
    } else {
        daos_gfx_draw_circle_filled(p->center_x, p->center_y, p->radius_inner, COLOR_BLACK);
    }
}

static void erase_player_at(int16_t x, int16_t y, uint8_t lane_idx) {
    int16_t erase_x_start = x - 15;
    int16_t erase_y_start = y - 15;
    int16_t erase_width = 31;
    int16_t erase_height = 31;

    if (lane_idx >= 6) return;

    // PROTECCIÓN: Leer estado de escudos
    mutex_lock(&disco_player_mutex);
    uint8_t local_shield_p1 = shield_p1;
    uint8_t local_shield_p2 = shield_p2;
    mutex_unlock(&disco_player_mutex);

    // Borrar escudo si estaba activo
    if ((lane_idx < 3 && last_shield_p1) || (lane_idx >= 3 && last_shield_p2)) {
        // PROTECCIÓN: Leer plataforma
        mutex_lock(&disco_platform_mutex);
        Platform p = platforms[lane_idx];
        mutex_unlock(&disco_platform_mutex);

        // Lógica de escudo: P1 defiende derecha, P2 defiende izquierda
        int16_t line_x_center;
        if (lane_idx < 3) { // P1
             // P1 defiende derecha (lado del oponente)
             line_x_center = p.center_x + p.radius_outer + 1;
        } else { // P2
             // P2 defiende izquierda (lado del oponente)
             line_x_center = p.center_x - p.radius_outer - 1;
        }

        int16_t line_y_start = p.center_y - p.radius_outer - 1;
        int16_t line_y_end = p.center_y + p.radius_outer + 1;
        daos_gfx_fill_rect(line_x_center - 1, line_y_start, 3, line_y_end - line_y_start + 1, COLOR_BLACK);
    }

    // PROTECCIÓN: Leer estado de plataforma
    mutex_lock(&disco_platform_mutex);
    Platform p = platforms[lane_idx];
    mutex_unlock(&disco_platform_mutex);

    if (!p.exists) {
        daos_gfx_fill_rect(erase_x_start, erase_y_start, erase_width, erase_height, COLOR_BLACK);
        return;
    }

    // Redibujar área con color de plataforma
    for (int dy = -15; dy <= 15; dy++) {
        for (int dx = -15; dx <= 15; dx++) {
            int16_t px = x + dx;
            int16_t py = y + dy;

            int16_t dist_x = px - p.center_x;
            int16_t dist_y = py - p.center_y;
            uint32_t dist_sq = (uint32_t)dist_x * dist_x + (uint32_t)dist_y * dist_y;

            uint32_t r_outer_sq = (uint32_t)p.radius_outer * p.radius_outer;
            uint32_t r_middle_sq = (uint32_t)p.radius_middle * p.radius_middle;
            uint32_t r_inner_sq = (uint32_t)p.radius_inner * p.radius_inner;

            if (p.layer >= 3 && dist_sq <= r_outer_sq) {
                if (p.layer >= 1 && dist_sq <= r_inner_sq) {
                    daos_gfx_draw_pixel(px, py, COLOR_RED);
                } else if (p.layer >= 2 && dist_sq <= r_middle_sq) {
                    daos_gfx_draw_pixel(px, py, COLOR_ORANGE);
                } else {
                    daos_gfx_draw_pixel(px, py, COLOR_YELLOW);
                }
            } else if (p.layer == 2 && dist_sq <= r_middle_sq) {
                if (p.layer >= 1 && dist_sq <= r_inner_sq) {
                    daos_gfx_draw_pixel(px, py, COLOR_RED);
                } else {
                    daos_gfx_draw_pixel(px, py, COLOR_ORANGE);
                }
            } else if (p.layer == 1 && dist_sq <= r_inner_sq) {
                daos_gfx_draw_pixel(px, py, COLOR_RED);
            } else {
                daos_gfx_draw_pixel(px, py, COLOR_BLACK);
            }
        }
    }
}

// FUNCIÓN AUXILIAR PARA DIBUJAR BOLAS DE TIEMPO
static void draw_time_ball(int16_t x, int16_t y, uint16_t color) {
    daos_gfx_draw_circle_filled(x, y, 5, color);
}

static void check_and_show_victory(void) {
    uint8_t p1_platforms = 0;
    uint8_t p2_platforms = 0;
    uint8_t game_over_by_destruction = 0;

    // PROTECCIÓN: Leer estado de plataformas
    mutex_lock(&disco_platform_mutex);
    for (uint8_t i = 0; i < 3; i++) {
        if (platforms[i].exists) p1_platforms++;
        if (platforms[i + 3].exists) p2_platforms++;
    }

    // PROTECCIÓN: Leer lanes de jugadores
    mutex_lock(&disco_player_mutex);
    uint8_t local_p1_lane = player1_lane;
    uint8_t local_p2_lane = player2_lane;
    mutex_unlock(&disco_player_mutex);

    // Comprobar derrota por caída
    if (platforms[local_p1_lane].exists == 0) {
        p1_platforms = 0;
    }
    if (platforms[local_p2_lane + 3].exists == 0) {
        p2_platforms = 0;
    }
    mutex_unlock(&disco_platform_mutex);

    // Comprobar derrota por tiempo
    uint32_t elapsed_ms = get_simulated_time() - start_time_ms;
    if (elapsed_ms >= MAX_GAME_TIME_MS) {
        game_over_by_destruction = 1;
    }

    // Si terminó el juego
    if ((p1_platforms == 0 || p2_platforms == 0 || game_over_by_destruction)) {
        // PROTECCIÓN: Verificar si ya terminó
        mutex_lock(&disco_game_state_mutex);
        if (!game_running) {
            mutex_unlock(&disco_game_state_mutex);
            return;
        }
        game_running = 0;
        mutex_unlock(&disco_game_state_mutex);

        daos_gfx_clear(COLOR_BLACK);

        // Determinar ganador
        uint8_t winner_id = 0; // 1=P1, 2=P2, 0=Empate

        if (game_over_by_destruction) {
            if (destruction_score_p1 > destruction_score_p2) {
                winner_id = 1;
            } else if (destruction_score_p2 > destruction_score_p1) {
                winner_id = 2;
            } else {
                winner_id = 0;
            }
        } else {
            if (p1_platforms == 0) {
                winner_id = 2;
            } else if (p2_platforms == 0) {
                winner_id = 1;
            }
        }

        // Pantalla de victoria
        uint16_t winner_color = (winner_id == 1) ? COLOR_P1 : (winner_id == 2) ? COLOR_P2 : COLOR_WHITE;

        // Borde
        daos_gfx_fill_rect(40, 60, 240, 140, winner_color);
        daos_gfx_fill_rect(44, 64, 232, 132, COLOR_BLACK);

        // Título
        daos_gfx_draw_text_large(70, 80, "GAME OVER", COLOR_WHITE, COLOR_BLACK, 2);

        // Ganador
        if (winner_id == 1) {
            daos_gfx_draw_text_large(65, 115, "PLAYER 1", COLOR_P1, COLOR_BLACK, 2);
            daos_gfx_draw_text_large(95, 140, "WINS!", COLOR_P1, COLOR_BLACK, 2);
        } else if (winner_id == 2) {
            daos_gfx_draw_text_large(65, 115, "PLAYER 2", COLOR_P2, COLOR_BLACK, 2);
            daos_gfx_draw_text_large(95, 140, "WINS!", COLOR_P2, COLOR_BLACK, 2);
        } else {
            daos_gfx_draw_text_large(100, 125, "DRAW!", COLOR_WHITE, COLOR_BLACK, 2);
        }

        // Marcadores
        char buffer[10];
        daos_gfx_draw_text_large(80, 165, "DESTROYED", COLOR_WHITE, COLOR_BLACK, 1);

        daos_gfx_draw_text_large(60, 180, "P1:", COLOR_P1, COLOR_BLACK, 1);
        integer_to_string((int32_t)destruction_score_p1, buffer);
        daos_gfx_draw_text_large(95, 180, buffer, COLOR_WHITE, COLOR_BLACK, 1);

        daos_gfx_draw_text_large(170, 180, "P2:", COLOR_P2, COLOR_BLACK, 1);
        integer_to_string((int32_t)destruction_score_p2, buffer);
        daos_gfx_draw_text_large(205, 180, buffer, COLOR_WHITE, COLOR_BLACK, 1);

        daos_gfx_draw_text_large(35, 215, "PRESS D15 TO RESTART", COLOR_WHITE, COLOR_BLACK, 1);
    }
}

/* ============================================================ */
/* TAREAS DEL JUEGO                                            */
/* ============================================================ */

void disco_input_task(void) {
    // Lectura de botones según los índices (0=P1 Shield, 1=P1 Up, 2=P1 Down, 3=P1 Shoot)
    // P1 tiene los controles rotados/invertidos

    uint32_t current_p1_shoot_btn = daos_btn_get_count_by_index(0); // Ahora Disparo (Originalmente Shield)
    uint32_t current_p1_up_btn = daos_btn_get_count_by_index(1);
    uint32_t current_p1_down_btn = daos_btn_get_count_by_index(2);
    uint32_t current_p1_shield_btn = daos_btn_get_count_by_index(3); // Ahora Escudo (Originalmente Shoot)

    uint32_t current_p2_shoot = daos_btn_get_count_by_index(4);
    uint32_t current_p2_up = daos_btn_get_count_by_index(5);
    uint32_t current_p2_down = daos_btn_get_count_by_index(6);
    uint32_t current_p2_shield = daos_btn_get_count_by_index(7);
    uint32_t current_d15 = daos_btn_get_count_by_index(BTN_D15_INDEX);

    if (!initialized_input) {
        last_p1_shoot_count = current_p1_shoot_btn;
        last_p1_up_count = current_p1_up_btn;
        last_p1_down_count = current_p1_down_btn;
        last_p1_shield_count = current_p1_shield_btn;
        last_p2_shoot_count = current_p2_shoot;
        last_p2_up_count = current_p2_up;
        last_p2_down_count = current_p2_down;
        last_p2_shield_count = current_p2_shield;
        last_d15_count = current_d15;
        initialized_input = 1;
        daos_sleep_ms(20);
        return;
    }

    // PROTECCIÓN: Verificar estado del juego
    mutex_lock(&disco_game_state_mutex);
    uint8_t local_game_running = game_running;
    mutex_unlock(&disco_game_state_mutex);

    if (!local_game_running) {
        if (current_d15 != last_d15_count) {
            disco_init();
        }

        last_p1_shoot_count = current_p1_shoot_btn;
        last_p1_up_count = current_p1_up_btn;
        last_p1_down_count = current_p1_down_btn;
        last_p1_shield_count = current_p1_shield_btn;
        last_p2_shoot_count = current_p2_shoot;
        last_p2_up_count = current_p2_up;
        last_p2_down_count = current_p2_down;
        last_p2_shield_count = current_p2_shield;
        last_d15_count = current_d15;

        daos_sleep_ms(100);
        return;
    }

    // **JUGADOR 1: CONTROLES MODIFICADOS E INVERTIDOS**

    // P1: Disparo (Botón 0) -> DISPARA DERECHA
    if (current_p1_shoot_btn != last_p1_shoot_count) {
        mutex_lock(&disco_player_mutex);
        if (shield_p1) shield_p1 = 0;

        if (!shield_p1) {
            mutex_lock(&disco_platform_mutex);
            Platform p = platforms[player1_lane];
            mutex_unlock(&disco_platform_mutex);

            mutex_lock(&disco_shot_mutex);
            for (uint8_t i = 0; i < MAX_SHOTS; i++) {
                if (!shots_p1[i].active) {
                    shots_p1[i].x = p.center_x + p.radius_outer + 2; // Dispara a la DERECHA
                    shots_p1[i].y = p.center_y + player1_offset_y;
                    shots_p1[i].vx = 4;
                    shots_p1[i].active = 1;
                    shots_p1[i].color_is_p1 = 1;
                    shots_p1[i].target_lane = player1_lane;
                    p1_shooting_frame_timer = 10;
                    break;
                }
            }
            mutex_unlock(&disco_shot_mutex);
        }
        mutex_unlock(&disco_player_mutex);
        last_p1_shoot_count = current_p1_shoot_btn;
    }

    // P1: Escudo (Botón 3) -> ACTIVA ESCUDO
    if (current_p1_shield_btn != last_p1_shield_count) {
        mutex_lock(&disco_player_mutex);
        shield_p1 = !shield_p1;
        mutex_unlock(&disco_player_mutex);
        last_p1_shield_count = current_p1_shield_btn;
    }

    // P1: Arriba (Botón 1) -> MOVER ABAJO (Invertido)
    if (current_p1_up_btn != last_p1_up_count) {
        mutex_lock(&disco_player_mutex);
        if (shield_p1) shield_p1 = 0;

        if (!shield_p1) {
            mutex_lock(&disco_platform_mutex);
            if (player1_lane < 2 && platforms[player1_lane + 1].exists) {
                player1_lane++;
                player1_offset_x = 0;
                player1_offset_y = 0;
            }
            mutex_unlock(&disco_platform_mutex);
        }
        mutex_unlock(&disco_player_mutex);
        last_p1_up_count = current_p1_up_btn;
    }

    // P1: Abajo (Botón 2) -> MOVER ARRIBA (Invertido)
    if (current_p1_down_btn != last_p1_down_count) {
        mutex_lock(&disco_player_mutex);
        if (shield_p1) shield_p1 = 0;

        if (!shield_p1) {
            mutex_lock(&disco_platform_mutex);
            if (player1_lane > 0 && platforms[player1_lane - 1].exists) {
                player1_lane--;
                player1_offset_x = 0;
                player1_offset_y = 0;
            }
            mutex_unlock(&disco_platform_mutex);
        }
        mutex_unlock(&disco_player_mutex);
        last_p1_down_count = current_p1_down_btn;
    }

    // **JUGADOR 2: CONTROLES ESTÁNDAR**

    // Escudo P2 (Toggle)
    if (current_p2_shield != last_p2_shield_count) {
        mutex_lock(&disco_player_mutex);
        shield_p2 = !shield_p2;
        mutex_unlock(&disco_player_mutex);
        last_p2_shield_count = current_p2_shield;
    }

    // P2: Disparo (Botón 4) -> DISPARA IZQUIERDA
    if (current_p2_shoot != last_p2_shoot_count) {
        mutex_lock(&disco_player_mutex);
        if (shield_p2) shield_p2 = 0;

        if (!shield_p2) {
            mutex_lock(&disco_platform_mutex);
            Platform p = platforms[player2_lane + 3];
            mutex_unlock(&disco_platform_mutex);

            mutex_lock(&disco_shot_mutex);
            for (uint8_t i = 0; i < MAX_SHOTS; i++) {
                if (!shots_p2[i].active) {
                    shots_p2[i].x = p.center_x - p.radius_outer - 2; // Dispara a la IZQUIERDA
                    shots_p2[i].y = p.center_y + player2_offset_y;
                    shots_p2[i].vx = -4;
                    shots_p2[i].active = 1;
                    shots_p2[i].color_is_p1 = 0;
                    shots_p2[i].target_lane = player2_lane;
                    p2_shooting_frame_timer = 10;
                    break;
                }
            }
            mutex_unlock(&disco_shot_mutex);
        }
        mutex_unlock(&disco_player_mutex);
        last_p2_shoot_count = current_p2_shoot;
    }

    // P2: Arriba
    if (current_p2_up != last_p2_up_count) {
        mutex_lock(&disco_player_mutex);
        if (shield_p2) shield_p2 = 0;

        if (!shield_p2) {
            mutex_lock(&disco_platform_mutex);
            if (player2_lane > 0 && platforms[player2_lane + 3 - 1].exists) {
                player2_lane--;
                player2_offset_x = 0;
                player2_offset_y = 0;
            }
            mutex_unlock(&disco_platform_mutex);
        }
        mutex_unlock(&disco_player_mutex);
        last_p2_up_count = current_p2_up;
    }

    // P2: Abajo
    if (current_p2_down != last_p2_down_count) {
        mutex_lock(&disco_player_mutex);
        if (shield_p2) shield_p2 = 0;

        if (!shield_p2) {
            mutex_lock(&disco_platform_mutex);
            if (player2_lane < 2 && platforms[player2_lane + 3 + 1].exists) {
                player2_lane++;
                player2_offset_x = 0;
                player2_offset_y = 0;
            }
            mutex_unlock(&disco_platform_mutex);
        }
        mutex_unlock(&disco_player_mutex);
        last_p2_down_count = current_p2_down;
    }

    daos_sleep_ms(20);
}

void disco_logic_task(void) {
    // PROTECCIÓN: Verificar estado del juego
    mutex_lock(&disco_game_state_mutex);
    uint8_t local_game_running = game_running;
    mutex_unlock(&disco_game_state_mutex);

    if (!local_game_running) {
        daos_sleep_ms(100);
        return;
    }

    simulated_time_ms += LOGIC_CYCLE_MS;

    uint32_t elapsed_ms = simulated_time_ms - start_time_ms;

    if (elapsed_ms >= MAX_GAME_TIME_MS) {
        check_and_show_victory();
        return;
    }

    // PROTECCIÓN: Resetear offsets
    mutex_lock(&disco_player_mutex);
    player1_offset_x = 0;
    player1_offset_y = 0;
    player2_offset_x = 0;
    player2_offset_y = 0;
    uint8_t local_shield_p1 = shield_p1;
    uint8_t local_shield_p2 = shield_p2;
    uint8_t local_p1_lane = player1_lane;
    uint8_t local_p2_lane = player2_lane;
    mutex_unlock(&disco_player_mutex);

    // Actualizar disparos P1 (van a la derecha)
    mutex_lock(&disco_shot_mutex);
    for (uint8_t i = 0; i < MAX_SHOTS; i++) {
        if (shots_p1[i].active) {
            shots_p1[i].x += shots_p1[i].vx;

            mutex_lock(&disco_platform_mutex);
            for (uint8_t p = 3; p < 6; p++) {
                if (platforms[p].exists) {
                    int16_t dx = shots_p1[i].x - platforms[p].center_x;
                    int16_t dy = shots_p1[i].y - platforms[p].center_y;
                    uint32_t dist_sq = (uint32_t)dx * dx + (uint32_t)dy * dy;
                    uint32_t r_sq = (uint32_t)platforms[p].radius_outer * platforms[p].radius_outer;

                    if (dist_sq <= r_sq) {
                        if (local_shield_p2 && (p - 3) == local_p2_lane) {
                            shots_p1[i].active = 0;
                        } else {
                            if (platforms[p].layer > 0) destruction_score_p1++;
                            platforms[p].layer--;
                            if (platforms[p].layer == 0) {
                                platforms[p].exists = 0;
                            }
                            shots_p1[i].active = 0;
                        }
                        break;
                    }
                }
            }
            mutex_unlock(&disco_platform_mutex);

            if (shots_p1[i].x > 320) {
                shots_p1[i].active = 0;
            }
        }
    }
    mutex_unlock(&disco_shot_mutex);

    // Actualizar disparos P2 (van a la izquierda)
    mutex_lock(&disco_shot_mutex);
    for (uint8_t i = 0; i < MAX_SHOTS; i++) {
        if (shots_p2[i].active) {
            shots_p2[i].x += shots_p2[i].vx;

            mutex_lock(&disco_platform_mutex);
            for (uint8_t p = 0; p < 3; p++) {
                if (platforms[p].exists) {
                    int16_t dx = shots_p2[i].x - platforms[p].center_x;
                    int16_t dy = shots_p2[i].y - platforms[p].center_y;
                    uint32_t dist_sq = (uint32_t)dx * dx + (uint32_t)dy * dy;
                    uint32_t r_sq = (uint32_t)platforms[p].radius_outer * platforms[p].radius_outer;

                    if (dist_sq <= r_sq) {
                        if (local_shield_p1 && p == local_p1_lane) {
                            shots_p2[i].active = 0;
                        } else {
                            if (platforms[p].layer > 0) destruction_score_p2++;
                            platforms[p].layer--;
                            if (platforms[p].layer == 0) {
                                platforms[p].exists = 0;
                            }
                            shots_p2[i].active = 0;
                        }
                        break;
                    }
                }
            }
            mutex_unlock(&disco_platform_mutex);

            if (shots_p2[i].x < 0) {
                shots_p2[i].active = 0;
            }
        }
    }
    mutex_unlock(&disco_shot_mutex);

    // Verificar derrota por plataforma destruida
    mutex_lock(&disco_platform_mutex);
    if (!platforms[local_p1_lane].exists) {
        platforms[0].exists = 0;
        platforms[1].exists = 0;
        platforms[2].exists = 0;
    }
    if (!platforms[local_p2_lane + 3].exists) {
        platforms[3].exists = 0;
        platforms[4].exists = 0;
        platforms[5].exists = 0;
    }
    mutex_unlock(&disco_platform_mutex);

    check_and_show_victory();

    daos_sleep_ms(LOGIC_CYCLE_MS);
}

void disco_render_task(void) {
    // PROTECCIÓN: Verificar estado del juego
    mutex_lock(&disco_game_state_mutex);
    uint8_t local_game_running = game_running;
    mutex_unlock(&disco_game_state_mutex);

    if (!local_game_running) {
        daos_sleep_ms(30);
        return;
    }

    if (first_draw) {
        daos_gfx_clear(COLOR_BLACK);

        mutex_lock(&disco_platform_mutex);
        for (uint8_t i = 0; i < 6; i++) {
            draw_platform(&platforms[i]);
            last_layers[i] = platforms[i].layer;
        }
        mutex_unlock(&disco_platform_mutex);

        // INICIALIZAR ÁREA DE LAS BOLAS DE TIEMPO
        daos_gfx_fill_rect(120, 0, 80, 20, COLOR_BLACK);
        last_time_ball_state = 3;
        // FIN DE INICIALIZACIÓN

        mutex_lock(&disco_player_mutex);
        mutex_lock(&disco_platform_mutex);
        Platform p1 = platforms[player1_lane];
        last_p1_x = p1.center_x;
        last_p1_y = p1.center_y;

        Platform p2 = platforms[player2_lane + 3];
        last_p2_x = p2.center_x;
        last_p2_y = p2.center_y;

        last_p1_lane = player1_lane;
        last_p2_lane = player2_lane;
        last_shield_p1 = shield_p1;
        last_shield_p2 = shield_p2;
        mutex_unlock(&disco_platform_mutex);
        mutex_unlock(&disco_player_mutex);

        for (uint8_t i = 0; i < MAX_SHOTS; i++) {
            last_shot_x_p1[i] = -1;
            last_shot_y_p1[i] = -1;
            last_shot_x_p2[i] = -1;
            last_shot_y_p2[i] = -1;
        }

        first_draw = 0;
    }

    // Marcadores superiores
    char buffer[10];

    mutex_lock(&disco_platform_mutex);
    uint8_t p1_count = platforms[0].exists + platforms[1].exists + platforms[2].exists;
    uint8_t p2_count = platforms[3].exists + platforms[4].exists + platforms[5].exists;
    mutex_unlock(&disco_platform_mutex);

    // P1 Score y Escudo
    daos_gfx_draw_text(5, 5, "P1:", COLOR_P1, COLOR_BLACK);
    integer_to_string(p1_count, buffer);
    daos_gfx_draw_text(35, 5, buffer, COLOR_WHITE, COLOR_BLACK);

    mutex_lock(&disco_player_mutex);
    // Texto SHIELD eliminado aquí
    if (shield_p1) {
        daos_gfx_fill_rect(60, 5, 50, 8, COLOR_BLACK); // Limpiar área del texto SHIELD
    } else {
        daos_gfx_fill_rect(60, 5, 50, 8, COLOR_BLACK);
    }
    mutex_unlock(&disco_player_mutex);

    // P2 Score y Escudo
    daos_gfx_draw_text(240, 5, "P2:", COLOR_P2, COLOR_BLACK);
    integer_to_string(p2_count, buffer);
    daos_gfx_draw_text(270, 5, buffer, COLOR_WHITE, COLOR_BLACK);

    mutex_lock(&disco_player_mutex);
    // Texto SHIELD eliminado aquí
    if (shield_p2) {
        daos_gfx_fill_rect(180, 5, 50, 8, COLOR_BLACK); // Limpiar área del texto SHIELD
    } else {
        daos_gfx_fill_rect(180, 5, 50, 8, COLOR_BLACK);
    }
    mutex_unlock(&disco_player_mutex);

    // **LÓGICA DEL CRONÓMETRO DE CÍRCULOS MODIFICADA PARA 15 SEGUNDOS**
    uint32_t elapsed_ms = simulated_time_ms - start_time_ms;
    uint32_t remaining_s = (MAX_GAME_TIME_MS > elapsed_ms) ? (MAX_GAME_TIME_MS - elapsed_ms) / 1000 : 0;

    // El área central donde se dibujan los círculos es (120, 0) a (200, 20)
    daos_gfx_fill_rect(120, 0, 80, 20, COLOR_BLACK);

    // Posiciones de las bolas (centradas en y=10)
    int16_t x_a = 145; // Bola 1 (15s -> 10s restantes)
    int16_t x_b = 160; // Bola 2 (10s -> 5s restantes)
    int16_t x_c = 175; // Bola 3 (5s -> 0s restantes)
    int16_t y_center = 10;

    // Bola 1 (Verde, desaparece cuando quedan <= 10s)
    if (remaining_s > 10) {
        draw_time_ball(x_a, y_center, COLOR_P2);
    } else {
        draw_time_ball(x_a, y_center, COLOR_BLACK);
    }

    // Bola 2 (Amarilla, desaparece cuando quedan <= 5s)
    if (remaining_s > 5) {
        draw_time_ball(x_b, y_center, COLOR_YELLOW);
    } else {
        draw_time_ball(x_b, y_center, COLOR_BLACK);
    }

    // Bola 3 (Roja, desaparece cuando se acaba el tiempo)
    if (remaining_s > 0) {
        draw_time_ball(x_c, y_center, COLOR_RED);
    } else {
        draw_time_ball(x_c, y_center, COLOR_BLACK);
    }

    // Actualizar estado anterior (Mantenemos la variable aunque su utilidad principal se redujo)
    if (remaining_s > 10) last_time_ball_state = 3;
    else if (remaining_s > 5) last_time_ball_state = 2;
    else if (remaining_s > 0) last_time_ball_state = 1;
    else last_time_ball_state = 0;
    // **FIN DE LÓGICA DEL CRONÓMETRO DE CÍRCULOS**


    // Redibujar plataformas dañadas
    mutex_lock(&disco_platform_mutex);
    for (uint8_t i = 0; i < 6; i++) {
        if (platforms[i].layer != last_layers[i] || platforms[i].exists != (last_layers[i] > 0)) {
            draw_platform(&platforms[i]);
            last_layers[i] = platforms[i].layer;
        }
    }
    mutex_unlock(&disco_platform_mutex);

    // Actualizar disparos P1
    mutex_lock(&disco_shot_mutex);
    for (uint8_t i = 0; i < MAX_SHOTS; i++) {
        if (last_shot_x_p1[i] >= 0) {
            daos_gfx_draw_circle_filled(last_shot_x_p1[i], last_shot_y_p1[i], 3, COLOR_BLACK);
        }

        if (shots_p1[i].active) {
            daos_gfx_draw_circle_filled(shots_p1[i].x, shots_p1[i].y, 3, COLOR_P1);
            last_shot_x_p1[i] = shots_p1[i].x;
            last_shot_y_p1[i] = shots_p1[i].y;
        } else {
            last_shot_x_p1[i] = -1;
        }
    }
    mutex_unlock(&disco_shot_mutex);

    // Actualizar disparos P2
    mutex_lock(&disco_shot_mutex);
    for (uint8_t i = 0; i < MAX_SHOTS; i++) {
        if (last_shot_x_p2[i] >= 0) {
            daos_gfx_draw_circle_filled(last_shot_x_p2[i], last_shot_y_p2[i], 3, COLOR_BLACK);
        }

        if (shots_p2[i].active) {
            daos_gfx_draw_circle_filled(shots_p2[i].x, shots_p2[i].y, 3, COLOR_P2);
            last_shot_x_p2[i] = shots_p2[i].x;
            last_shot_y_p2[i] = shots_p2[i].y;
        } else {
            last_shot_x_p2[i] = -1;
        }
    }
    mutex_unlock(&disco_shot_mutex);

    // Calcular posiciones
    mutex_lock(&disco_player_mutex);
    mutex_lock(&disco_platform_mutex);
    Platform p1_platform_current = platforms[player1_lane];
    int16_t p1_x = p1_platform_current.center_x + player1_offset_x;
    int16_t p1_y = p1_platform_current.center_y + player1_offset_y;

    Platform p2_platform_current = platforms[player2_lane + 3];
    int16_t p2_x = p2_platform_current.center_x + player2_offset_x;
    int16_t p2_y = p2_platform_current.center_y + player2_offset_y;
    mutex_unlock(&disco_platform_mutex);
    mutex_unlock(&disco_player_mutex);

    // Sistema de animación
    animation_frame_counter++;
    if (animation_frame_counter >= 15) {
        animation_frame_counter = 0;
    }
    uint8_t current_frame = (animation_frame_counter < 8) ? 0 : 1;

    // Decrementar timers de disparo
    if (p1_shooting_frame_timer > 0) p1_shooting_frame_timer--;
    if (p2_shooting_frame_timer > 0) p2_shooting_frame_timer--;

    // Borrar P1
    mutex_lock(&disco_player_mutex);
    uint8_t local_p1_lane = player1_lane;
    uint8_t local_p2_lane = player2_lane;
    uint8_t local_shield_p1 = shield_p1;
    uint8_t local_shield_p2 = shield_p2;
    mutex_unlock(&disco_player_mutex);

    if (last_p1_x != p1_x || last_p1_y != p1_y ||
        last_p1_lane != local_p1_lane || last_shield_p1 != local_shield_p1 ||
        animation_frame_counter == 0 || animation_frame_counter == 8) {
        erase_player_at(last_p1_x, last_p1_y, last_p1_lane);
    }

    // Borrar P2
    if (last_p2_x != p2_x || last_p2_y != p2_y ||
        last_p2_lane != local_p2_lane || last_shield_p2 != local_shield_p2 ||
        animation_frame_counter == 0 || animation_frame_counter == 8) {
        erase_player_at(last_p2_x, last_p2_y, last_p2_lane + 3);
    }

    // Escudo P1 (P1 defiende derecha)
    if (local_shield_p1) {
        mutex_lock(&disco_platform_mutex);
        if (platforms[local_p1_lane].exists) {
            Platform p = platforms[local_p1_lane];
            int16_t line_x_center = p.center_x + p.radius_outer + 1;
            int16_t line_y_start = p.center_y - p.radius_outer - 1;
            int16_t line_y_end = p.center_y + p.radius_outer + 1;
            daos_gfx_fill_rect(line_x_center - 1, line_y_start, 3, line_y_end - line_y_start + 1, COLOR_WHITE);
        }
        mutex_unlock(&disco_platform_mutex);
    }

    // Escudo P2 (P2 defiende izquierda)
    if (local_shield_p2) {
        mutex_lock(&disco_platform_mutex);
        if (platforms[local_p2_lane + 3].exists) {
            Platform p = platforms[local_p2_lane + 3];
            int16_t line_x_center = p.center_x - p.radius_outer - 1;
            int16_t line_y_start = p.center_y - p.radius_outer - 1;
            int16_t line_y_end = p.center_y + p.radius_outer + 1;
            daos_gfx_fill_rect(line_x_center - 1, line_y_start, 3, line_y_end - line_y_start + 1, COLOR_WHITE);
        }
        mutex_unlock(&disco_platform_mutex);
    }

    // Dibujar jugador 1
    mutex_lock(&disco_platform_mutex);
    uint8_t p1_exists = platforms[local_p1_lane].exists;
    mutex_unlock(&disco_platform_mutex);

    if (p1_exists) {
        const uint32_t* p1_sprite;

        if (p1_shooting_frame_timer > 0) {
            p1_sprite = sprite_p1_shoot;
        } else {
            p1_sprite = (current_frame == 0) ? sprite_p1_idle1 : sprite_p1_idle2;
        }

        draw_sprite_at(p1_x, p1_y, p1_sprite);
    }

    // Dibujar jugador 2
    mutex_lock(&disco_platform_mutex);
    uint8_t p2_exists = platforms[local_p2_lane + 3].exists;
    mutex_unlock(&disco_platform_mutex);

    if (p2_exists) {
        const uint32_t* p2_sprite;

        if (p2_shooting_frame_timer > 0) {
            p2_sprite = sprite_p2_shoot;
        } else {
            p2_sprite = (current_frame == 0) ? sprite_p2_idle1 : sprite_p2_idle2;
        }

        draw_sprite_at(p2_x, p2_y, p2_sprite);
    }

    // Actualizar variables de estado anterior
    last_p1_x = p1_x;
    last_p1_y = p1_y;
    last_p2_x = p2_x;
    last_p2_y = p2_y;
    last_p1_lane = local_p1_lane;
    last_p2_lane = local_p2_lane;
    last_shield_p1 = local_shield_p1;
    last_shield_p2 = local_shield_p2;

    daos_sleep_ms(30);
}

/* ============================================================ */
/* INICIALIZACIÓN Y CLEANUP                                    */
/* ============================================================ */

void disco_init(void) {
    // PROTECCIÓN: Detener el juego de forma segura
    mutex_lock(&disco_game_state_mutex);
    game_running = 0;
    mutex_unlock(&disco_game_state_mutex);

    daos_sleep_ms(100);

    daos_gfx_clear(COLOR_BLACK);

    // Reinicio de variables globales
    mutex_lock(&disco_player_mutex);
    player1_lane = 1;
    player1_offset_x = 0;
    player1_offset_y = 0;
    player2_lane = 1;
    player2_offset_x = 0;
    player2_offset_y = 0;
    shield_p1 = 0;
    shield_p2 = 0;
    mutex_unlock(&disco_player_mutex);

    simulated_time_ms = 0;
    start_time_ms = 0;
    destruction_score_p1 = 0;
    destruction_score_p2 = 0;

    // Reinicio de estados estáticos
    initialized_input = 0;
    first_draw = 1;
    animation_frame_counter = 0;
    p1_shooting_frame_timer = 0;
    p2_shooting_frame_timer = 0;

    // Inicialización del estado de la bola de tiempo
    last_time_ball_state = 3;

    // Resetear variables de renderizado
    last_p1_x = 0;
    last_p1_y = 0;
    last_p2_x = 0;
    last_p2_y = 0;
    last_p1_lane = 1;
    last_p2_lane = 1;
    last_shield_p1 = 0;
    last_shield_p2 = 0;

    for (uint8_t i = 0; i < 6; i++) {
        last_layers[i] = 3;
    }

    // Reinicio de disparos
    mutex_lock(&disco_shot_mutex);
    for (uint8_t i = 0; i < MAX_SHOTS; i++) {
        shots_p1[i].active = 0;
        shots_p1[i].x = 0;
        shots_p1[i].y = 0;
        shots_p1[i].vx = 0;

        shots_p2[i].active = 0;
        shots_p2[i].x = 0;
        shots_p2[i].y = 0;
        shots_p2[i].vx = 0;

        last_shot_x_p1[i] = -1;
        last_shot_y_p1[i] = -1;
        last_shot_x_p2[i] = -1;
        last_shot_y_p2[i] = -1;
    }
    mutex_unlock(&disco_shot_mutex);

    // Reinicio de plataformas
    mutex_lock(&disco_platform_mutex);
    platforms[0].center_x = 70;
    platforms[0].center_y = 60;
    platforms[0].radius_inner = 8;
    platforms[0].radius_middle = 14;
    platforms[0].radius_outer = 20;
    platforms[0].layer = 3;
    platforms[0].exists = 1;

    platforms[1].center_x = 70;
    platforms[1].center_y = 135;
    platforms[1].radius_inner = 8;
    platforms[1].radius_middle = 14;
    platforms[1].radius_outer = 20;
    platforms[1].layer = 3;
    platforms[1].exists = 1;

    platforms[2].center_x = 70;
    platforms[2].center_y = 210;
    platforms[2].radius_inner = 8;
    platforms[2].radius_middle = 14;
    platforms[2].radius_outer = 20;
    platforms[2].layer = 3;
    platforms[2].exists = 1;

    platforms[3].center_x = 250;
    platforms[3].center_y = 60;
    platforms[3].radius_inner = 8;
    platforms[3].radius_middle = 14;
    platforms[3].radius_outer = 20;
    platforms[3].layer = 3;
    platforms[3].exists = 1;

    platforms[4].center_x = 250;
    platforms[4].center_y = 135;
    platforms[4].radius_inner = 8;
    platforms[4].radius_middle = 14;
    platforms[4].radius_outer = 20;
    platforms[4].layer = 3;
    platforms[4].exists = 1;

    platforms[5].center_x = 250;
    platforms[5].center_y = 210;
    platforms[5].radius_inner = 8;
    platforms[5].radius_middle = 14;
    platforms[5].radius_outer = 20;
    platforms[5].layer = 3;
    platforms[5].exists = 1;
    mutex_unlock(&disco_platform_mutex);

    // INICIALIZACIÓN: Configurar mutexes y semáforos si no están inicializados
    static uint8_t mutexes_initialized = 0;
    if (!mutexes_initialized) {
        mutex_init(&disco_game_state_mutex);
        mutex_init(&disco_platform_mutex);
        mutex_init(&disco_player_mutex);
        mutex_init(&disco_shot_mutex);

        // Inicializar semáforos (NO SE USAN - se eliminaron las sincronizaciones bloqueantes)
        // Los dejamos inicializados por si se necesitan en el futuro
        sem_init(&disco_input_ready_sem, 0, 1);
        sem_init(&disco_logic_ready_sem, 0, 1);
        sem_init(&disco_render_ready_sem, 0, 1);

        mutexes_initialized = 1;
    }

    // PROTECCIÓN: Activar el juego de forma segura
    mutex_lock(&disco_game_state_mutex);
    game_running = 1;
    mutex_unlock(&disco_game_state_mutex);
}

void disco_cleanup(void) {
    // PROTECCIÓN: Limpiar de forma segura
    mutex_lock(&disco_game_state_mutex);
    game_running = 0;
    daos_gfx_clear(COLOR_BLACK);
    mutex_unlock(&disco_game_state_mutex);

    // Nota: Los mutexes y semáforos no necesitan destrucción explícita
    // en este sistema embebido, pero podrían agregarse funciones
    // mutex_destroy() y sem_destroy() si fuera necesario
}

/* ============================================================ */
/* API PÚBLICA                                                 */
/* ============================================================ */

disco_state_t disco_get_state(void) {
    // PROTECCIÓN: Lectura segura del estado
    mutex_lock(&disco_game_state_mutex);
    disco_state_t state = game_running ? DISCO_RUNNING : DISCO_GAME_OVER;
    mutex_unlock(&disco_game_state_mutex);
    return state;
}

uint16_t disco_get_score_p1(void) {
    // PROTECCIÓN: Lectura segura del score
    // destruction_score_p1 es de tipo uint16_t, la lectura es atómica en ARM Cortex-M4
    // pero por consistencia usamos mutex (aunque en este caso no es estrictamente necesario)
    uint16_t score = destruction_score_p1;
    return score;
}

uint16_t disco_get_score_p2(void) {
    // PROTECCIÓN: Lectura segura del score
    uint16_t score = destruction_score_p2;
    return score;
}
