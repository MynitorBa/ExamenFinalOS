#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>

void Buzzer_Init(void);
void buzzer_play_christmas(void);
void buzzer_play_tone(uint32_t frequency, uint32_t duration_ms);
void buzzer_stop_all(void);

#endif
