#pragma once // Directiva alternativa de guarda
#include <stdint.h> // Incluye tipos de enteros fijos
#include "sched.h"  // Incluye definiciones del planificador (ej: TaskPriority)

/* Descriptor de aplicación */
/** Estructura que describe una aplicación o juego cargable. */
typedef struct {
    const char *name;             /** Nombre de la aplicación. */
    void (*entry_point)(void);    /** Puntero a la función principal de inicio. */
    TaskPriority default_priority;/** Prioridad predeterminada de la tarea principal. */
} AppDescriptor;

/* Registrar aplicaciones disponibles */
/** Inicializa el sistema del cargador (ej: registra todas las AppDescriptors). */
void loader_init(void);

/* Listar aplicaciones disponibles */
/** Muestra o lista todas las aplicaciones registradas. */
void loader_list_apps(void);

/* Cargar y ejecutar aplicación por nombre */
/**
 * Carga una aplicación, crea sus tareas y la inicia.
 * @param app_name Nombre de la aplicación a ejecutar.
 * @param priority Prioridad inicial de la tarea principal.
 * @return 0 en éxito, < 0 en error.
 */
int loader_exec(const char *app_name, TaskPriority priority);

/* Obtener número de aplicaciones */
/**
 * Obtiene el número total de aplicaciones registradas.
 * @return Número de aplicaciones.
 */
int loader_get_app_count(void);

/* Tarea del menú del loader */
/** Función de tarea dedicada a manejar la lógica del menú del cargador. */
void loader_menu_task(void);

/* Mostrar menú en pantalla TFT */
/** Renderiza y muestra la interfaz del menú en la pantalla. */
void loader_show_menu(void);

/* Estado del loader */
/**
 * Verifica si el menú del cargador está activo/visible.
 * @return 1 si el menú está activo, 0 si no.
 */
uint8_t loader_is_menu_active(void);

/**
 * Verifica si actualmente hay un juego o aplicación cargada y ejecutándose.
 * @return 1 si hay un juego/app cargada, 0 si no.
 */
uint8_t loader_is_game_loaded(void);
