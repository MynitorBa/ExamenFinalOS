/**
 * ============================================================================
 * DaOS v2.0 - RAMFS (RAM File System)
 * ============================================================================
 * Sistema de archivos en RAM con API tipo POSIX
 * ============================================================================
 */

#ifndef RAMFS_H // Guarda de inclusión para RAMFS
#define RAMFS_H

#pragma once
#include <stdint.h>
#include <stddef.h> // Incluye size_t

/* ========================================================================== */
/* CONFIGURACIÓN                                     */
/* ========================================================================== */

/** Máximo número de archivos. */
#define RAMFS_MAX_FILES 16
/** Longitud máxima del nombre de archivo. */
#define RAMFS_MAX_FILENAME 32
/** Tamaño de cada bloque en bytes. */
#define RAMFS_BLOCK_SIZE 256
/** Total de bloques disponibles (capacidad total). */
#define RAMFS_MAX_BLOCKS 64
/** Máximo de archivos abiertos simultáneamente. */
#define RAMFS_MAX_OPEN 8

/* ========================================================================== */
/* MODOS DE APERTURA                                 */
/* ========================================================================== */

/** Abrir solo para lectura. */
#define RAMFS_O_RDONLY  0x01
/** Abrir solo para escritura. */
#define RAMFS_O_WRONLY  0x02
/** Abrir para lectura y escritura. */
#define RAMFS_O_RDWR    0x03
/** Crear el archivo si no existe. */
#define RAMFS_O_CREAT   0x04
/** Truncar el archivo a tamaño 0 si existe. */
#define RAMFS_O_TRUNC   0x08
/** Posicionar el puntero al final antes de cada escritura. */
#define RAMFS_O_APPEND  0x10

/* ========================================================================== */
/* SEEK WHENCE                                       */
/* ========================================================================== */

/** Desplazamiento relativo al inicio del archivo. */
#ifndef SEEK_SET
#define SEEK_SET 0
/** Desplazamiento relativo a la posición actual. */
#define SEEK_CUR 1
/** Desplazamiento relativo al final del archivo. */
#define SEEK_END 2
#endif

/* ========================================================================== */
/* ESTRUCTURAS INTERNAS                              */
/* ========================================================================== */

/**
 * Inodo - Metadatos de un archivo
 */
typedef struct {
    char name[RAMFS_MAX_FILENAME];  /** Nombre del archivo. */
    uint32_t size;                   /** Tamaño actual del archivo en bytes. */
    uint32_t capacity;               /** Capacidad asignada en bytes (bloques*RAMFS_BLOCK_SIZE). */
    uint16_t start_block;            /** Índice del primer bloque de datos. */
    uint16_t num_blocks;             /** Número total de bloques asignados. */
    uint8_t in_use;                  /** Bandera: 1 si el inodo está en uso. */
} ramfs_inode_t;

/**
 * File Descriptor - Estado de un archivo abierto
 */
typedef struct {
    int inode_idx;                   /** Índice del inodo asociado (-1 si no está abierto). */
    uint32_t position;               /** Posición actual de lectura/escritura (offset). */
    uint8_t mode;                    /** Modo de apertura (RAMFS_O_*). */
    uint8_t is_open;                 /** Bandera: 1 si está abierto. */
} ramfs_fd_t;

/* ========================================================================== */
/* API PÚBLICA (POSIX-LIKE)                          */
/* ========================================================================== */

/**
 * Inicializar el sistema de archivos RAMFS.
 */
void ramfs_init(void);

/**
 * Abrir o crear un archivo.
 * @param path Nombre del archivo.
 * @param flags Flags de apertura (RAMFS_O_*).
 * @return File descriptor (>=0) o error (-1).
 */
int ramfs_open(const char* path, int flags);

/**
 * Leer datos desde un archivo abierto.
 * @param fd File descriptor.
 * @param buf Buffer de destino.
 * @param count Número de bytes a leer.
 * @return Bytes leídos o error (<0).
 */
int ramfs_read(int fd, void* buf, size_t count);

/**
 * Escribir datos a un archivo abierto.
 * @param fd File descriptor.
 * @param buf Buffer de origen.
 * @param count Número de bytes a escribir.
 * @return Bytes escritos o error (<0).
 */
int ramfs_write(int fd, const void* buf, size_t count);

/**
 * Mover el puntero de lectura/escritura.
 * @param fd File descriptor.
 * @param offset Desplazamiento.
 * @param whence SEEK_SET, SEEK_CUR o SEEK_END.
 * @return Nueva posición o error (-1).
 */
int ramfs_seek(int fd, int offset, int whence);

/**
 * Cerrar un archivo abierto.
 * @param fd File descriptor.
 * @return 0 si OK, -1 si error.
 */
int ramfs_close(int fd);

/* ========================================================================== */
/* API EXTENDIDA                                     */
/* ========================================================================== */

/**
 * Crear archivo con contenido inicial (función auxiliar).
 * @return 0 si OK, -1 si error.
 */
int ramfs_create(const char* name, const void* data, uint32_t size);

/**
 * Eliminar un archivo.
 * @param name Nombre del archivo.
 * @return 0 si OK, -1 si error.
 */
int ramfs_delete(const char* name);

/**
 * Verificar si un archivo existe.
 * @return 1 si existe, 0 si no.
 */
int ramfs_exists(const char* name);

/**
 * Obtener tamaño de un archivo.
 * @return Tamaño en bytes o -1 si no existe.
 */
int ramfs_get_size(const char* name);

/**
 * Listar archivos a un buffer de salida.
 * @param out Buffer de salida.
 * @param max Tamaño máximo del buffer.
 * @return Bytes escritos en el buffer.
 */
int ramfs_listdir(char* out, int max);

/**
 * Listar archivos por UART (para debugging).
 */
void ramfs_listdir_uart(void);

/**
 * Obtener estadísticas de uso del filesystem.
 * @param total_files Puntero para el número total de archivos.
 * @param used_blocks Puntero para los bloques usados.
 * @param free_blocks Puntero para los bloques libres.
 */
void ramfs_stats(int* total_files, int* used_blocks, int* free_blocks);

/**
 * Truncar archivo a tamaño especificado (cambiar tamaño).
 * @param name Nombre del archivo.
 * @param new_size Nuevo tamaño.
 * @return 0 si OK, -1 si error.
 */
int ramfs_truncate(const char* name, uint32_t new_size);

/**
 * Renombrar archivo.
 * @param old_name Nombre actual.
 * @param new_name Nombre nuevo.
 * @return 0 si OK, -1 si error.
 */
int ramfs_rename(const char* old_name, const char* new_name);

/**
 * Añadir datos al final de un archivo.
 * @return 0 si OK, -1 si error.
 */
int ramfs_append(const char* name, const void* data, uint32_t size);

#endif /* RAMFS_H */
