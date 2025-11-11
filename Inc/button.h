#pragma once
#include <stdint.h>

void button_init(void);
int button_read(void);
void button_update(void);
int button_was_pressed(void);

void buttons_init(void);
int button_read_index(int index);
uint32_t button_get_press_count_index(int index);
const char* button_get_name(int index);
const char* button_get_label(int index);
void buttons_update(void);
void buttons_reset_counts(void);
int button_check_any_pressed(void);
