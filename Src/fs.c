#include "fs.h"
#include "uart.h"
#include <string.h>

/* Estructuras internas */
static FSInode inodes[FS_MAX_FILES];
static uint8_t data_blocks[FS_MAX_BLOCKS][FS_BLOCK_SIZE];
static uint8_t block_used[FS_MAX_BLOCKS];
static FSFile open_files[FS_MAX_FILES];

/* Inicializar filesystem */
void fs_init(void) {
    // Limpiar inodos
    for (int i = 0; i < FS_MAX_FILES; i++) {
        inodes[i].in_use = 0;
        inodes[i].name[0] = '\0';
        inodes[i].size = 0;
        inodes[i].start_block = 0;
        open_files[i].fd = -1;
    }

    // Limpiar bloques
    for (int i = 0; i < FS_MAX_BLOCKS; i++) {
        block_used[i] = 0;
    }
}

/* Encontrar inodo libre */
static int find_free_inode(void) {
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (!inodes[i].in_use) {
            return i;
        }
    }
    return -1;
}

/* Encontrar bloques contiguos libres */
static int find_free_blocks(uint32_t num_blocks) {
    for (int i = 0; i <= FS_MAX_BLOCKS - num_blocks; i++) {
        int found = 1;
        for (uint32_t j = 0; j < num_blocks; j++) {
            if (block_used[i + j]) {
                found = 0;
                break;
            }
        }
        if (found) {
            return i;
        }
    }
    return -1;
}

/* Crear archivo */
int fs_create(const char *name, const void *data, uint32_t size) {
    // Validar nombre
    if (!name || strlen(name) >= FS_MAX_FILENAME) {
        return -1;
    }

    // Verificar si ya existe
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (inodes[i].in_use && strcmp(inodes[i].name, name) == 0) {
            return -1; // Ya existe
        }
    }

    // Encontrar inodo libre
    int inode_idx = find_free_inode();
    if (inode_idx < 0) {
        return -1; // No hay inodos libres
    }

    // Calcular bloques necesarios
    uint32_t num_blocks = (size + FS_BLOCK_SIZE - 1) / FS_BLOCK_SIZE;
    if (num_blocks > FS_MAX_BLOCKS) {
        return -1; // Archivo demasiado grande
    }

    // Encontrar bloques libres
    int start_block = find_free_blocks(num_blocks);
    if (start_block < 0) {
        return -1; // No hay espacio
    }

    // Marcar bloques como usados
    for (uint32_t i = 0; i < num_blocks; i++) {
        block_used[start_block + i] = 1;
    }

    // Copiar datos
    const uint8_t *src = (const uint8_t *)data;
    for (uint32_t i = 0; i < size; i++) {
        uint32_t block = start_block + (i / FS_BLOCK_SIZE);
        uint32_t offset = i % FS_BLOCK_SIZE;
        data_blocks[block][offset] = src[i];
    }

    // Crear inodo
    strcpy(inodes[inode_idx].name, name);
    inodes[inode_idx].size = size;
    inodes[inode_idx].start_block = start_block;
    inodes[inode_idx].in_use = 1;

    return 0; // Éxito
}

/* Abrir archivo */
FSFile* fs_open(const char *name) {
    // Buscar archivo
    FSInode *inode = NULL;
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (inodes[i].in_use && strcmp(inodes[i].name, name) == 0) {
            inode = &inodes[i];
            break;
        }
    }

    if (!inode) {
        return NULL; // Archivo no existe
    }

    // Buscar descriptor libre
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (open_files[i].fd < 0) {
            open_files[i].fd = i;
            open_files[i].position = 0;
            open_files[i].inode = inode;
            return &open_files[i];
        }
    }

    return NULL; // No hay descriptores libres
}

/* Leer desde archivo */
int fs_read(FSFile *file, void *buffer, uint32_t size) {
    if (!file || file->fd < 0) {
        return -1;
    }

    FSInode *inode = file->inode;
    uint32_t remaining = inode->size - file->position;
    uint32_t to_read = (size < remaining) ? size : remaining;

    uint8_t *dst = (uint8_t *)buffer;
    for (uint32_t i = 0; i < to_read; i++) {
        uint32_t pos = file->position + i;
        uint32_t block = inode->start_block + (pos / FS_BLOCK_SIZE);
        uint32_t offset = pos % FS_BLOCK_SIZE;
        dst[i] = data_blocks[block][offset];
    }

    file->position += to_read;
    return to_read;
}

/* Cerrar archivo */
void fs_close(FSFile *file) {
    if (file) {
        file->fd = -1;
        file->position = 0;
        file->inode = NULL;
    }
}

/* Listar archivos */
void fs_list(void) {
    uart_puts("=== RAMFS Files ===\r\n");
    int count = 0;
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (inodes[i].in_use) {
            uart_puts("  ");
            uart_puts(inodes[i].name);
            uart_puts(" (");
            uart_putint(inodes[i].size);
            uart_puts(" bytes)\r\n");
            count++;
        }
    }
    uart_puts("Total: ");
    uart_putint(count);
    uart_puts(" files\r\n");
}

/* Obtener tamaño */
int fs_get_size(const char *name) {
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (inodes[i].in_use && strcmp(inodes[i].name, name) == 0) {
            return inodes[i].size;
        }
    }
    return -1;
}

/* Eliminar archivo */
int fs_delete(const char *name) {
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (inodes[i].in_use && strcmp(inodes[i].name, name) == 0) {
            // Liberar bloques
            uint32_t num_blocks = (inodes[i].size + FS_BLOCK_SIZE - 1) / FS_BLOCK_SIZE;
            for (uint32_t j = 0; j < num_blocks; j++) {
                block_used[inodes[i].start_block + j] = 0;
            }

            // Liberar inodo
            inodes[i].in_use = 0;
            return 0;
        }
    }
    return -1;
}
