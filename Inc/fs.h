#pragma once // Directiva alternativa de guarda
#include <stdint.h> // Incluye tipos de enteros fijos

/** Número máximo de archivos soportados. */
#define FS_MAX_FILES 8
/** Longitud máxima del nombre de un archivo. */
#define FS_MAX_FILENAME 16
/** Tamaño de cada bloque de datos en bytes. */
#define FS_BLOCK_SIZE 256
/** Número máximo de bloques disponibles en el sistema de archivos. */
#define FS_MAX_BLOCKS 32

/** Estructura Inodo (metadatos del archivo). */
typedef struct {
char name[FS_MAX_FILENAME];  /** Nombre del archivo. */
uint32_t size;               /** Tamaño del archivo en bytes. */
uint32_t start_block;        /** Primer bloque de datos del archivo. */
uint8_t in_use;              /** Bandera: 1 si está en uso, 0 si está libre. */
} FSInode;

/** Estructura de archivo abierto (descriptor de archivo). */
typedef struct {
int fd;                      /** Descriptor de archivo (File Descriptor). */
uint32_t position;           /** Posición actual del puntero de lectura/escritura. */
FSInode *inode;              /** Puntero al inodo asociado en el sistema. */
} FSFile;

/* Inicializar filesystem */
/** Inicializa la estructura del sistema de archivos. */
void fs_init(void);

/* Crear/escribir archivo */
/**
 * Crea un nuevo archivo y escribe datos.
 * @return 0 en éxito, < 0 en error.
 */
int fs_create(const char *name, const void *data, uint32_t size);

/* Abrir archivo para lectura */
/**
 * Abre un archivo existente.
 * @return Puntero a la estructura FSFile o NULL si no existe.
 */
FSFile* fs_open(const char *name);

/* Leer desde archivo */
/**
 * Lee datos desde la posición actual del archivo.
 * @return Número de bytes leídos, o < 0 si error.
 */
int fs_read(FSFile *file, void *buffer, uint32_t size);

/* Cerrar archivo */
/** Cierra un descriptor de archivo abierto. */
void fs_close(FSFile *file);

/* Listar archivos */
/** Muestra la lista de archivos disponibles. */
void fs_list(void);

/* Obtener tamaño de archivo */
/**
 * Obtiene el tamaño de un archivo por su nombre.
 * @return Tamaño en bytes, o < 0 si error.
 */
int fs_get_size(const char *name);

/* Eliminar archivo */
/**
 * Elimina un archivo y libera sus bloques.
 * @return 0 en éxito, < 0 si error.
 */
int fs_delete(const char *name);
