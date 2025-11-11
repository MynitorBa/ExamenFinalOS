#pragma once
#include <stdint.h>

#define FS_MAX_FILES 8
#define FS_MAX_FILENAME 16
#define FS_BLOCK_SIZE 256
#define FS_MAX_BLOCKS 32

typedef struct {
char name[FS_MAX_FILENAME];
uint32_t size;
uint32_t start_block;
uint8_t in_use;
} FSInode;

typedef struct {
int fd;
uint32_t position;
FSInode *inode;
} FSFile;

/* Inicializar filesystem */
void fs_init(void);

/* Crear/escribir archivo */
int fs_create(const char *name, const void *data, uint32_t size);

/* Abrir archivo para lectura */
FSFile* fs_open(const char *name);

/* Leer desde archivo */
int fs_read(FSFile *file, void *buffer, uint32_t size);

/* Cerrar archivo */
void fs_close(FSFile *file);

/* Listar archivos */
void fs_list(void);

/* Obtener tamaño de archivo */
int fs_get_size(const char *name);

/* Eliminar archivo */
int fs_delete(const char *name);
