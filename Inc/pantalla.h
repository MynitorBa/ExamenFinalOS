#ifndef PANTALLA_H
#define PANTALLA_H

#include <stdint.h>

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0
#define COLOR_BLUE    0x001F
#define COLOR_YELLOW  0xFFE0
#define COLOR_CYAN    0x07FF
#define COLOR_MAGENTA 0xF81F
#define COLOR_GRAY    0xAD55

void pantalla_init(void);
void pantalla_clear(uint16_t color);
void pantalla_draw_pixel(uint16_t x, uint16_t y, uint16_t color);
void pantalla_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void pantalla_draw_circle(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t color);
void pantalla_draw_circle_filled(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t color);
void pantalla_draw_char(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg);
void pantalla_draw_string(uint16_t x, uint16_t y, const char* str, uint16_t color, uint16_t bg);
void pantalla_draw_string_large(uint16_t x, uint16_t y, const char* str, uint16_t color, uint16_t bg, uint8_t scale);
void pantalla_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

#endif
