#ifndef ALEATORIO_H
#define ALEATORIO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Inicializa el generador. Llamar una vez al inicio.
 */
void aleatorio_init(void);

/**
 * Obtiene un número aleatorio de 32 bits.
 * @return Número aleatorio uint32_t
 */
uint32_t aleatorio_obtener(void);

/**
 * Obtiene un número aleatorio en un rango específico [min, max] (inclusivo).
 * @param min Valor mínimo
 * @param max Valor máximo
 * @return Número entre min y max
 */
uint32_t aleatorio_rango(uint32_t min, uint32_t max);

/**
 * Obtiene un byte aleatorio (0-255).
 * @return Byte aleatorio
 */
uint8_t aleatorio_byte(void);

/**
 * Refresca la semilla con el tiempo actual.
 */
void aleatorio_refrescar_semilla(void);

#ifdef __cplusplus
}
#endif

#endif // ALEATORIO_H
