/**
 * ============================================================================
 * DaOS v2.0 - RAMFS (RAM File System) - Implementación Completa
 * ============================================================================
 */

#include "ramfs.h"
#include "uart.h"
#include <string.h>

/* ========================================================================== */
/*                          ESTRUCTURAS INTERNAS                              */
/* ========================================================================== */

// Tabla de inodos (metadatos de archivos)
static ramfs_inode_t inodes[RAMFS_MAX_FILES];

// Bloques de datos (memoria real de los archivos)
static uint8_t data_blocks[RAMFS_MAX_BLOCKS][RAMFS_BLOCK_SIZE];

// Bitmap de bloques libres (1 = usado, 0 = libre)
static uint8_t block_bitmap[RAMFS_MAX_BLOCKS];

// Tabla de file descriptors
static ramfs_fd_t fd_table[RAMFS_MAX_OPEN];

/* ========================================================================== */
/*                          FUNCIONES AUXILIARES                              */
/* ========================================================================== */

/**
 * Encontrar inodo libre
 */
static int find_free_inode(void) {
    for (int i = 0; i < RAMFS_MAX_FILES; i++) {
        if (!inodes[i].in_use) {
            return i;
        }
    }
    return -1;
}

/**
 * Encontrar inodo por nombre
 */
static int find_inode_by_name(const char* name) {
    for (int i = 0; i < RAMFS_MAX_FILES; i++) {
        if (inodes[i].in_use && strcmp(inodes[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

/**
 * Contar bloques libres consecutivos a partir de una posición
 */
static int count_free_blocks_from(int start) {
    int count = 0;
    for (int i = start; i < RAMFS_MAX_BLOCKS && !block_bitmap[i]; i++) {
        count++;
    }
    return count;
}

/**
 * Encontrar N bloques libres consecutivos
 * @return Índice del primer bloque o -1 si no hay espacio
 */
static int find_free_blocks(int num_blocks) {
    for (int i = 0; i <= RAMFS_MAX_BLOCKS - num_blocks; i++) {
        if (count_free_blocks_from(i) >= num_blocks) {
            return i;
        }
    }
    return -1;
}

/**
 * Marcar bloques como usados
 */
static void mark_blocks_used(int start, int count) {
    for (int i = 0; i < count; i++) {
        block_bitmap[start + i] = 1;
    }
}

/**
 * Liberar bloques
 */
static void free_blocks(int start, int count) {
    for (int i = 0; i < count; i++) {
        block_bitmap[start + i] = 0;
    }
}

/**
 * Encontrar FD libre
 */
static int find_free_fd(void) {
    for (int i = 0; i < RAMFS_MAX_OPEN; i++) {
        if (!fd_table[i].is_open) {
            return i;
        }
    }
    return -1;
}

/* ========================================================================== */
/*                          INICIALIZACIÓN                                    */
/* ========================================================================== */

void ramfs_init(void) {
    // Limpiar inodos
    for (int i = 0; i < RAMFS_MAX_FILES; i++) {
        inodes[i].in_use = 0;
        inodes[i].name[0] = '\0';
        inodes[i].size = 0;
        inodes[i].capacity = 0;
        inodes[i].start_block = 0;
        inodes[i].num_blocks = 0;
    }

    // Limpiar bitmap de bloques
    for (int i = 0; i < RAMFS_MAX_BLOCKS; i++) {
        block_bitmap[i] = 0;
    }

    // Limpiar file descriptors
    for (int i = 0; i < RAMFS_MAX_OPEN; i++) {
        fd_table[i].is_open = 0;
        fd_table[i].inode_idx = -1;
        fd_table[i].position = 0;
        fd_table[i].mode = 0;
    }

    uart_puts("[RAMFS] Initialized: ");
    uart_putint(RAMFS_MAX_FILES);
    uart_puts(" files, ");
    uart_putint(RAMFS_MAX_BLOCKS);
    uart_puts(" blocks (");
    uart_putint(RAMFS_MAX_BLOCKS * RAMFS_BLOCK_SIZE / 1024);
    uart_puts(" KB)\r\n");
}

/* ========================================================================== */
/*                          OPEN                                              */
/* ========================================================================== */

int ramfs_open(const char* path, int flags) {
    if (!path || strlen(path) >= RAMFS_MAX_FILENAME) {
        return -1;
    }

    // Buscar si el archivo existe
    int inode_idx = find_inode_by_name(path);

    // Si no existe y tiene flag CREAT, crear
    if (inode_idx < 0 && (flags & RAMFS_O_CREAT)) {
        inode_idx = find_free_inode();
        if (inode_idx < 0) {
            return -1; // No hay inodos libres
        }

        // Crear inodo vacío
        strcpy(inodes[inode_idx].name, path);
        inodes[inode_idx].size = 0;
        inodes[inode_idx].capacity = 0;
        inodes[inode_idx].start_block = 0;
        inodes[inode_idx].num_blocks = 0;
        inodes[inode_idx].in_use = 1;
    }

    // Si no existe y NO tiene CREAT, error
    if (inode_idx < 0) {
        return -1;
    }

    // Si tiene TRUNC, vaciar el archivo
    if (flags & RAMFS_O_TRUNC) {
        if (inodes[inode_idx].num_blocks > 0) {
            free_blocks(inodes[inode_idx].start_block, inodes[inode_idx].num_blocks);
        }
        inodes[inode_idx].size = 0;
        inodes[inode_idx].capacity = 0;
        inodes[inode_idx].num_blocks = 0;
    }

    // Buscar FD libre
    int fd = find_free_fd();
    if (fd < 0) {
        return -1; // No hay FDs libres
    }

    // Configurar FD
    fd_table[fd].inode_idx = inode_idx;
    fd_table[fd].is_open = 1;
    fd_table[fd].mode = flags & 0xFF;

    // Posición inicial
    if (flags & RAMFS_O_APPEND) {
        fd_table[fd].position = inodes[inode_idx].size;
    } else {
        fd_table[fd].position = 0;
    }

    return fd;
}

/* ========================================================================== */
/*                          READ                                              */
/* ========================================================================== */

int ramfs_read(int fd, void* buf, size_t count) {
    // Validar FD
    if (fd < 0 || fd >= RAMFS_MAX_OPEN || !fd_table[fd].is_open) {
        return -1;
    }

    // Verificar que tiene permisos de lectura
    int mode = fd_table[fd].mode & 0x03;
    if (mode != RAMFS_O_RDONLY && mode != RAMFS_O_RDWR) {
        return -1; // No tiene permiso de lectura
    }

    int inode_idx = fd_table[fd].inode_idx;
    ramfs_inode_t* inode = &inodes[inode_idx];

    // Calcular cuánto se puede leer
    uint32_t pos = fd_table[fd].position;
    uint32_t remaining = (inode->size > pos) ? (inode->size - pos) : 0;
    uint32_t to_read = (count < remaining) ? count : remaining;

    if (to_read == 0) {
        return 0; // EOF
    }

    // Leer datos bloque por bloque
    uint8_t* dst = (uint8_t*)buf;
    uint32_t bytes_read = 0;

    while (bytes_read < to_read) {
        uint32_t current_pos = pos + bytes_read;
        uint32_t block_idx = current_pos / RAMFS_BLOCK_SIZE;
        uint32_t block_offset = current_pos % RAMFS_BLOCK_SIZE;
        uint32_t physical_block = inode->start_block + block_idx;

        // Leer desde el bloque
        uint32_t bytes_in_block = RAMFS_BLOCK_SIZE - block_offset;
        uint32_t bytes_to_copy = (to_read - bytes_read < bytes_in_block) ?
                                  (to_read - bytes_read) : bytes_in_block;

        memcpy(dst + bytes_read,
               &data_blocks[physical_block][block_offset],
               bytes_to_copy);

        bytes_read += bytes_to_copy;
    }

    // Actualizar posición
    fd_table[fd].position += bytes_read;

    return (int)bytes_read;
}

/* ========================================================================== */
/*                          WRITE                                             */
/* ========================================================================== */

int ramfs_write(int fd, const void* buf, size_t count) {
    // Validar FD
    if (fd < 0 || fd >= RAMFS_MAX_OPEN || !fd_table[fd].is_open) {
        return -1;
    }

    // Verificar que tiene permisos de escritura
    int mode = fd_table[fd].mode & 0x03;
    if (mode != RAMFS_O_WRONLY && mode != RAMFS_O_RDWR) {
        return -1; // No tiene permiso de escritura
    }

    int inode_idx = fd_table[fd].inode_idx;
    ramfs_inode_t* inode = &inodes[inode_idx];

    uint32_t pos = fd_table[fd].position;
    uint32_t new_size = pos + count;

    // Calcular bloques necesarios
    uint32_t blocks_needed = (new_size + RAMFS_BLOCK_SIZE - 1) / RAMFS_BLOCK_SIZE;

    // Si necesitamos más bloques, reasignar
    if (blocks_needed > inode->num_blocks) {
        // Liberar bloques viejos
        if (inode->num_blocks > 0) {
            free_blocks(inode->start_block, inode->num_blocks);
        }

        // Buscar nuevos bloques
        int new_start = find_free_blocks(blocks_needed);
        if (new_start < 0) {
            return -1; // No hay espacio
        }

        // Copiar datos viejos si existen
        if (inode->size > 0) {
            for (uint32_t i = 0; i < inode->size; i++) {
                uint32_t old_block = inode->start_block + (i / RAMFS_BLOCK_SIZE);
                uint32_t new_block = new_start + (i / RAMFS_BLOCK_SIZE);
                uint32_t offset = i % RAMFS_BLOCK_SIZE;
                data_blocks[new_block][offset] = data_blocks[old_block][offset];
            }
        }

        // Asignar nuevos bloques
        mark_blocks_used(new_start, blocks_needed);
        inode->start_block = new_start;
        inode->num_blocks = blocks_needed;
        inode->capacity = blocks_needed * RAMFS_BLOCK_SIZE;
    }

    // Escribir datos
    const uint8_t* src = (const uint8_t*)buf;
    uint32_t bytes_written = 0;

    while (bytes_written < count) {
        uint32_t current_pos = pos + bytes_written;
        uint32_t block_idx = current_pos / RAMFS_BLOCK_SIZE;
        uint32_t block_offset = current_pos % RAMFS_BLOCK_SIZE;
        uint32_t physical_block = inode->start_block + block_idx;

        uint32_t bytes_in_block = RAMFS_BLOCK_SIZE - block_offset;
        uint32_t bytes_to_copy = (count - bytes_written < bytes_in_block) ?
                                  (count - bytes_written) : bytes_in_block;

        memcpy(&data_blocks[physical_block][block_offset],
               src + bytes_written,
               bytes_to_copy);

        bytes_written += bytes_to_copy;
    }

    // Actualizar tamaño si creció
    if (new_size > inode->size) {
        inode->size = new_size;
    }

    // Actualizar posición
    fd_table[fd].position += bytes_written;

    return (int)bytes_written;
}

/* ========================================================================== */
/*                          SEEK                                              */
/* ========================================================================== */

int ramfs_seek(int fd, int offset, int whence) {
    if (fd < 0 || fd >= RAMFS_MAX_OPEN || !fd_table[fd].is_open) {
        return -1;
    }

    int inode_idx = fd_table[fd].inode_idx;
    ramfs_inode_t* inode = &inodes[inode_idx];

    int new_pos = 0;

    switch (whence) {
        case SEEK_SET:
            new_pos = offset;
            break;
        case SEEK_CUR:
            new_pos = fd_table[fd].position + offset;
            break;
        case SEEK_END:
            new_pos = inode->size + offset;
            break;
        default:
            return -1;
    }

    // Validar límites
    if (new_pos < 0) {
        new_pos = 0;
    }
    if ((uint32_t)new_pos > inode->size) {
        new_pos = inode->size;
    }

    fd_table[fd].position = new_pos;
    return new_pos;
}

/* ========================================================================== */
/*                          CLOSE                                             */
/* ========================================================================== */

int ramfs_close(int fd) {
    if (fd < 0 || fd >= RAMFS_MAX_OPEN || !fd_table[fd].is_open) {
        return -1;
    }

    fd_table[fd].is_open = 0;
    fd_table[fd].inode_idx = -1;
    fd_table[fd].position = 0;
    fd_table[fd].mode = 0;

    return 0;
}

/* ========================================================================== */
/*                          API EXTENDIDA                                     */
/* ========================================================================== */

int ramfs_create(const char* name, const void* data, uint32_t size) {
    int fd = ramfs_open(name, RAMFS_O_CREAT | RAMFS_O_WRONLY | RAMFS_O_TRUNC);
    if (fd < 0) return -1;

    int written = ramfs_write(fd, data, size);
    ramfs_close(fd);

    return (written == (int)size) ? 0 : -1;
}

int ramfs_delete(const char* name) {
    int idx = find_inode_by_name(name);
    if (idx < 0) return -1;

    // Liberar bloques
    if (inodes[idx].num_blocks > 0) {
        free_blocks(inodes[idx].start_block, inodes[idx].num_blocks);
    }

    // Liberar inodo
    inodes[idx].in_use = 0;
    return 0;
}

int ramfs_exists(const char* name) {
    return find_inode_by_name(name) >= 0 ? 1 : 0;
}

int ramfs_get_size(const char* name) {
    int idx = find_inode_by_name(name);
    return (idx >= 0) ? (int)inodes[idx].size : -1;
}

int ramfs_listdir(char* out, int max) {
    int used = 0;
    for (int i = 0; i < RAMFS_MAX_FILES; i++) {
        if (inodes[i].in_use) {
            int len = strlen(inodes[i].name);
            if (used + len + 2 >= max) break;

            strcpy(out + used, inodes[i].name);
            used += len;
            out[used++] = '\n';
        }
    }
    out[used] = '\0';
    return used;
}

void ramfs_listdir_uart(void) {
    uart_puts("\r\n📂 RAMFS Files:\r\n");
    uart_puts("==================\r\n");
    int count = 0;
    for (int i = 0; i < RAMFS_MAX_FILES; i++) {
        if (inodes[i].in_use) {
            uart_puts("  ");
            uart_puts(inodes[i].name);
            uart_puts(" (");
            uart_putint(inodes[i].size);
            uart_puts(" bytes, ");
            uart_putint(inodes[i].num_blocks);
            uart_puts(" blocks)\r\n");
            count++;
        }
    }
    uart_puts("------------------\r\n");
    uart_puts("Total: ");
    uart_putint(count);
    uart_puts(" files\r\n\r\n");
}

void ramfs_stats(int* total_files, int* used_blocks, int* free_blocks) {
    int files = 0;
    int blocks = 0;

    for (int i = 0; i < RAMFS_MAX_FILES; i++) {
        if (inodes[i].in_use) {
            files++;
        }
    }

    for (int i = 0; i < RAMFS_MAX_BLOCKS; i++) {
        if (block_bitmap[i]) {
            blocks++;
        }
    }

    if (total_files) *total_files = files;
    if (used_blocks) *used_blocks = blocks;
    if (free_blocks) *free_blocks = RAMFS_MAX_BLOCKS - blocks;
}

int ramfs_truncate(const char* name, uint32_t new_size) {
    int idx = find_inode_by_name(name);
    if (idx < 0) return -1;

    ramfs_inode_t* inode = &inodes[idx];

    if (new_size >= inode->size) {
        return 0; // No hay que truncar
    }

    inode->size = new_size;
    return 0;
}

int ramfs_rename(const char* old_name, const char* new_name) {
    int idx = find_inode_by_name(old_name);
    if (idx < 0) return -1;

    if (find_inode_by_name(new_name) >= 0) {
        return -1; // El nuevo nombre ya existe
    }

    if (strlen(new_name) >= RAMFS_MAX_FILENAME) {
        return -1;
    }

    strcpy(inodes[idx].name, new_name);
    return 0;
}

int ramfs_append(const char* name, const void* data, uint32_t size) {
    int fd = ramfs_open(name, RAMFS_O_RDWR | RAMFS_O_APPEND);
    if (fd < 0) return -1;

    int written = ramfs_write(fd, data, size);
    ramfs_close(fd);

    return (written == (int)size) ? 0 : -1;
}
