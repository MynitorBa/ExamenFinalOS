// fat.h - Librería SD con múltiples métodos de inicialización
#ifndef FAT_H
#define FAT_H

#include <stdint.h>

// Configuración de pines (ajustar según tu conexión)
#define SD_CS_PORT      GPIOA
#define SD_CS_PIN       10  // PA10 = D2

// Estructura de archivo simple
typedef struct {
    char nombre[32];
    char contenido[200];
    uint8_t usado;
} Archivo;

// Configuración
#define SECTOR_START    1000
#define MAX_FILES       10
#define SECTOR_SIZE     512

// Funciones públicas
uint8_t fat_init(void);
uint8_t fat_read_sector(uint32_t sector, uint8_t *buffer);
uint8_t fat_write_sector(uint32_t sector, uint8_t *buffer);
uint8_t fat_is_ready(void);

// Comandos de archivos
uint8_t fat_crear_archivo(const char *nombre, const char *contenido);
void fat_listar_archivos(void);
uint8_t fat_leer_archivo(const char *nombre, char *buffer);
uint8_t fat_borrar_archivo(const char *nombre);

#endif // FAT_H
