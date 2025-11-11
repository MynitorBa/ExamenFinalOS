#ifndef PANTALLA_H // Guarda de inclusión para la librería de pantalla
#define PANTALLA_H

#include <stdint.h> // Incluye tipos de enteros fijos

/** Ancho de la pantalla en píxeles. */
#define SCREEN_WIDTH  320
/** Alto de la pantalla en píxeles. */
#define SCREEN_HEIGHT 240

// Definiciones de colores (formato RGB565)
/** Color negro (0x0000). */
#define COLOR_BLACK   0x0000
/** Color blanco (0xFFFF). */
#define COLOR_WHITE   0xFFFF
/** Color rojo. */
#define COLOR_RED     0xF800
/** Color verde. */
#define COLOR_GREEN   0x07E0
/** Color azul. */
#define COLOR_BLUE    0x001F
/** Color amarillo. */
#define COLOR_YELLOW  0xFFE0
/** Color cian. */
#define COLOR_CYAN    0x07FF
/** Color magenta. */
#define COLOR_MAGENTA 0xF81F
/** Color gris. */
#define COLOR_GRAY    0xAD55

/** Inicializa el subsistema de la pantalla. */
void pantalla_init(void);

/**
 * Limpia la pantalla con un color uniforme.
 * @param color Color de fondo.
 */
void pantalla_clear(uint16_t color);

/**
 * Dibuja un solo píxel en una coordenada.
 * @param x Coordenada X.
 * @param y Coordenada Y.
 * @param color Color del píxel.
 */
void pantalla_draw_pixel(uint16_t x, uint16_t y, uint16_t color);

/**
 * Rellena un área rectangular.
 * @param x Coordenada X inicial.
 * @param y Coordenada Y inicial.
 * @param w Ancho.
 * @param h Alto.
 * @param color Color de relleno.
 */
void pantalla_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

/**
 * Dibuja el contorno de un círculo.
 * @param x0 Centro X.
 * @param y0 Centro Y.
 * @param radius Radio.
 * @param color Color.
 */
void pantalla_draw_circle(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t color);

/**
 * Dibuja un círculo relleno.
 * @param x0 Centro X.
 * @param y0 Centro Y.
 * @param radius Radio.
 * @param color Color de relleno.
 */
void pantalla_draw_circle_filled(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t color);

/**
 * Dibuja un carácter.
 * @param x Coordenada X.
 * @param y Coordenada Y.
 * @param c Carácter.
 * @param color Color del texto.
 * @param bg Color de fondo.
 */
void pantalla_draw_char(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg);

/**
 * Dibuja una cadena de texto.
 * @param x Coordenada X.
 * @param y Coordenada Y.
 * @param str Cadena de texto.
 * @param color Color del texto.
 * @param bg Color de fondo.
 */
void pantalla_draw_string(uint16_t x, uint16_t y, const char* str, uint16_t color, uint16_t bg);

/**
 * Dibuja una cadena de texto escalada.
 * @param x Coordenada X.
 * @param y Coordenada Y.
 * @param str Cadena de texto.
 * @param color Color del texto.
 * @param bg Color de fondo.
 * @param scale Factor de escala (1, 2, etc.).
 */
void pantalla_draw_string_large(uint16_t x, uint16_t y, const char* str, uint16_t color, uint16_t bg, uint8_t scale);

/**
 * Establece una ventana de dibujo (área de interés).
 * @param x0 X inicial.
 * @param y0 Y inicial.
 * @param x1 X final.
 * @param y1 Y final.
 */
void pantalla_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

#endif // PANTALLA_H
