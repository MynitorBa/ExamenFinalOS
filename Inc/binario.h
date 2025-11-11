#ifndef BINARIO_H // Guarda de inclusión para el archivo de binario
#define BINARIO_H

#include <stdint.h> // Incluye tipos de enteros fijos

/* Magic number para identificar binarios válidos */
#define BINARIO_MAGIC 0x44414F53  // "DAOS" en hex

/* Versión del formato de binario */
#define BINARIO_VERSION 1

/* Tipos de binarios */
typedef enum {
    BINARIO_TIPO_JUEGO = 1,  // Tipo: Juego
    BINARIO_TIPO_APP = 2,    // Tipo: Aplicación
    BINARIO_TIPO_SERVICIO = 3// Tipo: Servicio en segundo plano
} binario_tipo_t;

/* Estructura del header del binario */
typedef struct {
    uint32_t magic;              /** Número mágico DAOS */
    uint32_t version;            /** Versión del formato */
    uint32_t tipo;               /** Tipo de binario */
    char nombre[32];             /** Nombre del binario */
    uint32_t entry_point;        /** Offset al punto de entrada */
    uint32_t size;               /** Tamaño total del binario */
    uint32_t checksum;           /** Checksum simple para validación */
} binario_header_t;

/* Estructura de un binario ejecutable completo */
typedef struct {
    binario_header_t header;     /** Encabezado del binario */
    void (*init)(void);          /** Función de inicialización */
    void (*reset)(void);         /** Función de reset */
    void (*input_task)(void);    /** Tarea para manejo de entrada (input) */
    void (*logic_task)(void);    /** Tarea para la lógica del programa */
    void (*render_task)(void);   /** Tarea para el renderizado/dibujo */
    void (*cleanup)(void);       /** Función de limpieza al terminar */
    uint32_t (*get_state)(void); /** Obtener estado actual del binario */
} binario_ejecutable_t;

/* API del cargador de binarios */

/**
 * Carga un binario desde el sistema de archivos.
 * @param nombre Nombre del archivo.
 * @param binario Estructura de destino.
 * @return 0 en éxito, < 0 en error.
 */
int binario_cargar(const char *nombre, binario_ejecutable_t *binario);

/**
 * Valida el binario cargado (magic, versión, checksum).
 * @param binario Estructura a validar.
 * @return 0 si es válido, < 0 si error.
 */
int binario_validar(binario_ejecutable_t *binario);

/**
 * Ejecuta un binario válido (crea las tareas necesarias).
 * @param binario Estructura a ejecutar.
 * @return 0 en éxito, < 0 en error.
 */
int binario_ejecutar(binario_ejecutable_t *binario);

/**
 * Detiene la ejecución del binario y realiza la limpieza.
 * @param binario Estructura a detener.
 */
void binario_detener(binario_ejecutable_t *binario);

/**
 * Calcula el checksum de un bloque de datos.
 * @param data Puntero a los datos.
 * @param size Tamaño de los datos.
 * @return Checksum calculado.
 */
uint32_t binario_calcular_checksum(void *data, uint32_t size);

#endif // BINARIO_H
