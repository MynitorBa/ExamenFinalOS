#pragma once
#include <stdint.h>
#include "sched.h"

/* Descriptor de aplicación */
typedef struct {
    const char *name;
    void (*entry_point)(void);
    TaskPriority default_priority;
} AppDescriptor;

/* Registrar aplicaciones disponibles */
void loader_init(void);

/* Listar aplicaciones disponibles */
void loader_list_apps(void);

/* Cargar y ejecutar aplicación por nombre */
int loader_exec(const char *app_name, TaskPriority priority);

/* Obtener número de aplicaciones */
int loader_get_app_count(void);

/* Tarea del menú del loader */
void loader_menu_task(void);

/* Mostrar menú en pantalla TFT */
void loader_show_menu(void);

/* Estado del loader */
uint8_t loader_is_menu_active(void);
uint8_t loader_is_game_loaded(void);
