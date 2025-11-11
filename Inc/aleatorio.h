#ifndef ALEATORIO_H
#define ALEATORIO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Inicializa el generador aleatorio con semilla basada en tiempo del sistema
 * LLAMAR UNA VEZ al inicio del sistema
 */
void aleatorio_init(void);

/**
 * Obtiene un número aleatorio de 32 bits
 * @return Número aleatorio uint32_t
 */
uint32_t aleatorio_obtener(void);

/**
 * Obtiene un número aleatorio en un rango específico [min, max]
 * @param min Valor mínimo (inclusivo)
 * @param max Valor máximo (inclusivo)
 * @return Número entre min y max
 */
uint32_t aleatorio_rango(uint32_t min, uint32_t max);

/**
 * Obtiene un byte aleatorio (0-255)
 * @return Byte aleatorio
 */
uint8_t aleatorio_byte(void);

/**
 * Refresca la semilla con el tiempo actual del sistema
 * Útil para generar diferentes secuencias en cada reset
 */
void aleatorio_refrescar_semilla(void);

#ifdef __cplusplus
}
#endif

#endif // ALEATORIO_H
