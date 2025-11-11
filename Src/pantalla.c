#include "pantalla.h"

#define RCC_BASE      0x40023800
#define GPIOA_BASE    0x40020000
#define GPIOB_BASE    0x40020400
#define SPI1_BASE     0x40013000

#define RCC_AHB1ENR   ((volatile uint32_t*)(RCC_BASE + 0x30))
#define RCC_APB2ENR   ((volatile uint32_t*)(RCC_BASE + 0x44))

#define GPIOA_MODER   ((volatile uint32_t*)(GPIOA_BASE + 0x00))
#define GPIOA_OSPEEDR ((volatile uint32_t*)(GPIOA_BASE + 0x08))
#define GPIOA_AFR0    ((volatile uint32_t*)(GPIOA_BASE + 0x20))
#define GPIOA_BSRR    ((volatile uint32_t*)(GPIOA_BASE + 0x18))

#define GPIOB_MODER   ((volatile uint32_t*)(GPIOB_BASE + 0x00))
#define GPIOB_BSRR    ((volatile uint32_t*)(GPIOB_BASE + 0x18))

#define SPI1_CR1      ((volatile uint32_t*)(SPI1_BASE + 0x00))
#define SPI1_CR2      ((volatile uint32_t*)(SPI1_BASE + 0x04))
#define SPI1_SR       ((volatile uint32_t*)(SPI1_BASE + 0x08))
#define SPI1_DR       ((volatile uint32_t*)(SPI1_BASE + 0x0C))

#define CS_LOW()    *GPIOB_BSRR = (1<<22)
#define CS_HIGH()   *GPIOB_BSRR = (1<<6)
#define DC_LOW()    *GPIOA_BSRR = (1<<16)
#define DC_HIGH()   *GPIOA_BSRR = (1<<0)
#define RST_LOW()   *GPIOA_BSRR = (1<<17)
#define RST_HIGH()  *GPIOA_BSRR = (1<<1)

static const uint8_t font5x8[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00},
    {0x3E, 0x51, 0x49, 0x45, 0x3E},
    {0x00, 0x42, 0x7F, 0x40, 0x00},
    {0x42, 0x61, 0x51, 0x49, 0x46},
    {0x21, 0x41, 0x45, 0x4B, 0x31},
    {0x18, 0x14, 0x12, 0x7F, 0x10},
    {0x27, 0x45, 0x45, 0x45, 0x39},
    {0x3C, 0x4A, 0x49, 0x49, 0x30},
    {0x01, 0x71, 0x09, 0x05, 0x03},
    {0x36, 0x49, 0x49, 0x49, 0x36},
    {0x06, 0x49, 0x49, 0x29, 0x1E},
    {0x00, 0x36, 0x36, 0x00, 0x00},
    {0x7E, 0x11, 0x11, 0x11, 0x7E},
    {0x7F, 0x49, 0x49, 0x49, 0x36},
    {0x3E, 0x41, 0x41, 0x41, 0x22},
    {0x7F, 0x41, 0x41, 0x22, 0x1C},
    {0x7F, 0x49, 0x49, 0x49, 0x41},
    {0x7F, 0x09, 0x09, 0x09, 0x01},
    {0x3E, 0x41, 0x49, 0x49, 0x7A},
    {0x7F, 0x08, 0x08, 0x08, 0x7F},
    {0x00, 0x41, 0x7F, 0x41, 0x00},
    {0x20, 0x40, 0x41, 0x3F, 0x01},
    {0x7F, 0x08, 0x14, 0x22, 0x41},
    {0x7F, 0x40, 0x40, 0x40, 0x40},
    {0x7F, 0x02, 0x0C, 0x02, 0x7F},
    {0x7F, 0x04, 0x08, 0x10, 0x7F},
    {0x3E, 0x41, 0x41, 0x41, 0x3E},
    {0x7F, 0x09, 0x09, 0x09, 0x06},
    {0x3E, 0x41, 0x51, 0x21, 0x5E},
    {0x7F, 0x09, 0x19, 0x29, 0x46},
    {0x46, 0x49, 0x49, 0x49, 0x31},
    {0x01, 0x01, 0x7F, 0x01, 0x01},
    {0x3F, 0x40, 0x40, 0x40, 0x3F},
    {0x1F, 0x20, 0x40, 0x20, 0x1F},
    {0x3F, 0x40, 0x38, 0x40, 0x3F},
    {0x63, 0x14, 0x08, 0x14, 0x63},
    {0x07, 0x08, 0x70, 0x08, 0x07},
    {0x61, 0x51, 0x49, 0x45, 0x43},
};

static void delay_ms(uint32_t ms) {
    for(volatile uint32_t i = 0; i < ms * 8000; i++);
}

static void spi_write(uint8_t data) {
    while(!(*SPI1_SR & (1<<1)));
    *SPI1_DR = data;
    while(!(*SPI1_SR & (1<<0)));
    (void)*SPI1_DR;
}

static void lcd_cmd(uint8_t cmd) {
    DC_LOW();
    CS_LOW();
    spi_write(cmd);
    CS_HIGH();
}

static void lcd_data(uint8_t data) {
    DC_HIGH();
    CS_LOW();
    spi_write(data);
    CS_HIGH();
}

static int get_font_index(char c) {
    if(c >= '0' && c <= '9') return c - '0' + 1;
    if(c >= 'A' && c <= 'Z') return c - 'A' + 12;
    if(c >= 'a' && c <= 'z') return c - 'a' + 12;
    if(c == ' ') return 0;
    if(c == ':') return 11;
    return -1;
}

void pantalla_init(void) {
    *RCC_AHB1ENR |= (1<<0) | (1<<1);
    *RCC_APB2ENR |= (1<<12);

    *GPIOA_MODER &= ~((3<<10) | (3<<14));
    *GPIOA_MODER |= (2<<10) | (2<<14);
    *GPIOA_AFR0 |= (5<<20) | (5<<28);
    *GPIOA_OSPEEDR |= (3<<10) | (3<<14);

    *GPIOA_MODER &= ~((3<<0) | (3<<2));
    *GPIOA_MODER |= (1<<0) | (1<<2);

    *GPIOB_MODER &= ~(3<<12);
    *GPIOB_MODER |= (1<<12);

    CS_HIGH();
    DC_HIGH();
    RST_HIGH();

    *SPI1_CR1 = 0;
    *SPI1_CR1 = (1<<2) | (1<<8) | (1<<9) | (1<<3);
    *SPI1_CR2 = 0;
    *SPI1_CR1 |= (1<<6);

    delay_ms(100);

    RST_LOW();
    delay_ms(20);
    RST_HIGH();
    delay_ms(150);

    lcd_cmd(0x01);
    delay_ms(150);

    lcd_cmd(0x11);
    delay_ms(150);

    lcd_cmd(0x36);
    lcd_data(0xE8);

    lcd_cmd(0x3A);
    lcd_data(0x55);

    lcd_cmd(0x29);
    delay_ms(150);
}

void pantalla_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    lcd_cmd(0x2A);
    lcd_data(x0 >> 8);
    lcd_data(x0 & 0xFF);
    lcd_data(x1 >> 8);
    lcd_data(x1 & 0xFF);

    lcd_cmd(0x2B);
    lcd_data(y0 >> 8);
    lcd_data(y0 & 0xFF);
    lcd_data(y1 >> 8);
    lcd_data(y1 & 0xFF);

    lcd_cmd(0x2C);
}

void pantalla_clear(uint16_t color) {
    pantalla_fill_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, color);
}

void pantalla_draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
    if(x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) return;

    pantalla_set_window(x, y, x, y);
    DC_HIGH();
    CS_LOW();
    spi_write(color >> 8);
    spi_write(color & 0xFF);
    CS_HIGH();
}

void pantalla_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if(x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) return;
    if(x + w > SCREEN_WIDTH) w = SCREEN_WIDTH - x;
    if(y + h > SCREEN_HEIGHT) h = SCREEN_HEIGHT - y;

    pantalla_set_window(x, y, x + w - 1, y + h - 1);

    DC_HIGH();
    CS_LOW();
    for(uint32_t i = 0; i < (w * h); i++) {
        spi_write(color >> 8);
        spi_write(color & 0xFF);
    }
    CS_HIGH();
}

void pantalla_draw_circle(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t color) {
    int16_t x = radius;
    int16_t y = 0;
    int16_t err = 0;

    while(x >= y) {
        pantalla_draw_pixel(x0 + x, y0 + y, color);
        pantalla_draw_pixel(x0 + y, y0 + x, color);
        pantalla_draw_pixel(x0 - y, y0 + x, color);
        pantalla_draw_pixel(x0 - x, y0 + y, color);
        pantalla_draw_pixel(x0 - x, y0 - y, color);
        pantalla_draw_pixel(x0 - y, y0 - x, color);
        pantalla_draw_pixel(x0 + y, y0 - x, color);
        pantalla_draw_pixel(x0 + x, y0 - y, color);

        if(err <= 0) {
            y += 1;
            err += 2*y + 1;
        }
        if(err > 0) {
            x -= 1;
            err -= 2*x + 1;
        }
    }
}

void pantalla_draw_circle_filled(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t color) {
    for(int16_t y = -radius; y <= radius; y++) {
        for(int16_t x = -radius; x <= radius; x++) {
            if(x*x + y*y <= radius*radius) {
                pantalla_draw_pixel(x0 + x, y0 + y, color);
            }
        }
    }
}

void pantalla_draw_char(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg) {
    int idx = get_font_index(c);
    if(idx < 0) return;

    for(int col = 0; col < 5; col++) {
        uint8_t line = font5x8[idx][col];
        for(int row = 0; row < 8; row++) {
            if(line & (1 << row)) {
                pantalla_draw_pixel(x + col, y + row, color);
            } else {
                pantalla_draw_pixel(x + col, y + row, bg);
            }
        }
    }
}

void pantalla_draw_string(uint16_t x, uint16_t y, const char* str, uint16_t color, uint16_t bg) {
    uint16_t pos_x = x;
    while(*str) {
        pantalla_draw_char(pos_x, y, *str, color, bg);
        pos_x += 6;
        str++;
    }
}

void pantalla_draw_string_large(uint16_t x, uint16_t y, const char* str, uint16_t color, uint16_t bg, uint8_t scale) {
    uint16_t pos_x = x;

    while(*str) {
        int idx = get_font_index(*str);

        if(idx < 0 || *str == ' ') {
            pos_x += 6 * scale;
            str++;
            continue;
        }

        for(int col = 0; col < 5; col++) {
            uint8_t line = font5x8[idx][col];
            for(int row = 0; row < 8; row++) {
                uint16_t pixel_color = (line & (1 << row)) ? color : bg;
                for(uint8_t sx = 0; sx < scale; sx++) {
                    for(uint8_t sy = 0; sy < scale; sy++) {
                        pantalla_draw_pixel(pos_x + col*scale + sx, y + row*scale + sy, pixel_color);
                    }
                }
            }
        }

        pos_x += 6 * scale;
        str++;
    }
}
