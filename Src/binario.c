#include "binario.h"
#include "api.h"
#include <string.h>

/* Calcular checksum simple */
uint32_t binario_calcular_checksum(void *data, uint32_t size) {
    uint32_t checksum = 0;
    uint8_t *ptr = (uint8_t *)data;
    for (uint32_t i = 0; i < size; i++) {
        checksum += ptr[i];
    }
    return checksum;
}

/* Validar un binario */
int binario_validar(binario_ejecutable_t *binario) {
    if (binario == NULL) {
        daos_uart_puts("[BINARIO] Error: binario NULL\r\n");
        return -1;
    }

    // Verificar magic number
    if (binario->header.magic != BINARIO_MAGIC) {
        daos_uart_puts("[BINARIO] Error: Magic number inválido\r\n");
        return -2;
    }

    // Verificar versión
    if (binario->header.version != BINARIO_VERSION) {
        daos_uart_puts("[BINARIO] Error: Versión incompatible\r\n");
        return -3;
    }

    // Verificar que tenga al menos init
    if (binario->init == NULL) {
        daos_uart_puts("[BINARIO] Error: No tiene función init\r\n");
        return -4;
    }

    daos_uart_puts("[BINARIO] Validación OK: ");
    daos_uart_puts(binario->header.nombre);
    daos_uart_puts("\r\n");

    return 0;
}

/* Cargar un binario desde memoria o filesystem */
int binario_cargar(const char *nombre, binario_ejecutable_t *binario) {
    daos_uart_puts("[BINARIO] Cargando: ");
    daos_uart_puts(nombre);
    daos_uart_puts("\r\n");

    // Aquí podrías leer desde el filesystem
    // Por ahora, el binario ya está en memoria

    // Validar el binario
    return binario_validar(binario);
}

/* Ejecutar un binario */
int binario_ejecutar(binario_ejecutable_t *binario) {
    if (binario_validar(binario) != 0) {
        return -1;
    }

    daos_uart_puts("[BINARIO] Ejecutando: ");
    daos_uart_puts(binario->header.nombre);
    daos_uart_puts("\r\n");

    // Llamar a la función de inicialización
    if (binario->init != NULL) {
        binario->init();
    }

    // Crear las tareas del binario
    if (binario->input_task != NULL) {
        daos_task_create(binario->input_task, DAOS_PRIO_HIGH);
        daos_uart_puts("[BINARIO] Tarea input creada\r\n");
    }

    if (binario->logic_task != NULL) {
        daos_task_create(binario->logic_task, DAOS_PRIO_HIGH);
        daos_uart_puts("[BINARIO] Tarea logic creada\r\n");
    }

    if (binario->render_task != NULL) {
        daos_task_create(binario->render_task, DAOS_PRIO_NORMAL);
        daos_uart_puts("[BINARIO] Tarea render creada\r\n");
    }

    daos_uart_puts("[BINARIO] Binario ejecutándose\r\n");

    return 0;
}

/* Detener un binario */
void binario_detener(binario_ejecutable_t *binario) {
    if (binario == NULL) return;

    daos_uart_puts("[BINARIO] Deteniendo: ");
    daos_uart_puts(binario->header.nombre);
    daos_uart_puts("\r\n");

    // Llamar a cleanup si existe
    if (binario->cleanup != NULL) {
        binario->cleanup();
    }

    // Aquí podrías eliminar las tareas si el scheduler lo permite
    // Por ahora solo notificamos
    daos_uart_puts("[BINARIO] Binario detenido\r\n");
}
