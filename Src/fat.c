// fat.c - Implementación SD con múltiples métodos
#include "fat.h"
#include "uart.h"
#include <string.h>

// Registros SPI1 y GPIO
#define RCC_BASE        0x40023800
#define GPIOA_BASE      0x40020000
#define GPIOB_BASE      0x40020400
#define SPI1_BASE       0x40013000

#define RCC_AHB1ENR     (*(volatile uint32_t*)(RCC_BASE + 0x30))
#define RCC_APB2ENR     (*(volatile uint32_t*)(RCC_BASE + 0x44))

#define GPIOA_MODER     (*(volatile uint32_t*)(GPIOA_BASE + 0x00))
#define GPIOA_OSPEEDR   (*(volatile uint32_t*)(GPIOA_BASE + 0x08))
#define GPIOA_ODR       (*(volatile uint32_t*)(GPIOA_BASE + 0x14))

#define GPIOB_MODER     (*(volatile uint32_t*)(GPIOB_BASE + 0x00))
#define GPIOB_OSPEEDR   (*(volatile uint32_t*)(GPIOB_BASE + 0x08))
#define GPIOB_PUPDR     (*(volatile uint32_t*)(GPIOB_BASE + 0x0C))
#define GPIOB_AFRL      (*(volatile uint32_t*)(GPIOB_BASE + 0x20))

#define SPI1_CR1        (*(volatile uint32_t*)(SPI1_BASE + 0x00))
#define SPI1_SR         (*(volatile uint32_t*)(SPI1_BASE + 0x08))
#define SPI1_DR         (*(volatile uint32_t*)(SPI1_BASE + 0x0C))

// Variables privadas
static uint8_t sd_ready = 0;
static uint8_t is_sdhc = 0;
static uint8_t sector_buffer[SECTOR_SIZE];

// Funciones privadas
static void delay_ms(uint32_t ms);
static void cs_high(void);
static void cs_low(void);
static uint8_t spi_transfer(uint8_t data);
static void spi_init(void);
static void spi_set_speed(uint8_t divisor);
static uint8_t sd_command(uint8_t cmd, uint32_t arg);
static uint8_t sd_init_method_1(void);  // Método estándar
static uint8_t sd_init_method_2(void);  // Método alternativo 1
static uint8_t sd_init_method_3(void);  // Método alternativo 2

// ==================== FUNCIONES AUXILIARES ====================

static void delay_ms(uint32_t ms) {
    for(volatile uint32_t i = 0; i < ms * 8000; i++);
}

static void cs_high(void) {
    GPIOA_ODR |= (1 << SD_CS_PIN);
}

static void cs_low(void) {
    GPIOA_ODR &= ~(1 << SD_CS_PIN);
}

static uint8_t spi_transfer(uint8_t data) {
    while(!(SPI1_SR & (1 << 1)));
    SPI1_DR = data;
    while(!(SPI1_SR & (1 << 0)));
    return SPI1_DR;
}

static void spi_init(void) {
    // Habilitar clocks
    RCC_AHB1ENR |= (1 << 0) | (1 << 1);
    RCC_APB2ENR |= (1 << 12);

    // PB3 - SCK, PB4 - MISO, PB5 - MOSI
    GPIOB_MODER &= ~((3 << 6) | (3 << 8) | (3 << 10));
    GPIOB_MODER |= (2 << 6) | (2 << 8) | (2 << 10);
    GPIOB_AFRL &= ~((0xF << 12) | (0xF << 16) | (0xF << 20));
    GPIOB_AFRL |= (5 << 12) | (5 << 16) | (5 << 20);
    GPIOB_OSPEEDR |= (3 << 6) | (3 << 10);
    GPIOB_PUPDR &= ~(3 << 8);
    GPIOB_PUPDR |= (1 << 8);

    // PA10 - CS
    GPIOA_MODER &= ~(3 << (SD_CS_PIN * 2));
    GPIOA_MODER |= (1 << (SD_CS_PIN * 2));
    GPIOA_OSPEEDR |= (3 << (SD_CS_PIN * 2));
    cs_high();

    // SPI1 configuración inicial (velocidad lenta)
    SPI1_CR1 = 0;
    SPI1_CR1 |= (7 << 3);   // /256
    SPI1_CR1 |= (1 << 2);   // Master
    SPI1_CR1 |= (1 << 9);   // SSM
    SPI1_CR1 |= (1 << 8);   // SSI
    SPI1_CR1 |= (1 << 6);   // Enable

    delay_ms(100);
}

static void spi_set_speed(uint8_t divisor) {
    SPI1_CR1 &= ~(1 << 6);
    SPI1_CR1 &= ~(7 << 3);
    SPI1_CR1 |= (divisor << 3);
    SPI1_CR1 |= (1 << 6);
    delay_ms(10);
}

static uint8_t sd_command(uint8_t cmd, uint32_t arg) {
    uint8_t response, crc = 0xFF;

    if(cmd == 0) crc = 0x95;
    if(cmd == 8) crc = 0x87;

    // Esperar SD lista
    for(int i = 0; i < 10; i++) spi_transfer(0xFF);

    spi_transfer(cmd | 0x40);
    spi_transfer((arg >> 24) & 0xFF);
    spi_transfer((arg >> 16) & 0xFF);
    spi_transfer((arg >> 8) & 0xFF);
    spi_transfer(arg & 0xFF);
    spi_transfer(crc);

    // Esperar respuesta
    for(int i = 0; i < 10; i++) {
        response = spi_transfer(0xFF);
        if(!(response & 0x80)) return response;
    }

    return 0xFF;
}

// ==================== MÉTODOS DE INICIALIZACIÓN ====================

// MÉTODO 1: Estándar con timeouts cortos
static uint8_t sd_init_method_1(void) {
    uint8_t response;

    uart_puts("[M1] Init rapido...\r\n");

    cs_high();
    for(int i = 0; i < 20; i++) spi_transfer(0xFF);

    cs_low();
    delay_ms(1);
    response = sd_command(0, 0);
    cs_high();
    spi_transfer(0xFF);

    if(response != 0x01) {
        uart_puts("[M1] CMD0 fallo\r\n");
        return 0;
    }

    cs_low();
    delay_ms(1);
    response = sd_command(8, 0x1AA);
    if(response == 0x01) {
        for(int i = 0; i < 4; i++) spi_transfer(0xFF);
    }
    cs_high();
    spi_transfer(0xFF);

    // ACMD41 rápido (50 intentos)
    for(int i = 0; i < 50; i++) {
        delay_ms(10);
        cs_low();
        sd_command(55, 0);
        cs_high();
        spi_transfer(0xFF);

        cs_low();
        response = sd_command(41, 0x40000000);
        cs_high();
        spi_transfer(0xFF);

        if(response == 0x00) {
            uart_puts("[M1] OK!\r\n");
            return 1;
        }
    }

    uart_puts("[M1] Timeout\r\n");
    return 0;
}

// MÉTODO 2: Con delays largos
static uint8_t sd_init_method_2(void) {
    uint8_t response;

    uart_puts("[M2] Init con delays largos...\r\n");

    cs_high();
    for(int i = 0; i < 50; i++) spi_transfer(0xFF);
    delay_ms(50);

    cs_low();
    delay_ms(10);
    response = sd_command(0, 0);
    cs_high();
    delay_ms(10);

    if(response != 0x01) {
        uart_puts("[M2] CMD0 fallo\r\n");
        return 0;
    }

    cs_low();
    delay_ms(10);
    response = sd_command(8, 0x1AA);
    if(response == 0x01) {
        for(int i = 0; i < 4; i++) spi_transfer(0xFF);
    }
    cs_high();
    delay_ms(10);

    // ACMD41 con delays largos
    for(int i = 0; i < 100; i++) {
        delay_ms(50);
        cs_low();
        delay_ms(5);
        sd_command(55, 0);
        cs_high();
        delay_ms(5);

        cs_low();
        delay_ms(5);
        response = sd_command(41, 0x40000000);
        cs_high();
        delay_ms(5);

        if(response == 0x00) {
            uart_puts("[M2] OK!\r\n");
            return 1;
        }
    }

    uart_puts("[M2] Timeout\r\n");
    return 0;
}

// MÉTODO 3: Modo compatibilidad (sin HCS, como SDv1)
static uint8_t sd_init_method_3(void) {
    uint8_t response;

    uart_puts("[M3] Modo compatibilidad SDv1...\r\n");

    cs_high();
    for(int i = 0; i < 30; i++) spi_transfer(0xFF);
    delay_ms(20);

    cs_low();
    delay_ms(2);
    response = sd_command(0, 0);
    cs_high();
    delay_ms(2);

    if(response != 0x01) {
        uart_puts("[M3] CMD0 fallo\r\n");
        return 0;
    }

    // Sin CMD8 (modo SDv1)
    // ACMD41 sin HCS bit
    for(int i = 0; i < 200; i++) {
        delay_ms(20);
        cs_low();
        delay_ms(2);
        sd_command(55, 0);
        cs_high();
        delay_ms(2);

        cs_low();
        delay_ms(2);
        response = sd_command(41, 0x00000000);  // Sin HCS
        cs_high();
        delay_ms(2);

        if(response == 0x00) {
            uart_puts("[M3] OK!\r\n");
            is_sdhc = 0;  // Modo byte addressing
            return 1;
        }
    }

    uart_puts("[M3] Timeout\r\n");
    return 0;
}

// ==================== FUNCIONES PÚBLICAS ====================

uint8_t fat_init(void) {
    uart_puts("\r\n=== INICIALIZANDO SD ===\r\n");

    spi_init();
    sd_ready = 0;

    // Probar método 1
    if(sd_init_method_1()) {
        is_sdhc = 1;
        spi_set_speed(2);  // /8 = 2MHz
        uart_puts("Metodo 1 exitoso! (SDHC)\r\n");
        sd_ready = 1;
        return 1;
    }

    delay_ms(100);

    // Probar método 2
    if(sd_init_method_2()) {
        is_sdhc = 1;
        spi_set_speed(3);  // /16 = 1MHz (más lento)
        uart_puts("Metodo 2 exitoso! (SDHC)\r\n");
        sd_ready = 1;
        return 1;
    }

    delay_ms(100);

    // Probar método 3
    if(sd_init_method_3()) {
        is_sdhc = 0;
        spi_set_speed(4);  // /32 = 500kHz (muy lento)
        uart_puts("Metodo 3 exitoso! (SDSC)\r\n");
        sd_ready = 1;
        return 1;
    }

    uart_puts("ERROR: Todos los metodos fallaron\r\n");
    return 0;
}

uint8_t fat_read_sector(uint32_t sector, uint8_t *buffer) {
    if(!sd_ready) return 0;

    uint32_t addr = is_sdhc ? sector : (sector * 512);
    uint8_t response;

    cs_low();
    delay_ms(1);
    response = sd_command(17, addr);

    if(response != 0x00) {
        cs_high();
        return 0;
    }

    // Esperar token 0xFE
    for(int i = 0; i < 10000; i++) {
        response = spi_transfer(0xFF);
        if(response == 0xFE) break;
    }

    if(response != 0xFE) {
        cs_high();
        return 0;
    }

    // Leer 512 bytes
    for(int i = 0; i < SECTOR_SIZE; i++) {
        buffer[i] = spi_transfer(0xFF);
    }

    spi_transfer(0xFF);  // CRC
    spi_transfer(0xFF);

    cs_high();
    spi_transfer(0xFF);

    return 1;
}

uint8_t fat_write_sector(uint32_t sector, uint8_t *buffer) {
    if(!sd_ready) return 0;

    uint32_t addr = is_sdhc ? sector : (sector * 512);
    uint8_t response;

    cs_low();
    delay_ms(1);
    response = sd_command(24, addr);

    if(response != 0x00) {
        cs_high();
        return 0;
    }

    delay_ms(1);
    spi_transfer(0xFF);
    spi_transfer(0xFE);  // Token

    for(int i = 0; i < SECTOR_SIZE; i++) {
        spi_transfer(buffer[i]);
    }

    spi_transfer(0xFF);  // CRC
    spi_transfer(0xFF);

    response = spi_transfer(0xFF);

    if((response & 0x1F) != 0x05) {
        cs_high();
        return 0;
    }

    // Esperar fin de escritura
    for(int i = 0; i < 10000; i++) {
        if(spi_transfer(0xFF) != 0x00) break;
    }

    cs_high();
    spi_transfer(0xFF);

    return 1;
}

uint8_t fat_is_ready(void) {
    return sd_ready;
}

// ==================== COMANDOS DE ARCHIVOS ====================

uint8_t fat_crear_archivo(const char *nombre, const char *contenido) {
    if(!sd_ready) return 0;

    if(!fat_read_sector(SECTOR_START, sector_buffer)) {
        uart_puts("Error: No se puede leer sector\r\n");
        return 0;
    }

    Archivo *archivos = (Archivo*)sector_buffer;
    int slot = -1;

    for(int i = 0; i < MAX_FILES; i++) {
        if(!archivos[i].usado) {
            if(slot == -1) slot = i;
        } else if(strcmp(archivos[i].nombre, nombre) == 0) {
            slot = i;
            break;
        }
    }

    if(slot == -1) {
        uart_puts("Error: No hay espacio\r\n");
        return 0;
    }

    archivos[slot].usado = 1;
    strncpy(archivos[slot].nombre, nombre, 31);
    archivos[slot].nombre[31] = '\0';
    strncpy(archivos[slot].contenido, contenido, 199);
    archivos[slot].contenido[199] = '\0';

    if(fat_write_sector(SECTOR_START, sector_buffer)) {
        uart_puts("Archivo '");
        uart_puts(nombre);
        uart_puts("' guardado.\r\n");
        return 1;
    }

    uart_puts("Error al escribir\r\n");
    return 0;
}

void fat_listar_archivos(void) {
    if(!sd_ready) {
        uart_puts("SD no lista\r\n");
        return;
    }

    if(!fat_read_sector(SECTOR_START, sector_buffer)) {
        uart_puts("Error al leer\r\n");
        return;
    }

    Archivo *archivos = (Archivo*)sector_buffer;
    uint8_t count = 0;

    uart_puts("\r\n=== ARCHIVOS ===\r\n");

    for(int i = 0; i < MAX_FILES; i++) {
        if(archivos[i].usado) {
            uart_puts("- ");
            uart_puts(archivos[i].nombre);
            uart_puts(": ");
            uart_puts(archivos[i].contenido);
            uart_puts("\r\n");
            count++;
        }
    }

    if(count == 0) {
        uart_puts("(vacio)\r\n");
    }

    uart_puts("================\r\n");
}

uint8_t fat_leer_archivo(const char *nombre, char *buffer) {
    if(!sd_ready) return 0;

    if(!fat_read_sector(SECTOR_START, sector_buffer)) return 0;

    Archivo *archivos = (Archivo*)sector_buffer;

    for(int i = 0; i < MAX_FILES; i++) {
        if(archivos[i].usado && strcmp(archivos[i].nombre, nombre) == 0) {
            strcpy(buffer, archivos[i].contenido);
            return 1;
        }
    }

    return 0;
}

uint8_t fat_borrar_archivo(const char *nombre) {
    if(!sd_ready) return 0;

    if(!fat_read_sector(SECTOR_START, sector_buffer)) return 0;

    Archivo *archivos = (Archivo*)sector_buffer;

    for(int i = 0; i < MAX_FILES; i++) {
        if(archivos[i].usado && strcmp(archivos[i].nombre, nombre) == 0) {
            archivos[i].usado = 0;
            return fat_write_sector(SECTOR_START, sector_buffer);
        }
    }

    return 0;
}
