/**
 * ============================================================================
 * DaOS v2.0 - Shell Interactivo (Header)
 * ============================================================================
 * Shell con comandos mágicos para interactuar con el sistema
 * ============================================================================
 */

#ifndef SHELL_H // Guarda de inclusión para el Shell
#define SHELL_H

#pragma once
#include <stdint.h>
#include "sync.h" // Incluye la librería de sincronización

/* Mutex para proteger variables de estado del shell (ej. shell_mode_active) */
/** Mutex para sincronizar el acceso a las variables internas del Shell. */
extern mutex_t shell_sync_mutex;

/* Inicializar el shell */
/** Inicializa estructuras de datos y el intérprete de comandos. */
void shell_init(void);

/* Tarea principal del shell (crear con daos_task_create) */
/** Función de tarea que ejecuta el ciclo principal del intérprete (lee, procesa, ejecuta). */
void shell_task(void);

/* Tarea de monitoreo de botón D14 (doble capa de seguridad) */
/** Tarea dedicada a monitorear un botón para activar/desactivar el Shell. */
void shell_d14_monitor_task(void);

/* Comandos disponibles: */
/** help - Muestra la lista de comandos disponibles. */
/** chlorine - Limpia la pantalla de la terminal. */
/** bewitched - Lista las tareas activas en el planificador. */
/** library - Lista los archivos en el sistema de archivos (RAMFS). */
/** invoke <file> - Muestra el contenido de un archivo. */
/** touch <file> <txt> - Crea un nuevo archivo con contenido. */
/** edit <file> <txt> - Edita/sobrescribe completamente el contenido de un archivo. */
/** append <file> <txt> - Agrega contenido al final de un archivo existente. */
/** remove <file> - Elimina un archivo del sistema de archivos. */
/** rename <old> <new> - Renombra un archivo. */
/** hexdump <file> - Muestra el contenido de un archivo en formato hexadecimal. */
/** mingle <app> - Carga y ejecuta una aplicación registrada por el loader. */
/** fly - Muestra las estadísticas de uso de memoria (RAMFS y/o heap). */
/** hourglass - Muestra el tiempo transcurrido desde el inicio del sistema (uptime). */
/** cauldronVer - Muestra la versión del sistema operativo (DaOS). */
/** revive - Reinicia el sistema (soft reset). */
/** exit - Sale del Shell y regresa al menú principal/loader. */

/* Configuración */
/** Prompt que se muestra al usuario. */
#define SHELL_PROMPT "DaOS> "
/** Tamaño máximo del buffer de entrada de comandos. */
#define SHELL_BUFFER_SIZE 128
/** Número máximo de argumentos soportados por un comando. */
#define SHELL_MAX_ARGS 8
/** Número de comandos guardados en el historial. */
#define SHELL_HISTORY_SIZE 10

#endif /* SHELL_H */
