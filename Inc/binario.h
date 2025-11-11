#ifndef BINARIO_H
#define BINARIO_H

#include <stdint.h>

/* Magic number para identificar binarios válidos */
#define BINARIO_MAGIC 0x44414F53  // "DAOS" en hex

/* Versión del formato de binario */
#define BINARIO_VERSION 1

/* Tipos de binarios */
typedef enum {
    BINARIO_TIPO_JUEGO = 1,
    BINARIO_TIPO_APP = 2,
    BINARIO_TIPO_SERVICIO = 3
} binario_tipo_t;

/* Estructura del header del binario */
typedef struct {
    uint32_t magic;              // Número mágico DAOS
    uint32_t version;            // Versión del formato
    uint32_t tipo;               // Tipo de binario
    char nombre[32];             // Nombre del binario
    uint32_t entry_point;        // Offset al punto de entrada
    uint32_t size;               // Tamaño total del binario
    uint32_t checksum;           // Checksum simple
} binario_header_t;

/* Estructura de un binario ejecutable completo */
typedef struct {
    binario_header_t header;
    void (*init)(void);          // Función de inicialización
    void (*reset)(void);         // Función de reset
    void (*input_task)(void);    // Tarea de input
    void (*logic_task)(void);    // Tarea de lógica
    void (*render_task)(void);   // Tarea de renderizado
    void (*cleanup)(void);       // Función de limpieza
    uint32_t (*get_state)(void); // Obtener estado
} binario_ejecutable_t;

/* API del cargador de binarios */
int binario_cargar(const char *nombre, binario_ejecutable_t *binario);
int binario_validar(binario_ejecutable_t *binario);
int binario_ejecutar(binario_ejecutable_t *binario);
void binario_detener(binario_ejecutable_t *binario);
uint32_t binario_calcular_checksum(void *data, uint32_t size);

#endif // BINARIO_H
