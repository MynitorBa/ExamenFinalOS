#include "binario.h"
#include "api.h"    // Incluye la API del sistema (daos_uart_puts, daos_task_create, etc.)
#include <string.h> // Incluye funciones de cadena

/* Calcular checksum simple */
/**
 * Calcula un checksum aditivo simple de un bloque de memoria.
 * @param data Puntero al bloque de datos.
 * @param size Tamaño del bloque en bytes.
 * @return Checksum calculado.
 */
uint32_t binario_calcular_checksum(void *data, uint32_t size) {
    uint32_t checksum = 0;
    uint8_t *ptr = (uint8_t *)data;
    for (uint32_t i = 0; i < size; i++) {
        checksum += ptr[i];
    }
    return checksum;
}

/* Validar un binario */
/**
 * Verifica el encabezado del binario (magic number, versión y función 'init').
 * @param binario Puntero a la estructura binario_ejecutable_t.
 * @return 0 si es válido, < 0 si hay error.
 */
int binario_validar(binario_ejecutable_t *binario) {
    if (binario == NULL) {
        daos_uart_puts("[BINARIO] Error: binario NULL\r\n");
        return -1;
    }

    // Verificar magic number (identificador único)
    if (binario->header.magic != BINARIO_MAGIC) {
        daos_uart_puts("[BINARIO] Error: Magic number inválido\r\n");
        return -2;
    }

    // Verificar versión del formato
    if (binario->header.version != BINARIO_VERSION) {
        daos_uart_puts("[BINARIO] Error: Versión incompatible\r\n");
        return -3;
    }

    // Verificar que tenga al menos la función de inicialización
    if (binario->init == NULL) {
        daos_uart_puts("[BINARIO] Error: No tiene función init\r\n");
        return -4;
    }

    // Notificación de validación exitosa
    daos_uart_puts("[BINARIO] Validación OK: ");
    daos_uart_puts(binario->header.nombre);
    daos_uart_puts("\r\n");

    return 0;
}

/* Cargar un binario desde memoria o filesystem */
/**
 * Carga un binario (actualmente sólo valida su estructura en memoria).
 * @param nombre Nombre del binario.
 * @param binario Estructura a validar.
 * @return Resultado de la validación.
 */
int binario_cargar(const char *nombre, binario_ejecutable_t *binario) {
    daos_uart_puts("[BINARIO] Cargando: ");
    daos_uart_puts(nombre);
    daos_uart_puts("\r\n");

    // Lógica para leer desde filesystem (pendiente de implementar si aplica)

    // Validar el binario
    return binario_validar(binario);
}

/* Ejecutar un binario */
/**
 * Ejecuta un binario: llama a init() y crea sus tareas de input, logic y render.
 * @param binario Estructura ejecutable.
 * @return 0 en éxito, -1 si falla la validación.
 */
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

    // Crear la tarea de input (alta prioridad)
    if (binario->input_task != NULL) {
        daos_task_create(binario->input_task, DAOS_PRIO_HIGH);
        daos_uart_puts("[BINARIO] Tarea input creada\r\n");
    }

    // Crear la tarea de lógica (alta prioridad)
    if (binario->logic_task != NULL) {
        daos_task_create(binario->logic_task, DAOS_PRIO_HIGH);
        daos_uart_puts("[BINARIO] Tarea logic creada\r\n");
    }

    // Crear la tarea de renderizado (prioridad normal)
    if (binario->render_task != NULL) {
        daos_task_create(binario->render_task, DAOS_PRIO_NORMAL);
        daos_uart_puts("[BINARIO] Tarea render creada\r\n");
    }

    daos_uart_puts("[BINARIO] Binario ejecutándose\r\n");

    return 0;
}

/* Detener un binario */
/**
 * Detiene la ejecución: llama a cleanup() del binario.
 * @param binario Estructura a detener.
 */
void binario_detener(binario_ejecutable_t *binario) {
    if (binario == NULL) return;

    daos_uart_puts("[BINARIO] Deteniendo: ");
    daos_uart_puts(binario->header.nombre);
    daos_uart_puts("\r\n");

    // Llamar a cleanup si existe (liberación de recursos)
    if (binario->cleanup != NULL) {
        binario->cleanup();
    }

    // Notificación de detención
    daos_uart_puts("[BINARIO] Binario detenido\r\n");
}
