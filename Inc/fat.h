// fat.h - Librería SD con múltiples métodos de inicialización
#ifndef FAT_H // Guarda de inclusión para la librería FAT
#define FAT_H

#include <stdint.h> // Incluye tipos de enteros fijos

// Configuración de pines (ajustar según tu conexión)
/** Puerto para el pin Chip Select (CS) del SD. */
#define SD_CS_PORT      GPIOA
/** Pin específico para Chip Select (CS) del SD (ej: PA10 = D2). */
#define SD_CS_PIN       10

// Estructura de archivo simple
/** Estructura que representa un archivo dentro del sistema FAT simple. */
typedef struct {
    char nombre[32];    /** Nombre del archivo. */
    char contenido[200];/** Contenido del archivo (limitado a 200 bytes). */
    uint8_t usado;      /** Bandera que indica si la entrada está en uso (1) o libre (0). */
} Archivo;

// Configuración
/** Sector donde comienza el sistema de archivos (ej: 1000). */
#define SECTOR_START    1000
/** Número máximo de archivos soportados. */
#define MAX_FILES       10
/** Tamaño estándar de un sector en bytes (512). */
#define SECTOR_SIZE     512

// Funciones públicas
/**
 * Inicializa el sistema de archivos FAT y la tarjeta SD.
 * @return 0 en éxito, > 0 si error.
 */
uint8_t fat_init(void);

/**
 * Lee un sector específico de la tarjeta SD.
 * @param sector Número de sector a leer.
 * @param buffer Buffer de destino (debe ser de SECTOR_SIZE).
 * @return 0 en éxito, > 0 si error.
 */
uint8_t fat_read_sector(uint32_t sector, uint8_t *buffer);

/**
 * Escribe un sector específico en la tarjeta SD.
 * @param sector Número de sector a escribir.
 * @param buffer Buffer de origen (debe ser de SECTOR_SIZE).
 * @return 0 en éxito, > 0 si error.
 */
uint8_t fat_write_sector(uint32_t sector, uint8_t *buffer);

/**
 * Verifica si la tarjeta SD y el sistema FAT están listos.
 * @return 1 si está listo, 0 si no.
 */
uint8_t fat_is_ready(void);

// Comandos de archivos
/**
 * Crea un nuevo archivo en el sistema.
 * @param nombre Nombre del nuevo archivo.
 * @param contenido Contenido inicial del archivo.
 * @return 0 en éxito, > 0 si error.
 */
uint8_t fat_crear_archivo(const char *nombre, const char *contenido);

/**
 * Muestra una lista de todos los archivos existentes.
 */
void fat_listar_archivos(void);

/**
 * Lee el contenido de un archivo por su nombre.
 * @param nombre Nombre del archivo a leer.
 * @param buffer Buffer donde se almacenará el contenido.
 * @return 0 en éxito, > 0 si error.
 */
uint8_t fat_leer_archivo(const char *nombre, char *buffer);

/**
 * Borra un archivo por su nombre.
 * @param nombre Nombre del archivo a borrar.
 * @return 0 en éxito, > 0 si error.
 */
uint8_t fat_borrar_archivo(const char *nombre);

#endif // FAT_H
