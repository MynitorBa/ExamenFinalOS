#pragma once // Directiva alternativa de guarda

#include <stdint.h> // Incluye tipos de enteros fijos

/** Inicializa el subsistema del botón singular. */
void button_init(void);

/**
 * Lee el estado actual del botón singular.
 * @return 1 si presionado, 0 si no.
 */
int button_read(void);

/** Actualiza el estado del botón singular (manejo de debounce). */
void button_update(void);

/**
 * Verifica si el botón singular fue presionado una vez desde la última verificación.
 * @return 1 si fue presionado, 0 si no.
 */
int button_was_pressed(void);

/** Inicializa el subsistema de múltiples botones. */
void buttons_init(void);

/**
 * Lee el estado de un botón específico por su índice.
 * @param index Índice del botón.
 * @return 1 si presionado, 0 si no.
 */
int button_read_index(int index);

/**
 * Obtiene el conteo total de presiones detectadas para un botón por su índice.
 * @param index Índice del botón.
 * @return Contador de presiones.
 */
uint32_t button_get_press_count_index(int index);

/**
 * Obtiene el nombre estático del botón (ej: "A") por su índice.
 * @param index Índice del botón.
 * @return Cadena con el nombre.
 */
const char* button_get_name(int index);

/**
 * Obtiene la etiqueta funcional del botón (ej: "Start") por su índice.
 * @param index Índice del botón.
 * @return Cadena con la etiqueta.
 */
const char* button_get_label(int index);

/** Actualiza el estado de todos los botones. */
void buttons_update(void);

/** Resetea todos los contadores de presiones de los botones. */
void buttons_reset_counts(void);

/**
 * Verifica si algún botón está actualmente presionado.
 * @return 1 si al menos uno está presionado, 0 si no.
 */
int button_check_any_pressed(void);
