/**
 * ============================================================================
 * DaOS v2.0 - Shell Interactivo (Header)
 * ============================================================================
 * Shell con comandos mágicos para interactuar con el sistema
 * ============================================================================
 */

#ifndef SHELL_H
#define SHELL_H

#pragma once
#include <stdint.h>
#include "sync.h" // Incluir la librería de sincronización

/* Mutex para proteger variables de estado del shell (ej. shell_mode_active) */
extern mutex_t shell_sync_mutex;

/* Inicializar el shell */
void shell_init(void);

/* Tarea principal del shell (crear con daos_task_create) */
void shell_task(void);

/* Tarea de monitoreo de botón D14 (doble capa de seguridad) */
void shell_d14_monitor_task(void);

/* Comandos disponibles:
 *
 * help              - Muestra lista de comandos
 * chlorine          - Limpia la pantalla
 * bewitched         - Lista tareas activas
 * library           - Lista archivos en RAMFS
 * invoke <file>     - Muestra contenido de archivo
 * touch <file> <txt>- Crea archivo con contenido
 * edit <file> <txt> - Edita/sobrescribe archivo
 * append <file> <txt>- Agrega al final del archivo
 * remove <file>     - Elimina archivo
 * rename <old> <new>- Renombra archivo
 * hexdump <file>    - Ver archivo en hexadecimal
 * mingle <app>      - Ejecuta aplicación del loader
 * fly               - Muestra uso de memoria
 * hourglass         - Tiempo desde boot
 * cauldronVer       - Versión del SO
 * revive            - Reinicia el sistema
 * exit              - Salir al menú principal
 */

/* Configuración */
#define SHELL_PROMPT "DaOS> "
#define SHELL_BUFFER_SIZE 128
#define SHELL_MAX_ARGS 8
#define SHELL_HISTORY_SIZE 10

#endif /* SHELL_H */
