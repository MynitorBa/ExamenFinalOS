#include "reconocedor.h"
#include "api.h"
#include "sync.h"
#include <stdint.h>

/* ============================================================================
 * RECONOCEDOR ESPÍA - VERSIÓN FINAL COMPLETA CON SINCRONIZACIÓN
 * Fase 1: 2D - Esconderse y colocar maniquíes (con capacidad de recogerlos)
 * Fase 2: 3D - Buscar y abrir cajas (máximo mitad de cajas)
 * Victoria: Encontrar al escondedor
 * Derrota: Abrir 2 maniquíes sin encontrar al escondedor
 * ============================================================================
 */

// ========================================================================
// CONSTANTES GLOBALES
// ========================================================================
#define TILE_SIZE 16
#define TILE_DATA_SIZE 10
#define MAP_WIDTH 19
#define MAP_HEIGHT 14
#define MAP_OFFSET_X 8
#define MAP_OFFSET_Y 8

// Configuración 3D
#define NUM_RAYS 60
#define GAME_AREA_WIDTH 280
#define GAME_AREA_HEIGHT 160
#define HUD_Y (GAME_AREA_HEIGHT)
#define HUD_HEIGHT 80
#define MINIMAP_SIZE 70
#define MOVE_COOLDOWN_MS 150
#define TILE_SIZE_3D 128

// ========================================================================
// MUTEXES Y SEMÁFOROS PARA SINCRONIZACIÓN
// ========================================================================

// Mutexes para protección de recursos compartidos
mutex_t reconocedor_game_state_mutex;
mutex_t reconocedor_map_mutex;
mutex_t reconocedor_player_mutex;
mutex_t reconocedor_maniqui_mutex;

// Semáforos para sincronización de tareas
sem_t reconocedor_input_ready_sem;
sem_t reconocedor_logic_ready_sem;
sem_t reconocedor_render_ready_sem;

// ========================================================================
// ENUMS Y ESTRUCTURAS
// ========================================================================
typedef enum {
	DIR_UP = 0,
	DIR_RIGHT = 1,
	DIR_DOWN = 2,
	DIR_LEFT = 3
} Direccion;

typedef enum {
	TILE_VACIO = 0,
	TILE_PARED_AZUL,
	TILE_PARED_VERDE,
	TILE_PARED_AMARILLA,
	TILE_CAJA,
	TILE_CENTRO,
	TILE_RAIZ
} TipoTile;

typedef struct {
	uint8_t x;
	uint8_t y;
	Direccion dir;
	uint8_t desplegado;
} Maniqui;

typedef enum {
	FASE_2D_MOVIENDO = 0,
	FASE_2D_CONFIRMANDO,
	FASE_2D_ESCONDIDO,
	FASE_TRANSICION,
	FASE_3D_BUSCANDO,
	FASE_VICTORIA,
	FASE_DERROTA
} FaseJuego;

typedef struct {
	int16_t dx;
	int16_t dy;
} RayDir;

typedef struct {
	uint16_t distance;
	uint8_t wall_type;
	uint8_t side;
	uint8_t tex_x;
} RayHit;

// ========================================================================
// COLORES
// ========================================================================
#define COLOR_BLACK       0x0000
#define COLOR_WHITE       0xFFFF
#define COLOR_CYAN        0x07FF
#define COLOR_RED         0xF800
#define COLOR_YELLOW_BG   0xFFE0
#define COLOR_WALL_BLUE   0x001F
#define COLOR_WALL_GREEN  0x07E0
#define COLOR_WALL_YELLOW 0xFFE0
#define COLOR_WALL_ORANGE 0xFD20
#define COLOR_WALL_PURPLE 0x781F
#define COLOR_SKY         0x4A49
#define COLOR_FLOOR       0x3186
#define COLOR_HUD_BG      0x2104
#define COLOR_PLAYER      0xF81F
#define COLOR_MINIMAP_BG  0x18C3
#define COLOR_GREEN       0x07E0
#define COLOR_GRAY        0xAD55

// ========================================================================
// SPRITES 2D (10x10 pixels)
// ========================================================================
static const uint32_t sprite_pared_azul[100] = {
	0xff4930b0, 0xff23b6ce, 0xff7058d1, 0xff7058d1, 0xff7058d1, 0xff7058d1, 0xff7058d1, 0xff7058d1, 0xff7058d1, 0xff4930b0,
	0xff7058d1, 0xff4930b0, 0xff4930b0, 0xff4930b0, 0xff4930b0, 0xff4930b0, 0xff4930b0, 0xff4930b0, 0xff4930b0, 0xff7058d1,
	0xff7058d1, 0xff4930b0, 0xff4930b0, 0xff7058d1, 0xff4930b0, 0xff4930b0, 0xff7058d1, 0xff4930b0, 0xff4930b0, 0xff7058d1,
	0xff7058d1, 0xff4930b0, 0xff4930b0, 0xff7058d1, 0xff4930b0, 0xff4930b0, 0xff7058d1, 0xff4930b0, 0xff4930b0, 0xff7058d1,
	0xff7058d1, 0xff4930b0, 0xff4930b0, 0xff7058d1, 0xff4930b0, 0xff4930b0, 0xff7058d1, 0xff4930b0, 0xff4930b0, 0xff23b6ce,
	0xff7058d1, 0xff4930b0, 0xff4930b0, 0xff23b6ce, 0xff4930b0, 0xff4930b0, 0xff23b6ce, 0xff4930b0, 0xff4930b0, 0xff23b6ce,
	0xff7058d1, 0xff4930b0, 0xff4930b0, 0xff23b6ce, 0xff4930b0, 0xff4930b0, 0xff23b6ce, 0xff4930b0, 0xff4930b0, 0xff23b6ce,
	0xff7058d1, 0xff4930b0, 0xff4930b0, 0xff23b6ce, 0xff4930b0, 0xff4930b0, 0xff23b6ce, 0xff4930b0, 0xff4930b0, 0xff23b6ce,
	0xff23b6ce, 0xff4930b0, 0xff4930b0, 0xff4930b0, 0xff4930b0, 0xff4930b0, 0xff4930b0, 0xff4930b0, 0xff4930b0, 0xff23b6ce,
	0xff4930b0, 0xff7058d1, 0xff7058d1, 0xff7058d1, 0xff7058d1, 0xff7058d1, 0xff7058d1, 0xff7058d1, 0xff23b6ce, 0xff4930b0
};

static const uint32_t sprite_pared_verde[100] = {
	0xff30b030, 0xff33ce23, 0xff58d168, 0xff58d168, 0xff58d168, 0xff58d168, 0xff58d168, 0xff58d168, 0xff58d168, 0xff30b030,
	0xff58d168, 0xff30b030, 0xff30b030, 0xff30b030, 0xff30b030, 0xff30b030, 0xff30b030, 0xff30b030, 0xff30b030, 0xff58d168,
	0xff58d168, 0xff30b030, 0xff30b030, 0xff58d168, 0xff30b030, 0xff30b030, 0xff58d168, 0xff30b030, 0xff30b030, 0xff58d168,
	0xff58d168, 0xff30b030, 0xff30b030, 0xff58d168, 0xff30b030, 0xff30b030, 0xff58d168, 0xff30b030, 0xff30b030, 0xff58d168,
	0xff58d168, 0xff30b030, 0xff30b030, 0xff58d168, 0xff30b030, 0xff30b030, 0xff58d168, 0xff30b030, 0xff30b030, 0xff33ce23,
	0xff58d168, 0xff30b030, 0xff30b030, 0xff33ce23, 0xff30b030, 0xff30b030, 0xff33ce23, 0xff30b030, 0xff30b030, 0xff33ce23,
	0xff58d168, 0xff30b030, 0xff30b030, 0xff33ce23, 0xff30b030, 0xff30b030, 0xff33ce23, 0xff30b030, 0xff30b030, 0xff33ce23,
	0xff58d168, 0xff30b030, 0xff30b030, 0xff33ce23, 0xff30b030, 0xff30b030, 0xff33ce23, 0xff30b030, 0xff30b030, 0xff33ce23,
	0xff33ce23, 0xff30b030, 0xff30b030, 0xff30b030, 0xff30b030, 0xff30b030, 0xff30b030, 0xff30b030, 0xff30b030, 0xff33ce23,
	0xff30b030, 0xff58d168, 0xff58d168, 0xff58d168, 0xff58d168, 0xff58d168, 0xff58d168, 0xff58d168, 0xff33ce23, 0xff30b030
};

static const uint32_t sprite_pared_amarilla[100] = {
	0xffa7b030, 0xffc1ce23, 0xffd1c558, 0xffd1c558, 0xffd1c558, 0xffd1c558, 0xffd1c558, 0xffd1c558, 0xffd1c558, 0xffa7b030,
	0xffd1c558, 0xffa7b030, 0xffa7b030, 0xffa7b030, 0xffa7b030, 0xffa7b030, 0xffa7b030, 0xffa7b030, 0xffa7b030, 0xffd1c558,
	0xffd1c558, 0xffa7b030, 0xffa7b030, 0xffd1c558, 0xffa7b030, 0xffa7b030, 0xffd1c558, 0xffa7b030, 0xffa7b030, 0xffd1c558,
	0xffd1c558, 0xffa7b030, 0xffa7b030, 0xffd1c558, 0xffa7b030, 0xffa7b030, 0xffd1c558, 0xffa7b030, 0xffa7b030, 0xffd1c558,
	0xffd1c558, 0xffa7b030, 0xffa7b030, 0xffd1c558, 0xffa7b030, 0xffa7b030, 0xffd1c558, 0xffa7b030, 0xffa7b030, 0xffc1ce23,
	0xffd1c558, 0xffa7b030, 0xffa7b030, 0xffc1ce23, 0xffa7b030, 0xffa7b030, 0xffc1ce23, 0xffa7b030, 0xffa7b030, 0xffc1ce23,
	0xffd1c558, 0xffa7b030, 0xffa7b030, 0xffc1ce23, 0xffa7b030, 0xffa7b030, 0xffc1ce23, 0xffa7b030, 0xffa7b030, 0xffc1ce23,
	0xffd1c558, 0xffa7b030, 0xffa7b030, 0xffc1ce23, 0xffa7b030, 0xffa7b030, 0xffc1ce23, 0xffa7b030, 0xffa7b030, 0xffc1ce23,
	0xffc1ce23, 0xffa7b030, 0xffa7b030, 0xffa7b030, 0xffa7b030, 0xffa7b030, 0xffa7b030, 0xffa7b030, 0xffa7b030, 0xffc1ce23,
	0xffa7b030, 0xffd1c558, 0xffd1c558, 0xffd1c558, 0xffd1c558, 0xffd1c558, 0xffd1c558, 0xffd1c558, 0xffc1ce23, 0xffa7b030
};

static const uint32_t sprite_caja[100] = {
	0xffa65701, 0xff7e4c15, 0xff7e4c15, 0xff7e4c15, 0xff7e4c15, 0xff7e4c15, 0xff7e4c15, 0xff7e4c15, 0xff7e4c15, 0xffa65701,
	0xff7e4c15, 0xff7e4c15, 0xffa65701, 0xffa65701, 0xffa65701, 0xffa65701, 0xffa65701, 0xffa65701, 0xffa65701, 0xff7e4c15,
	0x00000000, 0xffa65701, 0xffe08b2f, 0xffa65701, 0xffe08b2f, 0xffe08b2f, 0xffa65701, 0xffe08b2f, 0xffa65701, 0x00000000,
	0x00000000, 0xff7e4c15, 0xffe08b2f, 0xff7e4c15, 0xffe08b2f, 0xffe08b2f, 0xff7e4c15, 0xffe08b2f, 0xffa65701, 0x00000000,
	0x00000000, 0xff7e4c15, 0xffe08b2f, 0xff7e4c15, 0xffe08b2f, 0xffe08b2f, 0xff7e4c15, 0xffe08b2f, 0xffa65701, 0x00000000,
	0x00000000, 0xff7e4c15, 0xffe08b2f, 0xff7e4c15, 0xffe08b2f, 0xffe08b2f, 0xff7e4c15, 0xffe08b2f, 0xffa65701, 0x00000000,
	0x00000000, 0xff7e4c15, 0xffe08b2f, 0xff7e4c15, 0xffe08b2f, 0xffe08b2f, 0xff7e4c15, 0xffe08b2f, 0xffa65701, 0x00000000,
	0x00000000, 0xff7e4c15, 0xffe08b2f, 0xff7e4c15, 0xffe08b2f, 0xffe08b2f, 0xff7e4c15, 0xffe08b2f, 0xffa65701, 0x00000000,
	0xffa65701, 0xff7e4c15, 0xff7e4c15, 0xff7e4c15, 0xff7e4c15, 0xff7e4c15, 0xff7e4c15, 0xff7e4c15, 0xff7e4c15, 0xffa65701,
	0xffa65701, 0xffa65701, 0xffa65701, 0xffa65701, 0xffa65701, 0xffa65701, 0xffa65701, 0xffa65701, 0xffa65701, 0xffa65701
};

static const uint32_t sprite_jugador[100] = {
	0x00000000, 0xff24a4b9, 0xffb9b9b9, 0xff24a4b9, 0xffb9b9b9, 0xffb9b9b9, 0xff24a4b9, 0xffb9b9b9, 0xff24a4b9, 0x00000000,
	0xff24a4b9, 0xff24a4b9, 0xff24a4b9, 0xff24a4b9, 0xff24a4b9, 0xff24a4b9, 0xff24a4b9, 0xff24a4b9, 0xff24a4b9, 0xff24a4b9,
	0xffb9b9b9, 0xff24a4b9, 0xff24a4b9, 0xffb9b9b9, 0xffb9b9b9, 0xffb9b9b9, 0xffb9b9b9, 0xff24a4b9, 0xff24a4b9, 0xffb9b9b9,
	0xffb9b9b9, 0xff24a4b9, 0xffb9b9b9, 0xff202020, 0xffb9b9b9, 0xffb9b9b9, 0xff202020, 0xffb9b9b9, 0xff24a4b9, 0xffb9b9b9,
	0xff24a4b9, 0xff24a4b9, 0xffb9b9b9, 0xff202020, 0xffb9b9b9, 0xffb9b9b9, 0xff202020, 0xffb9b9b9, 0xff24a4b9, 0xff24a4b9,
	0x00000000, 0xff24a4b9, 0xffb9b9b9, 0xffb9b9b9, 0xffb9b9b9, 0xffb9b9b9, 0xffb9b9b9, 0xffb9b9b9, 0xff24a4b9, 0x00000000,
	0x00000000, 0x00000000, 0xffb9b9b9, 0xffb9b9b9, 0xffb9b9b9, 0xffb9b9b9, 0xffb9b9b9, 0xffb9b9b9, 0x00000000, 0x00000000,
	0xff24a4b9, 0xff24a4b9, 0xff24a4b9, 0xffb9b9b9, 0xff24a4b9, 0xff24a4b9, 0xffb9b9b9, 0xff24a4b9, 0xff24a4b9, 0xff24a4b9,
	0xffb9b9b9, 0xff24a4b9, 0xff24a4b9, 0xff24a4b9, 0xffb9b9b9, 0xffb9b9b9, 0xff24a4b9, 0xff24a4b9, 0xff24a4b9, 0xffb9b9b9,
	0x00000000, 0xff24a4b9, 0xff24a4b9, 0xff24a4b9, 0x00000000, 0x00000000, 0xff24a4b9, 0xff24a4b9, 0xff24a4b9, 0x00000000
};

static const uint32_t sprite_maniqui_1[100] = {
	0x00000000, 0xff000000, 0xff252525, 0xff252525, 0xff252525, 0xff252525, 0xff252525, 0xff252525, 0xff000000, 0x00000000,
	0xff000000, 0xffffe608, 0xffffe608, 0xff252525, 0xff252525, 0xff252525, 0xff252525, 0xffffe608, 0xffffe608, 0xff000000,
	0xff000000, 0xff000000, 0xffffe608, 0xff252525, 0xff252525, 0xff252525, 0xff252525, 0xffffe608, 0xff000000, 0xff000000,
	0xff000000, 0xff000000, 0xffffe608, 0xff000000, 0xff252525, 0xff252525, 0xff000000, 0xffffe608, 0xff000000, 0xff000000,
	0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff252525, 0xff252525, 0xff000000, 0xff000000, 0xff000000, 0xff000000,
	0x00000000, 0xff000000, 0xff000000, 0xffffe608, 0xff000000, 0xff000000, 0xffffe608, 0xff000000, 0xff000000, 0x00000000,
	0x00000000, 0x00000000, 0xff252525, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff252525, 0x00000000, 0x00000000,
	0xff000000, 0xffffe608, 0xff252525, 0xffffe608, 0xff252525, 0xff252525, 0xffffe608, 0xff252525, 0xffffe608, 0xff000000,
	0xff252525, 0xff000000, 0xff000000, 0xffffe608, 0xff000000, 0xff000000, 0xffffe608, 0xff000000, 0xff000000, 0xff252525,
	0x00000000, 0xff000000, 0xff000000, 0xff000000, 0x00000000, 0x00000000, 0xff000000, 0xff000000, 0xff000000, 0x00000000
};

static const uint32_t sprite_maniqui_2[100] = {
	0x00000000, 0xff000000, 0xff252525, 0xff252525, 0xff252525, 0xff252525, 0xff252525, 0xff252525, 0xff000000, 0x00000000,
	0xff000000, 0xffff0000, 0xffff0000, 0xff252525, 0xff252525, 0xff252525, 0xff252525, 0xffff0000, 0xffff0000, 0xff000000,
	0xff000000, 0xff000000, 0xffff0000, 0xff252525, 0xff252525, 0xff252525, 0xff252525, 0xffff0000, 0xff000000, 0xff000000,
	0xff000000, 0xff000000, 0xffff0000, 0xff000000, 0xff252525, 0xff252525, 0xff000000, 0xffff0000, 0xff000000, 0xff000000,
	0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff252525, 0xff252525, 0xff000000, 0xff000000, 0xff000000, 0xff000000,
	0x00000000, 0xff000000, 0xff000000, 0xffff0000, 0xff000000, 0xff000000, 0xffff0000, 0xff000000, 0xff000000, 0x00000000,
	0x00000000, 0x00000000, 0xff252525, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff252525, 0x00000000, 0x00000000,
	0xff000000, 0xffff0000, 0xff252525, 0xff252525, 0xff252525, 0xff252525, 0xff252525, 0xff252525, 0xffff0000, 0xff000000,
	0xff252525, 0xff000000, 0xff000000, 0xffff0000, 0xff000000, 0xff000000, 0xffff0000, 0xff000000, 0xff000000, 0xff252525,
	0x00000000, 0xff000000, 0xff000000, 0xff000000, 0x00000000, 0x00000000, 0xff000000, 0xff000000, 0xff000000, 0x00000000
};

// Textura 3D para cajas (10x10 - conversión RGB565)
static const uint16_t textura_caja_3d[100] = {
	0xD2E0, 0xA260, 0xA260, 0xA260, 0xA260, 0xA260, 0xA260, 0xA260, 0xA260, 0xD2E0,
	0xA260, 0xA260, 0xD2E0, 0xD2E0, 0xD2E0, 0xD2E0, 0xD2E0, 0xD2E0, 0xD2E0, 0xA260,
	0x0000, 0xD2E0, 0xF457, 0xD2E0, 0xF457, 0xF457, 0xD2E0, 0xF457, 0xD2E0, 0x0000,
	0x0000, 0xA260, 0xF457, 0xA260, 0xF457, 0xF457, 0xA260, 0xF457, 0xD2E0, 0x0000,
	0x0000, 0xA260, 0xF457, 0xA260, 0xF457, 0xF457, 0xA260, 0xF457, 0xD2E0, 0x0000,
	0x0000, 0xA260, 0xF457, 0xA260, 0xF457, 0xF457, 0xA260, 0xF457, 0xD2E0, 0x0000,
	0x0000, 0xA260, 0xF457, 0xA260, 0xF457, 0xF457, 0xA260, 0xF457, 0xD2E0, 0x0000,
	0x0000, 0xA260, 0xF457, 0xA260, 0xF457, 0xF457, 0xA260, 0xF457, 0xD2E0, 0x0000,
	0xD2E0, 0xA260, 0xA260, 0xA260, 0xA260, 0xA260, 0xA260, 0xA260, 0xA260, 0xD2E0,
	0xD2E0, 0xD2E0, 0xD2E0, 0xD2E0, 0xD2E0, 0xD2E0, 0xD2E0, 0xD2E0, 0xD2E0, 0xD2E0
};

// ========================================================================
// VARIABLES GLOBALES (Limpiadas de referencias a FPS)
// ========================================================================
static uint8_t game_running = 0;
static TipoTile mapa[MAP_HEIGHT][MAP_WIDTH];
static uint8_t first_draw = 1;
static uint8_t visitado[MAP_HEIGHT][MAP_WIDTH];

// Fase del juego
static FaseJuego fase_actual = FASE_2D_MOVIENDO;

// JUGADOR ESCONDEDOR (Fase 2D)
static uint8_t escondedor_x;
static uint8_t escondedor_y;
static Direccion escondedor_dir = DIR_RIGHT;
static uint8_t escondedor_oculto = 0;

// JUGADOR BUSCADOR (Fase 3D)
static uint8_t buscador_x;
static uint8_t buscador_y;
static int buscador_angle = 0;

// Maniquíes
static Maniqui maniqui_1 = {0, 0, DIR_RIGHT, 0};
static Maniqui maniqui_2 = {0, 0, DIR_RIGHT, 0};

// Motor 3D
static RayDir ray_table[4][NUM_RAYS];
static int last_rendered_x_3d = -1;
static int last_rendered_y_3d = -1;
static int last_rendered_angle_3d = -1;
static volatile uint32_t move_count_3d = 0;
static volatile uint32_t turn_count_3d = 0;
static uint8_t minimap_needs_redraw = 1;
static uint8_t hud_initialized = 0;
static volatile uint32_t debug_update_counter = 0;
static const int8_t dir_x_3d[4] = {0, 1, 0, -1};
static const int8_t dir_y_3d[4] = {-1, 0, 1, 0};

// Sistema de cajas y lógica de juego
static uint8_t cajas_abiertas[MAP_HEIGHT][MAP_WIDTH];
static uint8_t total_cajas_en_mapa = 0;
static uint8_t max_cajas_abrir = 0;
static uint8_t cajas_abiertas_count = 0;
static uint8_t maniquies_encontrados = 0;

// ========================================================================
// PROTOTIPOS
// ========================================================================
static void generar_mapa_laberinto(void);
static void fase_2d_render(void);
static void fase_3d_render(void);
static void init_ray_table_3d(void);
static void draw_tile(uint8_t x, uint8_t y, const uint32_t* sprite_data);
static void draw_tile_rotated(uint8_t x, uint8_t y, const uint32_t* sprite_data, Direccion dir);
static void redraw_tile_content(uint8_t x, uint8_t y);

// ========================================================================
// FUNCIONES AUXILIARES
// ========================================================================
static inline uint8_t random_range(uint8_t min, uint8_t max) {
	return (uint8_t)daos_random_range(min, max);
}

static uint8_t es_valido(int8_t x, int8_t y) {
	return x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT;
}

static uint8_t is_maniqui_at(uint8_t x, uint8_t y) {
	// PROTECCIÓN: Lectura segura de maniquíes
	mutex_lock(&reconocedor_maniqui_mutex);
	uint8_t result = 0;

	if (maniqui_1.desplegado && maniqui_1.x == x && maniqui_1.y == y) {
		result = 1;
	}
	if (maniqui_2.desplegado && maniqui_2.x == x && maniqui_2.y == y) {
		result = 1;
	}

	mutex_unlock(&reconocedor_maniqui_mutex);
	return result;
}

// ========================================================================
// GENERACIÓN DE MAPA
// ========================================================================
static void generar_laberinto_recursivo(int8_t x, int8_t y) {
	visitado[y][x] = 1;
	mapa[y][x] = TILE_VACIO;

	int8_t dx[] = {0, 2, 0, -2};
	int8_t dy[] = {-2, 0, 2, 0};

	for (uint8_t i = 0; i < 4; i++) {
		uint8_t j = random_range(0, 3);
		int8_t temp_dx = dx[i];
		int8_t temp_dy = dy[i];
		dx[i] = dx[j];
		dy[i] = dy[j];
		dx[j] = temp_dx;
		dy[j] = temp_dy;
	}

	for (uint8_t i = 0; i < 4; i++) {
		int8_t nx = x + dx[i];
		int8_t ny = y + dy[i];

		if (es_valido(nx, ny) && !visitado[ny][nx]) {
			mapa[y + dy[i]/2][x + dx[i]/2] = TILE_VACIO;
			generar_laberinto_recursivo(nx, ny);
		}
	}
}

static void crear_area_central(void) {
	int8_t centro_x = MAP_WIDTH / 2;
	int8_t centro_y = MAP_HEIGHT / 2;

	for (int8_t i = centro_y - 1; i <= centro_y + 1; i++) {
		for (int8_t j = centro_x - 1; j <= centro_x + 1; j++) {
			if (es_valido(j, i)) {
				mapa[i][j] = TILE_CENTRO;
			}
		}
	}
}

static void colocar_cajas(void) {
	total_cajas_en_mapa = 0;
	uint8_t cajas_colocadas = 0;
	uint8_t max_cajas = random_range(15, 20);
	uint16_t intentos = 0;

	while (cajas_colocadas < max_cajas && intentos < 200) {
		uint8_t x = random_range(0, MAP_WIDTH - 1);
		uint8_t y = random_range(0, MAP_HEIGHT - 1);

		if (mapa[y][x] == TILE_VACIO) {
			mapa[y][x] = TILE_CAJA;
			cajas_abiertas[y][x] = 0;
			cajas_colocadas++;
			total_cajas_en_mapa++;
		}
		intentos++;
	}

	max_cajas_abrir = (total_cajas_en_mapa + 1) / 2;

	daos_uart_puts("[MAPA] Total cajas: ");
	daos_uart_putint(total_cajas_en_mapa);
	daos_uart_puts(" - Max abrir: ");
	daos_uart_putint(max_cajas_abrir);
	daos_uart_puts("\r\n");
}

static void colocar_raices(void) {
	if (mapa[0][0] != TILE_CENTRO) mapa[0][0] = TILE_RAIZ;
	if (mapa[0][MAP_WIDTH-1] != TILE_CENTRO) mapa[0][MAP_WIDTH-1] = TILE_RAIZ;
	if (mapa[MAP_HEIGHT-1][0] != TILE_CENTRO) mapa[MAP_HEIGHT-1][0] = TILE_RAIZ;
	if (mapa[MAP_HEIGHT-1][MAP_WIDTH-1] != TILE_CENTRO) mapa[MAP_HEIGHT-1][MAP_WIDTH-1] = TILE_RAIZ;

	uint8_t mid_x = MAP_WIDTH / 2;
	uint8_t mid_y = MAP_HEIGHT / 2;

	if (mapa[0][mid_x] != TILE_CENTRO) mapa[0][mid_x] = TILE_RAIZ;
	if (mapa[MAP_HEIGHT-1][mid_x] != TILE_CENTRO) mapa[MAP_HEIGHT-1][mid_x] = TILE_RAIZ;
	if (mapa[mid_y][0] != TILE_CENTRO) mapa[mid_y][0] = TILE_RAIZ;
	if (mapa[mid_y][MAP_WIDTH-1] != TILE_CENTRO) mapa[mid_y][MAP_WIDTH-1] = TILE_RAIZ;
}

static void generar_mapa_laberinto(void) {
	// PROTECCIÓN: Generación del mapa con mutex
	mutex_lock(&reconocedor_map_mutex);

	for (uint8_t y = 0; y < MAP_HEIGHT; y++) {
		for (uint8_t x = 0; x < MAP_WIDTH; x++) {
			mapa[y][x] = TILE_PARED_AZUL;
			visitado[y][x] = 0;
			cajas_abiertas[y][x] = 0;
		}
	}

	generar_laberinto_recursivo(MAP_WIDTH/2, MAP_HEIGHT/2);

	for (uint8_t i = 0; i < 10; i++) {
		uint8_t x = random_range(0, MAP_WIDTH - 1);
		uint8_t y = random_range(0, MAP_HEIGHT - 1);
		if (mapa[y][x] != TILE_VACIO && mapa[y][x] != TILE_CENTRO) {
			mapa[y][x] = TILE_VACIO;
		}
	}

	crear_area_central();
	colocar_raices();
	colocar_cajas();

	for (uint8_t y = 0; y < MAP_HEIGHT; y++) {
		for (uint8_t x = 0; x < MAP_WIDTH; x++) {
			if (mapa[y][x] >= TILE_PARED_AZUL && mapa[y][x] <= TILE_PARED_AMARILLA) {
				uint8_t tipo = random_range(0, 2);
				if (tipo == 0) {
					mapa[y][x] = TILE_PARED_AZUL;
				} else if (tipo == 1) {
					mapa[y][x] = TILE_PARED_VERDE;
				} else {
					mapa[y][x] = TILE_PARED_AMARILLA;
				}
			}
		}
	}

	mutex_unlock(&reconocedor_map_mutex);

	// PROTECCIÓN: Inicialización del jugador con mutex
	mutex_lock(&reconocedor_player_mutex);
	escondedor_x = MAP_WIDTH / 2;
	escondedor_y = MAP_HEIGHT / 2;
	escondedor_dir = DIR_RIGHT;
	escondedor_oculto = 0;
	mutex_unlock(&reconocedor_player_mutex);
}

// ========================================================================
// FUNCIONES DE DIBUJO 2D
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

static void draw_tile(uint8_t x, uint8_t y, const uint32_t* sprite_data) {
	int16_t screen_x = (x * TILE_SIZE) + MAP_OFFSET_X;
	int16_t screen_y = (y * TILE_SIZE) + MAP_OFFSET_Y;

	for (uint8_t row = 0; row < TILE_SIZE; row++) {
		for (uint8_t col = 0; col < TILE_SIZE; col++) {
			uint8_t mapped_row = (row * TILE_DATA_SIZE) / TILE_SIZE;
			uint8_t mapped_col = (col * TILE_DATA_SIZE) / TILE_SIZE;

			if (mapped_row >= TILE_DATA_SIZE) mapped_row = TILE_DATA_SIZE - 1;
			if (mapped_col >= TILE_DATA_SIZE) mapped_col = TILE_DATA_SIZE - 1;

			uint32_t color_32bit = sprite_data[mapped_row * TILE_DATA_SIZE + mapped_col];

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

static void draw_tile_centro(uint8_t x, uint8_t y) {
	int16_t screen_x = (x * TILE_SIZE) + MAP_OFFSET_X;
	int16_t screen_y = (y * TILE_SIZE) + MAP_OFFSET_Y;
	daos_gfx_fill_rect(screen_x, screen_y, TILE_SIZE, TILE_SIZE, COLOR_BLACK);
	for (uint8_t row = 3; row < TILE_SIZE-3; row++) {
		for (uint8_t col = 3; col < TILE_SIZE-3; col++) {
			daos_gfx_draw_pixel(screen_x + col, screen_y + row, COLOR_CYAN);
		}
	}
}

static void draw_tile_raiz(uint8_t x, uint8_t y) {
	int16_t screen_x = (x * TILE_SIZE) + MAP_OFFSET_X;
	int16_t screen_y = (y * TILE_SIZE) + MAP_OFFSET_Y;
	daos_gfx_fill_rect(screen_x, screen_y, TILE_SIZE, TILE_SIZE, COLOR_BLACK);
	for (uint8_t row = 6; row < TILE_SIZE-6; row++) {
		for (uint8_t col = 6; col < TILE_SIZE-6; col++) {
			daos_gfx_draw_pixel(screen_x + col, screen_y + row, COLOR_RED);
		}
	}
}

static void redraw_tile_content(uint8_t x, uint8_t y) {
	int16_t screen_x = (x * TILE_SIZE) + MAP_OFFSET_X;
	int16_t screen_y = (y * TILE_SIZE) + MAP_OFFSET_Y;
	daos_gfx_fill_rect(screen_x, screen_y, TILE_SIZE, TILE_SIZE, COLOR_BLACK);

	// PROTECCIÓN: Lectura segura del mapa
	mutex_lock(&reconocedor_map_mutex);
	TipoTile tile = mapa[y][x];
	mutex_unlock(&reconocedor_map_mutex);

	switch(tile) {
		case TILE_PARED_AZUL:
			draw_tile(x, y, sprite_pared_azul);
			break;
		case TILE_PARED_VERDE:
			draw_tile(x, y, sprite_pared_verde);
			break;
		case TILE_PARED_AMARILLA:
			draw_tile(x, y, sprite_pared_amarilla);
			break;
		case TILE_CAJA:
			draw_tile(x, y, sprite_caja);
			break;
		case TILE_CENTRO:
			draw_tile_centro(x, y);
			break;
		case TILE_RAIZ:
			draw_tile_raiz(x, y);
			break;
		case TILE_VACIO:
		default:
			break;
	}
}

// ========================================================================
// LÓGICA DE MOVIMIENTO 2D
// ========================================================================
static uint8_t try_move_escondedor(Direccion new_dir) {
	// PROTECCIÓN: Verificar estado del juego
	mutex_lock(&reconocedor_game_state_mutex);
	FaseJuego current_fase = fase_actual;
	mutex_unlock(&reconocedor_game_state_mutex);

	// PROTECCIÓN: Leer y modificar estado del jugador
	mutex_lock(&reconocedor_player_mutex);

	if (escondedor_oculto || current_fase != FASE_2D_MOVIENDO) {
		mutex_unlock(&reconocedor_player_mutex);
		return 0;
	}

	uint8_t old_x = escondedor_x;
	uint8_t old_y = escondedor_y;
	uint8_t new_x = escondedor_x;
	uint8_t new_y = escondedor_y;

	if (new_dir != escondedor_dir) {
		escondedor_dir = new_dir;
		mutex_unlock(&reconocedor_player_mutex);
		return 0;
	}

	switch(new_dir) {
		case DIR_UP: new_y = (new_y > 0) ? new_y - 1 : new_y; break;
		case DIR_DOWN: new_y = (new_y < MAP_HEIGHT - 1) ? new_y + 1 : new_y; break;
		case DIR_LEFT: new_x = (new_x > 0) ? new_x - 1 : new_x; break;
		case DIR_RIGHT: new_x = (new_x < MAP_WIDTH - 1) ? new_x + 1 : new_x; break;
	}

	if (new_x == old_x && new_y == old_y) {
		mutex_unlock(&reconocedor_player_mutex);
		return 0;
	}

	// PROTECCIÓN: Lectura segura del mapa
	mutex_lock(&reconocedor_map_mutex);
	TipoTile target_tile = mapa[new_y][new_x];
	mutex_unlock(&reconocedor_map_mutex);

	if (target_tile == TILE_PARED_AZUL ||
		target_tile == TILE_PARED_VERDE ||
		target_tile == TILE_PARED_AMARILLA) {
		mutex_unlock(&reconocedor_player_mutex);
		return 0;
	}

	redraw_tile_content(old_x, old_y);
	escondedor_x = new_x;
	escondedor_y = new_y;

	if (escondedor_oculto && target_tile != TILE_CAJA) {
		escondedor_oculto = 0;
	}

	mutex_unlock(&reconocedor_player_mutex);
	return 1;
}

// NUEVA FUNCIÓN: Gestión unificada de maniquíes con capacidad de recolección
static void toggle_maniqui(Maniqui* m, uint8_t maniqui_id) {
	// PROTECCIÓN: Verificar si el jugador está oculto
	mutex_lock(&reconocedor_player_mutex);
	uint8_t is_hidden = escondedor_oculto;
	uint8_t player_x = escondedor_x;
	uint8_t player_y = escondedor_y;
	Direccion player_dir = escondedor_dir;
	mutex_unlock(&reconocedor_player_mutex);

	if (is_hidden) return;

	// PROTECCIÓN: Gestión segura de maniquíes
	mutex_lock(&reconocedor_maniqui_mutex);

	if (m->desplegado) {
		// RECOLECCIÓN: Si el maniquí está desplegado y estamos en su posición, lo recogemos
		if (player_x == m->x && player_y == m->y) {
			m->desplegado = 0;
			mutex_unlock(&reconocedor_maniqui_mutex);

			// Redibujar el tile para mostrar solo la caja
			redraw_tile_content(m->x, m->y);

			daos_uart_puts("[MANIQUI] Maniqui ");
			daos_uart_putint(maniqui_id);
			daos_uart_puts(" recogido de (");
			daos_uart_putint(m->x);
			daos_uart_puts(",");
			daos_uart_putint(m->y);
			daos_uart_puts(")\r\n");
			return;
		}
	} else {
		// DESPLIEGUE: Colocar el maniquí en la posición actual
		mutex_lock(&reconocedor_map_mutex);
		TipoTile current_tile = mapa[player_y][player_x];
		mutex_unlock(&reconocedor_map_mutex);

		// Solo se puede desplegar en cajas
		if (current_tile != TILE_CAJA) {
			mutex_unlock(&reconocedor_maniqui_mutex);
			return;
		}

		// Verificar que no haya otro maniquí en esta posición
		Maniqui* other_m = (maniqui_id == 1) ? &maniqui_2 : &maniqui_1;
		if (other_m->desplegado && player_x == other_m->x && player_y == other_m->y) {
			mutex_unlock(&reconocedor_maniqui_mutex);
			return;
		}

		// Desplegar el maniquí
		m->x = player_x;
		m->y = player_y;
		m->dir = player_dir;
		m->desplegado = 1;

		daos_uart_puts("[MANIQUI] Maniqui ");
		daos_uart_putint(maniqui_id);
		daos_uart_puts(" desplegado en (");
		daos_uart_putint(m->x);
		daos_uart_puts(",");
		daos_uart_putint(m->y);
		daos_uart_puts(")\r\n");
	}

	mutex_unlock(&reconocedor_maniqui_mutex);
}

// ========================================================================
// MOTOR 3D - RAYCASTING
// ========================================================================
static void init_ray_table_3d(void) {
	for (int angle_idx = 0; angle_idx < 4; angle_idx++) {
		for (int ray = 0; ray < NUM_RAYS; ray++) {
			int offset = ((ray - NUM_RAYS/2) * 70) / NUM_RAYS;
			int total_angle = (angle_idx * 90 + offset);

			while (total_angle < 0) total_angle += 360;
			while (total_angle >= 360) total_angle -= 360;

			if (total_angle < 45 || total_angle >= 315) {
				ray_table[angle_idx][ray].dx = (total_angle < 45) ?
					(total_angle * 100 / 45) : ((total_angle - 360) * 100 / 45);
				ray_table[angle_idx][ray].dy = -100;
			} else if (total_angle >= 45 && total_angle < 135) {
				ray_table[angle_idx][ray].dx = 100;
				ray_table[angle_idx][ray].dy = ((total_angle - 90) * 100 / 45);
			} else if (total_angle >= 135 && total_angle < 225) {
				ray_table[angle_idx][ray].dx = ((180 - total_angle) * 100 / 45);
				ray_table[angle_idx][ray].dy = 100;
			} else {
				ray_table[angle_idx][ray].dx = -100;
				ray_table[angle_idx][ray].dy = ((270 - total_angle) * 100 / 45);
			}
		}
	}
}

static RayHit cast_ray_optimized(int grid_x, int grid_y, const RayDir* ray_dir) {
	RayHit hit = {0, 0, 0, 0};

	int map_x = grid_x;
	int map_y = grid_y;

	int step_x = (ray_dir->dx > 0) ? 1 : -1;
	int step_y = (ray_dir->dy > 0) ? 1 : -1;

	int abs_dx = (ray_dir->dx > 0) ? ray_dir->dx : -ray_dir->dx;
	int abs_dy = (ray_dir->dy > 0) ? ray_dir->dy : -ray_dir->dy;

	if (abs_dx < 1) abs_dx = 1;
	if (abs_dy < 1) abs_dy = 1;

	int delta_x = (TILE_SIZE_3D << 6) / abs_dx;
	int delta_y = (TILE_SIZE_3D << 6) / abs_dy;

	int side_dist_x = delta_x >> 1;
	int side_dist_y = delta_y >> 1;

	uint8_t hit_wall = 0;
	uint8_t side = 0;

	// PROTECCIÓN: Lectura segura del mapa durante raycasting
	for (int depth = 0; depth < 20 && !hit_wall; depth++) {
		if (side_dist_x < side_dist_y) {
			side_dist_x += delta_x;
			map_x += step_x;
			side = 1;
		} else {
			side_dist_y += delta_y;
			map_y += step_y;
			side = 0;
		}

		if ((uint8_t)map_x >= MAP_WIDTH || (uint8_t)map_y >= MAP_HEIGHT) {
			hit_wall = 1;
			hit.wall_type = 1;
			hit.side = side;
			break;
		}

		mutex_lock(&reconocedor_map_mutex);
		TipoTile tile = mapa[map_y][map_x];
		uint8_t caja_abierta = cajas_abiertas[map_y][map_x];
		mutex_unlock(&reconocedor_map_mutex);

		if (tile == TILE_PARED_AZUL || tile == TILE_PARED_VERDE || tile == TILE_PARED_AMARILLA) {
			hit_wall = 1;
			hit.wall_type = (uint8_t)tile;
			hit.side = side;
		} else if (tile == TILE_CAJA && caja_abierta == 0) {
			hit_wall = 1;
			hit.wall_type = (uint8_t)tile;
			hit.side = side;
		}
	}

	hit.distance = (side == 1) ? (side_dist_x - delta_x) : (side_dist_y - delta_y);
	if (hit.distance < 32) hit.distance = 32;

	if (side == 1) {
		hit.tex_x = ((map_y * TILE_SIZE_3D) % TILE_SIZE_3D) * 10 / TILE_SIZE_3D;
	} else {
		hit.tex_x = ((map_x * TILE_SIZE_3D) % TILE_SIZE_3D) * 10 / TILE_SIZE_3D;
	}
	if (hit.tex_x >= 10) hit.tex_x = 9;

	return hit;
}

static uint16_t get_wall_color_fast_3d(uint8_t wall_type, uint8_t side, uint8_t tex_x, uint8_t tex_y) {
	uint16_t base_color;

	if (wall_type == TILE_CAJA) {
		if (tex_x >= 10) tex_x = 9;
		if (tex_y >= 10) tex_y = 9;
		base_color = textura_caja_3d[tex_y * 10 + tex_x];

		if (side == 1) {
			int r = ((base_color >> 11) & 0x1F) * 3 / 4;
			int g = ((base_color >> 5) & 0x3F) * 3 / 4;
			int b = (base_color & 0x1F) * 3 / 4;
			return (r << 11) | (g << 5) | b;
		}
		return base_color;
	}

	switch(wall_type) {
		case TILE_PARED_AZUL: base_color = COLOR_WALL_BLUE; break;
		case TILE_PARED_VERDE: base_color = COLOR_WALL_GREEN; break;
		case TILE_PARED_AMARILLA: base_color = COLOR_WALL_YELLOW; break;
		default: base_color = COLOR_WALL_BLUE; break;
	}

	if (side == 1) {
		int r = ((base_color >> 11) & 0x1F) * 3 / 4;
		int g = ((base_color >> 5) & 0x3F) * 3 / 4;
		int b = (base_color & 0x1F) * 3 / 4;
		return (r << 11) | (g << 5) | b;
	}

	return base_color;
}

static void render_3d_view(void) {
	uint32_t start_time = daos_millis();

	// PROTECCIÓN: Lectura segura de la posición del jugador
	mutex_lock(&reconocedor_player_mutex);
	uint8_t current_x = buscador_x;
	uint8_t current_y = buscador_y;
	int current_angle = buscador_angle;
	mutex_unlock(&reconocedor_player_mutex);

	if (current_x == last_rendered_x_3d &&
		current_y == last_rendered_y_3d &&
		current_angle == last_rendered_angle_3d) {
		return;
	}

	daos_gfx_fill_rect(0, 0, GAME_AREA_WIDTH, GAME_AREA_HEIGHT >> 1, COLOR_SKY);
	daos_gfx_fill_rect(0, GAME_AREA_HEIGHT >> 1, GAME_AREA_WIDTH, GAME_AREA_HEIGHT >> 1, COLOR_FLOOR);

	int angle_idx = current_angle / 90;
	int column_width = GAME_AREA_WIDTH / NUM_RAYS;
	if (column_width < 1) column_width = 1;

	RayHit last_hit = {0, 0, 0, 0};

	for (int ray = 0; ray < NUM_RAYS; ray++) {
		RayHit hit = cast_ray_optimized(current_x, current_y, &ray_table[angle_idx][ray]);

		int wall_height = ((GAME_AREA_HEIGHT << 7) / (hit.distance + 1));
		if (wall_height > (GAME_AREA_HEIGHT << 1)) {
			wall_height = GAME_AREA_HEIGHT << 1;
		}

		int draw_start = (GAME_AREA_HEIGHT - wall_height) >> 1;
		int draw_end = draw_start + wall_height;

		if (draw_start < 0) draw_start = 0;
		if (draw_end >= GAME_AREA_HEIGHT) draw_end = GAME_AREA_HEIGHT - 1;

		int x_start = ray * column_width;

		if (hit.wall_type == TILE_CAJA) {
			for (int y = draw_start; y < draw_end; y++) {
				uint8_t tex_y = ((y - draw_start) * 10) / wall_height;
				if (tex_y >= 10) tex_y = 9;

				uint16_t color = get_wall_color_fast_3d(hit.wall_type, hit.side, hit.tex_x, tex_y);
				daos_gfx_fill_rect(x_start, y, column_width, 1, color);
			}

			if (ray > 0 && last_hit.wall_type == TILE_CAJA &&
				(hit.distance != last_hit.distance || hit.side != last_hit.side)) {
				daos_gfx_fill_rect(x_start, draw_start, 1, draw_end - draw_start, COLOR_BLACK);
			}

		} else {
			uint16_t color = get_wall_color_fast_3d(hit.wall_type, hit.side, 0, 0);
			daos_gfx_fill_rect(x_start, draw_start, column_width, draw_end - draw_start, color);
		}

		last_hit = hit;
	}

	last_rendered_x_3d = current_x;
	last_rendered_y_3d = current_y;
	last_rendered_angle_3d = current_angle;
}

static void draw_minimap_3d(void) {
	int map_x = 95;
	int map_y = HUD_Y + 10;

	int cell_w = MINIMAP_SIZE / MAP_WIDTH;
	int cell_h = MINIMAP_SIZE / MAP_HEIGHT;
	int cell_size = (cell_w < cell_h) ? cell_w : cell_h;

	int actual_width = MAP_WIDTH * cell_size;
	int actual_height = MAP_HEIGHT * cell_size;

	static uint8_t last_player_x = 255;
	static uint8_t last_player_y = 255;
	static int last_player_angle = -1;

	// PROTECCIÓN: Lectura segura de posición del jugador
	mutex_lock(&reconocedor_player_mutex);
	uint8_t current_x = buscador_x;
	uint8_t current_y = buscador_y;
	int current_angle = buscador_angle;
	mutex_unlock(&reconocedor_player_mutex);

	if (last_player_x == current_x &&
		last_player_y == current_y &&
		last_player_angle == current_angle &&
		!minimap_needs_redraw) {
		return;
	}

	daos_gfx_fill_rect(map_x - 1, map_y - 1,
					  actual_width + 2, actual_height + 2, COLOR_CYAN);

	daos_gfx_fill_rect(map_x, map_y,
					  actual_width, actual_height, COLOR_MINIMAP_BG);

	// PROTECCIÓN: Lectura segura del mapa para el minimapa
	mutex_lock(&reconocedor_map_mutex);
	for (int y = 0; y < MAP_HEIGHT; y++) {
		for (int x = 0; x < MAP_WIDTH; x++) {
			TipoTile tile = mapa[y][x];
			uint16_t color = COLOR_MINIMAP_BG;

			if (tile >= TILE_PARED_AZUL && tile <= TILE_PARED_AMARILLA) {
				switch(tile) {
					case TILE_PARED_VERDE: color = COLOR_WALL_GREEN; break;
					case TILE_PARED_AMARILLA: color = COLOR_WALL_YELLOW; break;
					default: color = COLOR_WALL_BLUE; break;
				}
			}

			int px = map_x + x * cell_size;
			int py = map_y + y * cell_size;
			daos_gfx_fill_rect(px, py, cell_size, cell_size, color);
		}
	}
	mutex_unlock(&reconocedor_map_mutex);

	int player_px = map_x + current_x * cell_size + (cell_size >> 1);
	int player_py = map_y + current_y * cell_size + (cell_size >> 1);

	daos_gfx_fill_rect(player_px - 2, player_py - 2, 5, 5, COLOR_PLAYER);

	int dir_idx = current_angle / 90;
	int arrow_len = cell_size / 2 + 1;
	int end_x = player_px + dir_x_3d[dir_idx] * arrow_len;
	int end_y = player_py + dir_y_3d[dir_idx] * arrow_len;
	daos_gfx_fill_rect(end_x - 1, end_y - 1, 3, 3, COLOR_YELLOW_BG);

	last_player_x = current_x;
	last_player_y = current_y;
	last_player_angle = current_angle;
	minimap_needs_redraw = 0;
}

// ========================================================================
// FUNCION draw_hud_3d ORIGINAL
// ========================================================================
static void draw_hud_3d(void) {
	if (!hud_initialized) {
		daos_gfx_fill_rect(0, HUD_Y, 320, HUD_HEIGHT, COLOR_HUD_BG);
		daos_gfx_fill_rect(0, HUD_Y, 320, 2, COLOR_CYAN);

		daos_gfx_draw_text(5, HUD_Y + 5, "BUSCAR 3D", COLOR_CYAN, COLOR_HUD_BG);
		daos_gfx_draw_text(5, HUD_Y + 15, "[D6:ABRIR]", COLOR_WALL_GREEN, COLOR_HUD_BG);

		hud_initialized = 1;
		minimap_needs_redraw = 1;
	}

	draw_minimap_3d();

	int x = 180;
	int y = HUD_Y + 5;
	char buf[32];

	// Direcciones cardinales (NORTE, ESTE, SUR, OESTE)
	const char* dirs[] = {"NORTE", "ESTE ", "SUR  ", "OESTE"};
	const uint16_t dir_colors[] = {COLOR_CYAN, COLOR_WALL_GREEN,
								   COLOR_WALL_ORANGE, COLOR_WALL_PURPLE};

	// PROTECCIÓN: Lectura segura del ángulo
	mutex_lock(&reconocedor_player_mutex);
	int current_angle = buscador_angle;
	uint8_t current_x = buscador_x;
	uint8_t current_y = buscador_y;
	mutex_unlock(&reconocedor_player_mutex);

	int dir_idx = current_angle / 90;

	// DIBUJAR DIRECCIÓN CARDINAL
	daos_gfx_draw_text(x, y, dirs[dir_idx], dir_colors[dir_idx], COLOR_HUD_BG);

	y += 12;
	buf[0] = 'P'; buf[1] = 'O'; buf[2] = 'S'; buf[3] = ':';
	buf[4] = '0' + ((current_x / 10) % 10);
	buf[5] = '0' + (current_x % 10);
	buf[6] = ',';
	buf[7] = '0' + ((current_y / 10) % 10);
	buf[8] = '0' + (current_y % 10);
	buf[9] = '\0';
	daos_gfx_draw_text(x, y, buf, COLOR_WHITE, COLOR_HUD_BG);

	// PROTECCIÓN: Lectura segura de contadores
	mutex_lock(&reconocedor_map_mutex);
	uint8_t local_cajas_abiertas = cajas_abiertas_count;
	uint8_t local_max_cajas = max_cajas_abrir;
	uint8_t local_maniquies = maniquies_encontrados;
	mutex_unlock(&reconocedor_map_mutex);

	y += 12;
	buf[0] = 'C'; buf[1] = 'A'; buf[2] = 'J'; buf[3] = ':';
	buf[4] = '0' + ((local_cajas_abiertas / 10) % 10);
	buf[5] = '0' + (local_cajas_abiertas % 10);
	buf[6] = '/';
	buf[7] = '0' + ((local_max_cajas / 10) % 10);
	buf[8] = '0' + (local_max_cajas % 10);
	buf[9] = '\0';
	daos_gfx_draw_text(x, y, buf, COLOR_WALL_ORANGE, COLOR_HUD_BG);

	y += 12;
	buf[0] = 'M'; buf[1] = 'A'; buf[2] = 'N'; buf[3] = ':';
	buf[4] = '0' + local_maniquies;
	buf[5] = '/'; buf[6] = '2';
	buf[7] = '\0';
	daos_gfx_draw_text(x, y, buf, COLOR_YELLOW_BG, COLOR_HUD_BG);

	// El espacio donde antes iba FPS/MS queda vacío o se puede usar para el contador de movimientos/giros
	x = 250;
	y = HUD_Y + 5;

	daos_gfx_draw_text(x, y, "MOV:", COLOR_GRAY, COLOR_HUD_BG);
	buf[0] = '0' + ((move_count_3d / 100) % 10);
	buf[1] = '0' + ((move_count_3d / 10) % 10);
	buf[2] = '0' + (move_count_3d % 10);
	buf[3] = '\0';
	daos_gfx_draw_text(x, y + 12, buf, COLOR_WHITE, COLOR_HUD_BG);

	daos_gfx_draw_text(x, y + 24, "GIR:", COLOR_GRAY, COLOR_HUD_BG);
	buf[0] = '0' + ((turn_count_3d / 100) % 10);
	buf[1] = '0' + ((turn_count_3d / 10) % 10);
	buf[2] = '0' + (turn_count_3d % 10);
	buf[3] = '\0';
	daos_gfx_draw_text(x, y + 36, buf, COLOR_WHITE, COLOR_HUD_BG);

	uint16_t pulse = (debug_update_counter % 8 < 4) ? COLOR_WALL_GREEN : COLOR_HUD_BG;
	daos_gfx_fill_rect(310, HUD_Y + 5, 8, 8, pulse);

	debug_update_counter++;
}

// ========================================================================
// RENDERIZADO POR FASE
// ========================================================================
static void fase_2d_render(void) {
	if (first_draw) {
		daos_gfx_clear(COLOR_BLACK);

		// PROTECCIÓN: Lectura segura del mapa para renderizado inicial
		for (uint8_t y = 0; y < MAP_HEIGHT; y++) {
			for (uint8_t x = 0; x < MAP_WIDTH; x++) {
				mutex_lock(&reconocedor_map_mutex);
				TipoTile tile = mapa[y][x];
				mutex_unlock(&reconocedor_map_mutex);

				int16_t screen_x = (x * TILE_SIZE) + MAP_OFFSET_X;
				int16_t screen_y = (y * TILE_SIZE) + MAP_OFFSET_Y;
				daos_gfx_fill_rect(screen_x, screen_y, TILE_SIZE, TILE_SIZE, COLOR_BLACK);

				switch(tile) {
					case TILE_PARED_AZUL:
						draw_tile(x, y, sprite_pared_azul);
						break;
					case TILE_PARED_VERDE:
						draw_tile(x, y, sprite_pared_verde);
						break;
					case TILE_PARED_AMARILLA:
						draw_tile(x, y, sprite_pared_amarilla);
						break;
					case TILE_CAJA:
						draw_tile(x, y, sprite_caja);
						break;
					case TILE_CENTRO:
						draw_tile_centro(x, y);
						break;
					case TILE_RAIZ:
						draw_tile_raiz(x, y);
						break;
					case TILE_VACIO:
					default:
						break;
				}
			}
		}

		first_draw = 0;
	}

	// PROTECCIÓN: Renderizado seguro de maniquíes
	mutex_lock(&reconocedor_maniqui_mutex);
	if (maniqui_1.desplegado) {
		redraw_tile_content(maniqui_1.x, maniqui_1.y);
		draw_tile_rotated(maniqui_1.x, maniqui_1.y, sprite_maniqui_1, maniqui_1.dir);
	}
	if (maniqui_2.desplegado) {
		redraw_tile_content(maniqui_2.x, maniqui_2.y);
		draw_tile_rotated(maniqui_2.x, maniqui_2.y, sprite_maniqui_2, maniqui_2.dir);
	}
	mutex_unlock(&reconocedor_maniqui_mutex);

	// PROTECCIÓN: Renderizado seguro del jugador
	mutex_lock(&reconocedor_player_mutex);
	uint8_t player_x = escondedor_x;
	uint8_t player_y = escondedor_y;
	Direccion player_dir = escondedor_dir;
	uint8_t is_hidden = escondedor_oculto;
	mutex_unlock(&reconocedor_player_mutex);

	if (is_hidden) {
		redraw_tile_content(player_x, player_y);
	} else {
		draw_tile_rotated(player_x, player_y, sprite_jugador, player_dir);
	}

	// PROTECCIÓN: Verificar fase actual
	mutex_lock(&reconocedor_game_state_mutex);
	FaseJuego current_fase = fase_actual;
	mutex_unlock(&reconocedor_game_state_mutex);

	if (current_fase == FASE_2D_MOVIENDO) {
		int16_t map_end_x = MAP_OFFSET_X + (MAP_WIDTH * TILE_SIZE) - 1;
		int16_t map_end_y = MAP_OFFSET_Y + (MAP_HEIGHT * TILE_SIZE) - 1;
		draw_horizontal_line(MAP_OFFSET_X, MAP_OFFSET_Y, map_end_x, COLOR_WHITE);
		draw_horizontal_line(MAP_OFFSET_X, map_end_y, map_end_x, COLOR_WHITE);
		draw_vertical_line(MAP_OFFSET_X, MAP_OFFSET_Y, map_end_y, COLOR_WHITE);
		draw_vertical_line(map_end_x, MAP_OFFSET_Y, map_end_y, COLOR_WHITE);
	}
}

static void fase_3d_render(void) {
	if (first_draw) {
		daos_gfx_clear(COLOR_BLACK);
		first_draw = 0;
		minimap_needs_redraw = 1;
	}

	render_3d_view();
	draw_hud_3d();
}

static void intentar_abrir_caja(void) {
	// PROTECCIÓN: Lectura segura de posición y ángulo del jugador
	mutex_lock(&reconocedor_player_mutex);
	uint8_t player_x = buscador_x;
	uint8_t player_y = buscador_y;
	int player_angle = buscador_angle;
	uint8_t escondedor_local_x = escondedor_x;
	uint8_t escondedor_local_y = escondedor_y;
	uint8_t is_hidden = escondedor_oculto;
	mutex_unlock(&reconocedor_player_mutex);

	int dir_idx = player_angle / 90;
	int target_x = player_x + dir_x_3d[dir_idx];
	int target_y = player_y + dir_y_3d[dir_idx];

	if (target_x < 0 || target_x >= MAP_WIDTH || target_y < 0 || target_y >= MAP_HEIGHT) {
		return;
	}

	// PROTECCIÓN: Verificar y modificar estado de caja
	mutex_lock(&reconocedor_map_mutex);
	TipoTile tile_tipo = mapa[target_y][target_x];
	uint8_t ya_abierta = cajas_abiertas[target_y][target_x];

	if (tile_tipo != TILE_CAJA || ya_abierta == 1) {
		mutex_unlock(&reconocedor_map_mutex);
		return;
	}

	cajas_abiertas[target_y][target_x] = 1;
	cajas_abiertas_count++;

	daos_uart_puts("[JUEGO] Caja abierta en (");
	daos_uart_putint(target_x);
	daos_uart_puts(",");
	daos_uart_putint(target_y);
	daos_uart_puts(") - Total: ");
	daos_uart_putint(cajas_abiertas_count);
	daos_uart_puts("/");
	daos_uart_putint(max_cajas_abrir);
	daos_uart_puts("\r\n");

	// Verificar si encontró al escondedor
	if (target_x == escondedor_local_x && target_y == escondedor_local_y && is_hidden) {
		mutex_unlock(&reconocedor_map_mutex);

		daos_uart_puts("[JUEGO] VICTORIA! Encontro al escondedor!\r\n");

		mutex_lock(&reconocedor_game_state_mutex);
		fase_actual = FASE_VICTORIA;
		mutex_unlock(&reconocedor_game_state_mutex);

		first_draw = 1;
		return;
	}

	// PROTECCIÓN: Verificar si encontró un maniquí
	mutex_lock(&reconocedor_maniqui_mutex);
	uint8_t encontro_maniqui = 0;

	if (maniqui_1.desplegado && maniqui_1.x == target_x && maniqui_1.y == target_y) {
		maniquies_encontrados++;
		encontro_maniqui = 1;
		daos_uart_puts("[JUEGO] Maniqui 1 encontrado! (");
		daos_uart_putint(maniquies_encontrados);
		daos_uart_puts("/2)\r\n");
	}

	if (maniqui_2.desplegado && maniqui_2.x == target_x && maniqui_2.y == target_y) {
		maniquies_encontrados++;
		encontro_maniqui = 1;
		daos_uart_puts("[JUEGO] Maniqui 2 encontrado! (");
		daos_uart_putint(maniquies_encontrados);
		daos_uart_puts("/2)\r\n");
	}

	mutex_unlock(&reconocedor_maniqui_mutex);

	if (encontro_maniqui && maniquies_encontrados >= 2) {
		mutex_unlock(&reconocedor_map_mutex);

		daos_uart_puts("[JUEGO] DERROTA! Encontraste 2 maniquies!\r\n");

		mutex_lock(&reconocedor_game_state_mutex);
		fase_actual = FASE_DERROTA;
		mutex_unlock(&reconocedor_game_state_mutex);

		first_draw = 1;
		return;
	}

	if (encontro_maniqui) {
		mutex_unlock(&reconocedor_map_mutex);
		return;
	}

	// Caja vacía - desaparece del mapa
	mapa[target_y][target_x] = TILE_VACIO;
	mutex_unlock(&reconocedor_map_mutex);

	last_rendered_x_3d = -1;
	last_rendered_y_3d = -1;
	last_rendered_angle_3d = -1;
	minimap_needs_redraw = 1;

	daos_uart_puts("[JUEGO] Caja vacia - desaparecio del mapa\r\n");

	// Verificar límite de cajas
	mutex_lock(&reconocedor_map_mutex);
	if (cajas_abiertas_count >= max_cajas_abrir) {
		mutex_unlock(&reconocedor_map_mutex);

		daos_uart_puts("[JUEGO] DERROTA! Alcanzaste el limite de cajas (");
		daos_uart_putint(cajas_abiertas_count);
		daos_uart_puts("/");
		daos_uart_putint(max_cajas_abrir);
		daos_uart_puts(") sin encontrar al escondedor!\r\n");

		mutex_lock(&reconocedor_game_state_mutex);
		fase_actual = FASE_DERROTA;
		mutex_unlock(&reconocedor_game_state_mutex);

		first_draw = 1;
		return;
	}
	mutex_unlock(&reconocedor_map_mutex);
}

// ========================================================================
// TAREAS DEL JUEGO
// ========================================================================
void reconocedor_input_task(void) {
	if (!game_running) {
		daos_sleep_ms(100);
		return;
	}

	uint8_t d2 = daos_btn_read_by_index(0);
	uint8_t d3 = daos_btn_read_by_index(1);
	uint8_t d4 = daos_btn_read_by_index(2);
	uint8_t d5 = daos_btn_read_by_index(3);
	uint8_t d6 = daos_btn_read_by_index(4);
	uint8_t d7 = daos_btn_read_by_index(5);
	uint8_t d8 = daos_btn_read_by_index(6);
	uint8_t d9 = daos_btn_read_by_index(7);

	static uint8_t last_d6 = 0, last_d7 = 0, last_d8 = 0, last_d9 = 0;
	static uint32_t last_move_time = 0;

	// PROTECCIÓN: Lectura segura de la fase actual
	mutex_lock(&reconocedor_game_state_mutex);
	FaseJuego current_fase = fase_actual;
	mutex_unlock(&reconocedor_game_state_mutex);

	if (current_fase == FASE_2D_MOVIENDO) {
		// D6: Toggle Maniquí 1 (colocar/recoger)
		if (d6 && !last_d6) {
			mutex_lock(&reconocedor_player_mutex);
			uint8_t px = escondedor_x;
			uint8_t py = escondedor_y;
			mutex_unlock(&reconocedor_player_mutex);

			mutex_lock(&reconocedor_map_mutex);
			TipoTile current_tile = mapa[py][px];
			mutex_unlock(&reconocedor_map_mutex);

			// Solo operar si estamos en una caja o si vamos a recoger
			mutex_lock(&reconocedor_maniqui_mutex);
			uint8_t esta_desplegado = maniqui_1.desplegado;
			uint8_t en_posicion = (maniqui_1.x == px && maniqui_1.y == py);
			mutex_unlock(&reconocedor_maniqui_mutex);

			if (current_tile == TILE_CAJA || (esta_desplegado && en_posicion)) {
				toggle_maniqui(&maniqui_1, 1);
			}
		}

		// D7: Toggle Maniquí 2 (colocar/recoger)
		if (d7 && !last_d7) {
			mutex_lock(&reconocedor_player_mutex);
			uint8_t px = escondedor_x;
			uint8_t py = escondedor_y;
			mutex_unlock(&reconocedor_player_mutex);

			mutex_lock(&reconocedor_map_mutex);
			TipoTile current_tile = mapa[py][px];
			mutex_unlock(&reconocedor_map_mutex);

			// Solo operar si estamos en una caja o si vamos a recoger
			mutex_lock(&reconocedor_maniqui_mutex);
			uint8_t esta_desplegado = maniqui_2.desplegado;
			uint8_t en_posicion = (maniqui_2.x == px && maniqui_2.y == py);
			mutex_unlock(&reconocedor_maniqui_mutex);

			if (current_tile == TILE_CAJA || (esta_desplegado && en_posicion)) {
				toggle_maniqui(&maniqui_2, 2);
			}
		}

		// D8: Confirmar ocultamiento
		if (d8 && !last_d8) {
			mutex_lock(&reconocedor_player_mutex);
			uint8_t px = escondedor_x;
			uint8_t py = escondedor_y;
			mutex_unlock(&reconocedor_player_mutex);

			mutex_lock(&reconocedor_map_mutex);
			TipoTile current_tile = mapa[py][px];
			mutex_unlock(&reconocedor_map_mutex);

			if (current_tile == TILE_CAJA && !is_maniqui_at(px, py)) {
				mutex_lock(&reconocedor_game_state_mutex);
				fase_actual = FASE_2D_CONFIRMANDO;
				mutex_unlock(&reconocedor_game_state_mutex);
				first_draw = 1;
			}
		}

		// Movimiento direccional
		if (d2) try_move_escondedor(DIR_RIGHT);
		else if (d3) try_move_escondedor(DIR_DOWN);
		else if (d4) try_move_escondedor(DIR_UP);
		else if (d5) try_move_escondedor(DIR_LEFT);
	}
	else if (current_fase == FASE_2D_CONFIRMANDO) {
		if (d9 && !last_d9) {
			mutex_lock(&reconocedor_player_mutex);
			uint8_t px = escondedor_x;
			uint8_t py = escondedor_y;
			mutex_unlock(&reconocedor_player_mutex);

			mutex_lock(&reconocedor_map_mutex);
			TipoTile current_tile = mapa[py][px];
			mutex_unlock(&reconocedor_map_mutex);

			if (current_tile == TILE_CAJA && !is_maniqui_at(px, py)) {
				mutex_lock(&reconocedor_player_mutex);
				escondedor_oculto = 1;
				mutex_unlock(&reconocedor_player_mutex);

				mutex_lock(&reconocedor_game_state_mutex);
				fase_actual = FASE_2D_ESCONDIDO;
				mutex_unlock(&reconocedor_game_state_mutex);

				first_draw = 1;
			} else {
				mutex_lock(&reconocedor_game_state_mutex);
				fase_actual = FASE_2D_MOVIENDO;
				mutex_unlock(&reconocedor_game_state_mutex);
				first_draw = 1;
			}
		}
		else if (d8 && !last_d8) {
			mutex_lock(&reconocedor_game_state_mutex);
			fase_actual = FASE_2D_MOVIENDO;
			mutex_unlock(&reconocedor_game_state_mutex);
			first_draw = 1;
		}
	}
	else if (current_fase == FASE_2D_ESCONDIDO) {
		if (d9 && !last_d9) {
			// PROTECCIÓN: Inicialización segura del buscador
			mutex_lock(&reconocedor_player_mutex);
			buscador_x = MAP_WIDTH / 2;
			buscador_y = MAP_HEIGHT / 2;
			buscador_angle = 0;
			mutex_unlock(&reconocedor_player_mutex);

			move_count_3d = 0;
			turn_count_3d = 0;
			minimap_needs_redraw = 1;
			hud_initialized = 0;

			mutex_lock(&reconocedor_map_mutex);
			cajas_abiertas_count = 0;
			maniquies_encontrados = 0;
			mutex_unlock(&reconocedor_map_mutex);

			mutex_lock(&reconocedor_game_state_mutex);
			fase_actual = FASE_3D_BUSCANDO;
			mutex_unlock(&reconocedor_game_state_mutex);

			first_draw = 1;
			last_rendered_x_3d = -1;
			last_rendered_y_3d = -1;
			last_rendered_angle_3d = -1;

			daos_uart_puts("[3D] Fase de busqueda iniciada\r\n");
		}
	}
	else if (current_fase == FASE_3D_BUSCANDO) {
		uint32_t current_time = daos_millis();

		// D6: Abrir caja
		if (d6 && !last_d6) {
			intentar_abrir_caja();
		}

		// D2: Rotar derecha
		if (d2 && current_time - last_move_time >= MOVE_COOLDOWN_MS) {
			mutex_lock(&reconocedor_player_mutex);
			buscador_angle = (buscador_angle + 90) % 360;
			mutex_unlock(&reconocedor_player_mutex);

			turn_count_3d++;
			last_move_time = current_time;
		}
		// D5: Rotar izquierda
		else if (d5 && current_time - last_move_time >= MOVE_COOLDOWN_MS) {
			mutex_lock(&reconocedor_player_mutex);
			buscador_angle = (buscador_angle - 90 + 360) % 360;
			mutex_unlock(&reconocedor_player_mutex);

			turn_count_3d++;
			last_move_time = current_time;
		}
		// D3: Retroceder
		else if (d3 && current_time - last_move_time >= MOVE_COOLDOWN_MS) {
			mutex_lock(&reconocedor_player_mutex);
			int direction_idx = buscador_angle / 90;
			int new_x = buscador_x - dir_x_3d[direction_idx];
			int new_y = buscador_y - dir_y_3d[direction_idx];
			mutex_unlock(&reconocedor_player_mutex);

			if (new_x >= 0 && new_x < MAP_WIDTH && new_y >= 0 && new_y < MAP_HEIGHT) {
				mutex_lock(&reconocedor_map_mutex);
				TipoTile tile = mapa[new_y][new_x];
				mutex_unlock(&reconocedor_map_mutex);

				uint8_t can_walk = (tile != TILE_PARED_AZUL && tile != TILE_PARED_VERDE &&
								   tile != TILE_PARED_AMARILLA);

				if (can_walk) {
					mutex_lock(&reconocedor_player_mutex);
					buscador_x = new_x;
					buscador_y = new_y;
					mutex_unlock(&reconocedor_player_mutex);

					move_count_3d++;
					last_move_time = current_time;
				}
			}
		}
		// D4: Avanzar
		else if (d4 && current_time - last_move_time >= MOVE_COOLDOWN_MS) {
			mutex_lock(&reconocedor_player_mutex);
			int direction_idx = buscador_angle / 90;
			int new_x = buscador_x + dir_x_3d[direction_idx];
			int new_y = buscador_y + dir_y_3d[direction_idx];
			mutex_unlock(&reconocedor_player_mutex);

			if (new_x >= 0 && new_x < MAP_WIDTH && new_y >= 0 && new_y < MAP_HEIGHT) {
				mutex_lock(&reconocedor_map_mutex);
				TipoTile tile = mapa[new_y][new_x];
				mutex_unlock(&reconocedor_map_mutex);

				uint8_t can_walk = (tile != TILE_PARED_AZUL && tile != TILE_PARED_VERDE &&
								   tile != TILE_PARED_AMARILLA);

				if (can_walk) {
					mutex_lock(&reconocedor_player_mutex);
					buscador_x = new_x;
					buscador_y = new_y;
					mutex_unlock(&reconocedor_player_mutex);

					move_count_3d++;
					last_move_time = current_time;
				}
			}
		}
	}

	last_d6 = d6;
	last_d7 = d7;
	last_d8 = d8;
	last_d9 = d9;

	daos_sleep_ms(50);
}

void reconocedor_logic_task(void) {
	if (!game_running) {
		daos_sleep_ms(100);
		return;
	}

	// Esta tarea puede ser usada para lógica adicional en el futuro
	daos_sleep_ms(50);
}

void reconocedor_render_task(void) {
	if (!game_running) {
		daos_sleep_ms(50);
		return;
	}

	// PROTECCIÓN: Lectura segura de la fase actual
	mutex_lock(&reconocedor_game_state_mutex);
	FaseJuego current_fase = fase_actual;
	mutex_unlock(&reconocedor_game_state_mutex);

	if (current_fase == FASE_2D_CONFIRMANDO || current_fase == FASE_2D_ESCONDIDO) {
		daos_gfx_clear(COLOR_BLACK);
		first_draw = 0;
	} else if (current_fase == FASE_2D_MOVIENDO) {
		fase_2d_render();
	} else if (current_fase == FASE_3D_BUSCANDO) {
		fase_3d_render();
	}

	if (current_fase == FASE_2D_CONFIRMANDO) {
		daos_gfx_fill_rect(60, 90, 200, 60, COLOR_BLACK);
		daos_gfx_fill_rect(58, 88, 204, 64, COLOR_WHITE);
		daos_gfx_fill_rect(60, 90, 200, 60, COLOR_BLACK);
		daos_gfx_draw_text_large(75, 100, "ESTAS SEGURO?", COLOR_WHITE, COLOR_BLACK, 2);
		daos_gfx_draw_text(65, 125, "D9: SI / D8: NO", COLOR_CYAN, COLOR_BLACK);
	}

	if (current_fase == FASE_2D_ESCONDIDO) {
		daos_gfx_fill_rect(60, 90, 200, 60, COLOR_BLACK);
		daos_gfx_fill_rect(58, 88, 204, 64, COLOR_WHITE);
		daos_gfx_fill_rect(60, 90, 200, 60, COLOR_BLACK);
		daos_gfx_draw_text_large(75, 105, "ESCONDIDOS!", COLOR_YELLOW_BG, COLOR_BLACK, 2);
		daos_gfx_draw_text(70, 130, "D9: INICIAR 3D", COLOR_CYAN, COLOR_BLACK);
	}

	if (current_fase == FASE_VICTORIA) {
		daos_gfx_clear(COLOR_BLACK);
		daos_gfx_fill_rect(40, 80, 240, 80, COLOR_GREEN);
		daos_gfx_fill_rect(38, 78, 244, 84, COLOR_WHITE);
		daos_gfx_fill_rect(40, 80, 240, 80, COLOR_GREEN);
		daos_gfx_draw_text_large(70, 95, "VICTORIA!", COLOR_WHITE, COLOR_GREEN, 3);
		daos_gfx_draw_text(60, 130, "Encontraste al escondedor", COLOR_WHITE, COLOR_GREEN);

		// PROTECCIÓN: Lectura segura de estadísticas
		mutex_lock(&reconocedor_map_mutex);
		uint8_t local_cajas = cajas_abiertas_count;
		uint8_t local_max = max_cajas_abrir;
		mutex_unlock(&reconocedor_map_mutex);

		char buf[32];
		buf[0] = 'C'; buf[1] = 'a'; buf[2] = 'j'; buf[3] = 'a'; buf[4] = 's'; buf[5] = ':'; buf[6] = ' ';
		buf[7] = '0' + ((local_cajas / 10) % 10);
		buf[8] = '0' + (local_cajas % 10);
		buf[9] = '/';
		buf[10] = '0' + ((local_max / 10) % 10);
		buf[11] = '0' + (local_max % 10);
		buf[12] = '\0';
		daos_gfx_draw_text(100, 145, buf, COLOR_WHITE, COLOR_GREEN);
	}

	if (current_fase == FASE_DERROTA) {
		daos_gfx_clear(COLOR_BLACK);
		daos_gfx_fill_rect(40, 80, 240, 80, COLOR_RED);
		daos_gfx_fill_rect(38, 78, 244, 84, COLOR_WHITE);
		daos_gfx_fill_rect(40, 80, 240, 80, COLOR_RED);
		daos_gfx_draw_text_large(75, 95, "DERROTA!", COLOR_WHITE, COLOR_RED, 3);

		// PROTECCIÓN: Lectura segura del contador de maniquíes
		mutex_lock(&reconocedor_map_mutex);
		uint8_t local_maniquies = maniquies_encontrados;
		uint8_t local_cajas = cajas_abiertas_count;
		uint8_t local_max = max_cajas_abrir;
		mutex_unlock(&reconocedor_map_mutex);

		if (local_maniquies >= 2) {
			daos_gfx_draw_text(50, 130, "Encontraste 2 maniquies", COLOR_WHITE, COLOR_RED);
		} else {
			daos_gfx_draw_text(40, 130, "Limite de cajas alcanzado", COLOR_WHITE, COLOR_RED);
		}

		char buf[32];
		buf[0] = 'C'; buf[1] = 'a'; buf[2] = 'j'; buf[3] = 'a'; buf[4] = 's'; buf[5] = ':'; buf[6] = ' ';
		buf[7] = '0' + ((local_cajas / 10) % 10);
		buf[8] = '0' + (local_cajas % 10);
		buf[9] = '/';
		buf[10] = '0' + ((local_max / 10) % 10);
		buf[11] = '0' + (local_max % 10);
		buf[12] = '\0';
		daos_gfx_draw_text(100, 145, buf, COLOR_WHITE, COLOR_RED);
	}

	daos_sleep_ms(50);
}

// ========================================================================
// INICIALIZACIÓN
// ========================================================================
void reconocedor_init(void) {
	game_running = 0;
	daos_sleep_ms(100);

	// INICIALIZACIÓN: Configurar mutexes y semáforos
	mutex_init(&reconocedor_game_state_mutex);
	mutex_init(&reconocedor_map_mutex);
	mutex_init(&reconocedor_player_mutex);
	mutex_init(&reconocedor_maniqui_mutex);

	// Inicializar semáforos (para uso futuro)
	sem_init(&reconocedor_input_ready_sem, 0, 1);
	sem_init(&reconocedor_logic_ready_sem, 0, 1);
	sem_init(&reconocedor_render_ready_sem, 0, 1);

	daos_gfx_clear(COLOR_BLACK);

	generar_mapa_laberinto();
	init_ray_table_3d();

	// PROTECCIÓN: Inicialización segura del estado del juego
	mutex_lock(&reconocedor_player_mutex);
	escondedor_dir = DIR_RIGHT;
	escondedor_oculto = 0;
	mutex_unlock(&reconocedor_player_mutex);

	mutex_lock(&reconocedor_game_state_mutex);
	fase_actual = FASE_2D_MOVIENDO;
	mutex_unlock(&reconocedor_game_state_mutex);

	mutex_lock(&reconocedor_maniqui_mutex);
	maniqui_1.desplegado = 0;
	maniqui_2.desplegado = 0;
	mutex_unlock(&reconocedor_maniqui_mutex);

	first_draw = 1;
	game_running = 1;

	daos_uart_puts("\r\n[RECONOCEDOR] Juego iniciado con sincronizacion\r\n");
}

void reconocedor_regenerar_mapa(void) {
	daos_random_reseed();
	generar_mapa_laberinto();

	// PROTECCIÓN: Reset seguro del estado del juego
	mutex_lock(&reconocedor_player_mutex);
	escondedor_dir = DIR_RIGHT;
	escondedor_oculto = 0;
	mutex_unlock(&reconocedor_player_mutex);

	mutex_lock(&reconocedor_game_state_mutex);
	fase_actual = FASE_2D_MOVIENDO;
	mutex_unlock(&reconocedor_game_state_mutex);

	mutex_lock(&reconocedor_maniqui_mutex);
	maniqui_1.desplegado = 0;
	maniqui_2.desplegado = 0;
	mutex_unlock(&reconocedor_maniqui_mutex);

	first_draw = 1;
}

void reconocedor_reset(void) {
	reconocedor_init();
}

void reconocedor_cleanup(void) {
	// PROTECCIÓN: Limpieza segura
	game_running = 0;
	daos_gfx_clear(COLOR_BLACK);

	// Nota: Los mutexes y semáforos no necesitan destrucción explícita
	// en este sistema embebido, pero podrían agregarse funciones
	// mutex_destroy() y sem_destroy() si fuera necesario
}

/* ============================================================ */
/* FIN DEL ARCHIVO reconocedor.c                      */
/* ============================================================ */
