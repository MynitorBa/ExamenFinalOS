#ifndef BUZZER_H // Guarda de inclusión para el archivo del zumbador (buzzer)
#define BUZZER_H

#include <stdint.h> // Incluye tipos de enteros fijos

/** Inicializa el subsistema del zumbador. */
void Buzzer_Init(void);

/** Reproduce una melodía navideña pregrabada. */
void buzzer_play_christmas(void);

/**
 * Reproduce un tono de frecuencia y duración específicas.
 * @param frequency Frecuencia del tono en Hz.
 * @param duration_ms Duración del tono en milisegundos.
 */
void buzzer_play_tone(uint32_t frequency, uint32_t duration_ms);

/** Detiene cualquier reproducción de sonido o tono activa. */
void buzzer_stop_all(void);

#endif // BUZZER_H
