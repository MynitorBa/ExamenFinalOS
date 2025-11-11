/**
 * ============================================================================
 * DaOS v2.0 - Shell Interactivo con EXIT automático al menú
 * ============================================================================
 * Shell que usa ÚNICAMENTE la API pública de DaOS
 * Incluye comandos para crear, leer, escribir y editar archivos
 * EXIT ahora FUERZA el regreso al menú sin botones
 * ============================================================================
 */

#include "shell.h"
#include "api.h"
#include "sync.h"
#include <string.h>
#include <ctype.h>

/* Mutex de sincronización para el shell */
mutex_t shell_sync_mutex;

/* Buffer de entrada */
static char input_buffer[SHELL_BUFFER_SIZE];
static int input_pos = 0;

/* Historial (simple) */
static char history[SHELL_HISTORY_SIZE][SHELL_BUFFER_SIZE];
static int history_count = 0;

/* Flag para controlar la salida del shell */
extern volatile uint8_t shell_mode_active;

/* ============================================================ */
/* UTILIDADES DE ENTRADA                      */
/* ============================================================ */

static char uart_getc_nonblocking(void) {
    volatile uint32_t *USART2_SR = (uint32_t *)0x40004400;
    volatile uint32_t *USART2_DR = (uint32_t *)0x40004404;
    #define USART_SR_RXNE (1 << 5)

    if (*USART2_SR & USART_SR_RXNE) {
        return (char)(*USART2_DR);
    }
    return 0;
}

/* ============================================================ */
/* COMANDOS DEL SHELL                         */
/* ============================================================ */

static void cmd_help(void) {
    daos_uart_puts("\r\n");
    daos_uart_puts("╔══════════════════════════════════════════╗\r\n");
    daos_uart_puts("║  Della and Operating System (DaOS)      ║\r\n");
    daos_uart_puts("╚══════════════════════════════════════════╝\r\n");
    daos_uart_puts("\r\n📁 ARCHIVO COMANDOS:\r\n");
    daos_uart_puts("  library           - Listar archivos\r\n");
    daos_uart_puts("  invoke <file>     - Leer archivo\r\n");
    daos_uart_puts("  touch <file> <txt>- Crear archivo\r\n");
    daos_uart_puts("  edit <file> <txt> - Editar/sobrescribir archivo\r\n");
    daos_uart_puts("  append <file> <txt>- Agregar al final\r\n");
    daos_uart_puts("  remove <file>     - Eliminar archivo\r\n");
    daos_uart_puts("  rename <old> <new>- Renombrar archivo\r\n");
    daos_uart_puts("  hexdump <file>    - Ver en hexadecimal\r\n");
    daos_uart_puts("\r\n⚙️  SISTEMA:\r\n");
    daos_uart_puts("  chlorine          - Limpiar pantalla\r\n");
    daos_uart_puts("  bewitched         - Listar tareas\r\n");
    daos_uart_puts("  mingle <app>      - Ejecutar aplicación\r\n");
    daos_uart_puts("  fly               - Memoria\r\n");
    daos_uart_puts("  hourglass         - Uptime\r\n");
    daos_uart_puts("  cauldronVer       - Versión\r\n");
    daos_uart_puts("  revive            - Reiniciar\r\n");
    daos_uart_puts("  exit              - Salir al menú (automático)\r\n");
    daos_uart_puts("  help              - Esta ayuda\r\n");
    daos_uart_puts("\r\n");
}

static void cmd_chlorine(void) {
    daos_uart_puts("\033[2J\033[H");
    for (int i = 0; i < 50; i++) daos_uart_puts("\r\n");
}

static void cmd_bewitched(void) {
    daos_uart_puts("\r\n📋 Active Tasks:\r\n");
    daos_uart_puts("PID  STATE      PRIO  CPU_TIME\r\n");
    daos_uart_puts("==================================\r\n");

    daos_task_info_t tasks[16];
    int count = daos_get_task_list(tasks, 16);

    for (int i = 0; i < count; i++) {
        daos_uart_puts("  ");
        daos_uart_putint(tasks[i].id);
        daos_uart_puts("  ");

        switch(tasks[i].state) {
            case 0: daos_uart_puts("READY    "); break;
            case 1: daos_uart_puts("BLOCKED  "); break;
            case 2: daos_uart_puts("SUSPENDED"); break;
            default: daos_uart_puts("UNKNOWN  "); break;
        }

        daos_uart_puts("  ");
        daos_uart_putint(tasks[i].priority);
        daos_uart_puts("     ");
        daos_uart_putint(tasks[i].cpu_time);
        daos_uart_puts(" ms\r\n");
    }
    daos_uart_puts("\r\n");
}

static void cmd_library(void) {
    daos_uart_puts("\r\n📚 Library of Files:\r\n");
    daos_uart_puts("==================\r\n");

    char buffer[512];
    int n = daos_listdir(buffer, sizeof(buffer));

    if (n > 0) {
        int count = 0;
        char *line = buffer;
        while (*line) {
            char *end = strchr(line, '\n');
            if (end) *end = '\0';

            daos_uart_puts("  ");
            daos_uart_puts(line);

            int size = daos_get_file_size(line);
            if (size >= 0) {
                daos_uart_puts(" (");
                daos_uart_putint(size);
                daos_uart_puts(" bytes)");
            }
            daos_uart_puts("\r\n");

            count++;
            if (!end) break;
            line = end + 1;
        }
        daos_uart_puts("------------------\r\n");
        daos_uart_puts("Total: ");
        daos_uart_putint(count);
        daos_uart_puts(" files\r\n");
    } else {
        daos_uart_puts("(vacío)\r\n");
    }

    daos_uart_puts("\r\n");
}

static void cmd_invoke(const char* filename) {
    if (!filename || strlen(filename) == 0) {
        daos_uart_puts("\r\n❌ Uso: invoke <archivo>\r\n\r\n");
        return;
    }

    daos_uart_puts("\r\n📖 Leyendo: ");
    daos_uart_puts(filename);
    daos_uart_puts("\r\n");
    daos_uart_puts("------------------\r\n");

    if (!daos_exists(filename)) {
        daos_uart_puts("❌ Archivo no encontrado\r\n\r\n");
        return;
    }

    int fd = daos_open(filename);
    if (fd < 0) {
        daos_uart_puts("❌ Error al abrir\r\n\r\n");
        return;
    }

    char buffer[256];
    int total = 0;
    int n;

    while ((n = daos_read(fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[n] = '\0';
        daos_uart_puts(buffer);
        total += n;
    }

    daos_close(fd);

    daos_uart_puts("\r\n------------------\r\n");
    daos_uart_puts("(");
    daos_uart_putint(total);
    daos_uart_puts(" bytes leídos)\r\n\r\n");
}

static void cmd_touch(const char* args) {
    if (!args || strlen(args) == 0) {
        daos_uart_puts("\r\n❌ Uso: touch <archivo> <contenido>\r\n");
        daos_uart_puts("Ejemplo: touch nota.txt Hola mundo desde RAMFS\r\n\r\n");
        return;
    }

    char filename[64];
    int i = 0;

    while (*args == ' ') args++;
    while (*args && *args != ' ' && i < 63) {
        filename[i++] = *args++;
    }
    filename[i] = '\0';

    if (strlen(filename) == 0) {
        daos_uart_puts("\r\n❌ Nombre vacío\r\n\r\n");
        return;
    }

    while (*args == ' ') args++;
    const char* content = args;

    if (strlen(content) == 0) {
        daos_uart_puts("\r\n❌ Contenido vacío\r\n\r\n");
        return;
    }

    daos_uart_puts("\r\n✨ Creando: ");
    daos_uart_puts(filename);
    daos_uart_puts("\r\n");

    int result = daos_create_file(filename, content, strlen(content));

    if (result == 0) {
        daos_uart_puts("✅ Archivo creado\r\n");
        daos_uart_puts("   Tamaño: ");
        daos_uart_putint(strlen(content));
        daos_uart_puts(" bytes\r\n");
    } else {
        daos_uart_puts("❌ Error (ya existe o sin espacio)\r\n");
    }

    daos_uart_puts("\r\n");
}

static void cmd_edit(const char* args) {
    if (!args || strlen(args) == 0) {
        daos_uart_puts("\r\n❌ Uso: edit <archivo> <nuevo_contenido>\r\n");
        daos_uart_puts("Ejemplo: edit nota.txt Este es el nuevo texto\r\n\r\n");
        return;
    }

    char filename[64];
    int i = 0;

    while (*args == ' ') args++;
    while (*args && *args != ' ' && i < 63) {
        filename[i++] = *args++;
    }
    filename[i] = '\0';

    while (*args == ' ') args++;
    const char* content = args;

    if (strlen(filename) == 0 || strlen(content) == 0) {
        daos_uart_puts("\r\n❌ Faltan argumentos\r\n\r\n");
        return;
    }

    daos_uart_puts("\r\n✏️  Editando: ");
    daos_uart_puts(filename);
    daos_uart_puts("\r\n");

    if (daos_exists(filename)) {
        daos_delete_file(filename);
    }

    int result = daos_create_file(filename, content, strlen(content));

    if (result == 0) {
        daos_uart_puts("✅ Archivo actualizado\r\n");
        daos_uart_puts("   Nuevo tamaño: ");
        daos_uart_putint(strlen(content));
        daos_uart_puts(" bytes\r\n");
    } else {
        daos_uart_puts("❌ Error al editar\r\n");
    }

    daos_uart_puts("\r\n");
}

static void cmd_append(const char* args) {
    if (!args || strlen(args) == 0) {
        daos_uart_puts("\r\n❌ Uso: append <archivo> <texto_a_agregar>\r\n\r\n");
        return;
    }

    char filename[64];
    int i = 0;

    while (*args == ' ') args++;
    while (*args && *args != ' ' && i < 63) {
        filename[i++] = *args++;
    }
    filename[i] = '\0';

    while (*args == ' ') args++;
    const char* new_content = args;

    if (!daos_exists(filename)) {
        daos_uart_puts("\r\n❌ Archivo no existe\r\n\r\n");
        return;
    }

    daos_uart_puts("\r\n➕ Agregando a: ");
    daos_uart_puts(filename);
    daos_uart_puts("\r\n");

    int result = daos_append(filename, new_content, strlen(new_content));

    if (result == 0) {
        daos_uart_puts("✅ Contenido agregado\r\n");
        daos_uart_puts("   Nuevo tamaño: ");
        daos_uart_putint(daos_get_file_size(filename));
        daos_uart_puts(" bytes\r\n");
    } else {
        daos_uart_puts("❌ Error\r\n");
    }

    daos_uart_puts("\r\n");
}

static void cmd_remove(const char* filename) {
    if (!filename || strlen(filename) == 0) {
        daos_uart_puts("\r\n❌ Uso: remove <archivo>\r\n\r\n");
        return;
    }

    daos_uart_puts("\r\n🗑️  Eliminando: ");
    daos_uart_puts(filename);
    daos_uart_puts("\r\n");

    if (!daos_exists(filename)) {
        daos_uart_puts("❌ No existe\r\n\r\n");
        return;
    }

    int result = daos_delete_file(filename);

    if (result == 0) {
        daos_uart_puts("✅ Archivo eliminado\r\n");
    } else {
        daos_uart_puts("❌ Error al eliminar\r\n");
    }

    daos_uart_puts("\r\n");
}

static void cmd_rename(const char* args) {
    if (!args || strlen(args) == 0) {
        daos_uart_puts("\r\n❌ Uso: rename <nombre_actual> <nombre_nuevo>\r\n");
        daos_uart_puts("Ejemplo: rename viejo.txt nuevo.txt\r\n\r\n");
        return;
    }

    char old_name[64];
    char new_name[64];
    int i = 0;

    while (*args == ' ') args++;
    while (*args && *args != ' ' && i < 63) {
        old_name[i++] = *args++;
    }
    old_name[i] = '\0';

    i = 0;
    while (*args == ' ') args++;
    while (*args && *args != ' ' && i < 63) {
        new_name[i++] = *args++;
    }
    new_name[i] = '\0';

    if (strlen(old_name) == 0 || strlen(new_name) == 0) {
        daos_uart_puts("\r\n❌ Debes especificar ambos nombres\r\n");
        daos_uart_puts("Uso: rename <actual> <nuevo>\r\n\r\n");
        return;
    }

    daos_uart_puts("\r\n🔄 Renombrando: ");
    daos_uart_puts(old_name);
    daos_uart_puts(" → ");
    daos_uart_puts(new_name);
    daos_uart_puts("\r\n");

    if (!daos_exists(old_name)) {
        daos_uart_puts("❌ El archivo '");
        daos_uart_puts(old_name);
        daos_uart_puts("' no existe\r\n\r\n");
        return;
    }

    if (daos_exists(new_name)) {
        daos_uart_puts("❌ Ya existe un archivo llamado '");
        daos_uart_puts(new_name);
        daos_uart_puts("'\r\n\r\n");
        return;
    }

    int fd = daos_open(old_name);
    if (fd < 0) {
        daos_uart_puts("❌ Error al abrir archivo original\r\n\r\n");
        return;
    }

    char buffer[1024];
    int size = daos_read(fd, buffer, sizeof(buffer));
    daos_close(fd);

    if (size < 0) {
        daos_uart_puts("❌ Error al leer archivo\r\n\r\n");
        return;
    }

    int result = daos_create_file(new_name, buffer, size);
    if (result != 0) {
        daos_uart_puts("❌ Error al crear archivo con nuevo nombre\r\n\r\n");
        return;
    }

    result = daos_delete_file(old_name);
    if (result != 0) {
        daos_uart_puts("⚠️  Archivo creado pero no se pudo eliminar el original\r\n\r\n");
        return;
    }

    daos_uart_puts("✅ Archivo renombrado exitosamente\r\n\r\n");
}

static void cmd_hexdump(const char* filename) {
    if (!filename || strlen(filename) == 0) {
        daos_uart_puts("\r\n❌ Uso: hexdump <archivo>\r\n\r\n");
        return;
    }

    if (!daos_exists(filename)) {
        daos_uart_puts("\r\n❌ Archivo no existe\r\n\r\n");
        return;
    }

    int fd = daos_open(filename);
    if (fd < 0) {
        daos_uart_puts("\r\n❌ Error al abrir\r\n\r\n");
        return;
    }

    daos_uart_puts("\r\n🔍 Hexdump de: ");
    daos_uart_puts(filename);
    daos_uart_puts("\r\n");
    daos_uart_puts("Offset   Hex                                          ASCII\r\n");
    daos_uart_puts("--------------------------------------------------------------\r\n");

    uint8_t buffer[16];
    int offset = 0;
    int n;

    while ((n = daos_read(fd, buffer, 16)) > 0) {
        daos_uart_putint(offset);
        daos_uart_puts("  ");

        for (int i = 0; i < 16; i++) {
            if (i < n) {
                uint8_t byte = buffer[i];
                char hex[3];
                hex[0] = "0123456789ABCDEF"[byte >> 4];
                hex[1] = "0123456789ABCDEF"[byte & 0x0F];
                hex[2] = '\0';
                daos_uart_puts(hex);
                daos_uart_putc(' ');
            } else {
                daos_uart_puts("   ");
            }
        }

        daos_uart_puts(" ");

        for (int i = 0; i < n; i++) {
            char c = buffer[i];
            if (c >= 32 && c <= 126) {
                daos_uart_putc(c);
            } else {
                daos_uart_putc('.');
            }
        }

        daos_uart_puts("\r\n");
        offset += n;
    }

    daos_close(fd);
    daos_uart_puts("\r\n");
}

static void cmd_mingle(const char* app_name) {
    if (!app_name || strlen(app_name) == 0) {
        daos_uart_puts("\r\n🎮 Aplicaciones disponibles:\r\n");
        daos_loader_list_apps();
        daos_uart_puts("\r\n");
        return;
    }

    daos_uart_puts("\r\n🪄 Ejecutando: ");
    daos_uart_puts(app_name);
    daos_uart_puts("\r\n");

    int result = daos_loader_exec(app_name, DAOS_PRIO_NORMAL);

    if (result == 0) {
        daos_uart_puts("✅ Iniciada\r\n\r\n");
    } else {
        daos_uart_puts("❌ No existe\r\n\r\n");
    }
}

static void cmd_fly(void) {
    daos_uart_puts("\r\n🪽 Memoria:\r\n");
    daos_uart_puts("==================\r\n");

    daos_memory_info_t mem;
    daos_get_memory_info(&mem);

    daos_uart_puts("  RAMFS Files:   ");
    daos_uart_putint(mem.total_files);
    daos_uart_puts("\r\n");

    daos_uart_puts("  Used Blocks:   ");
    daos_uart_putint(mem.used_blocks);
    daos_uart_puts(" / 64\r\n");

    daos_uart_puts("  Free Blocks:   ");
    daos_uart_putint(mem.free_blocks);
    daos_uart_puts("\r\n");

    daos_uart_puts("  Used Space:    ");
    daos_uart_putint(mem.total_kb);
    daos_uart_puts(" KB\r\n");

    daos_uart_puts("  Free Space:    ");
    daos_uart_putint((mem.free_blocks * 256) / 1024);
    daos_uart_puts(" KB\r\n");

    daos_uart_puts("\r\n");
}

static void cmd_hourglass(void) {
    uint32_t ms = daos_millis();
    uint32_t seconds = ms / 1000;
    uint32_t minutes = seconds / 60;
    uint32_t hours = minutes / 60;

    daos_uart_puts("\r\n⏳ Uptime: ");
    daos_uart_putint(hours);
    daos_uart_puts("h ");
    daos_uart_putint(minutes % 60);
    daos_uart_puts("m ");
    daos_uart_putint(seconds % 60);
    daos_uart_puts("s\r\n\r\n");
}

static void cmd_cauldronVer(void) {
    daos_uart_puts("\r\n");
    daos_uart_puts("╔══════════════════════════════════════════╗\r\n");
    daos_uart_puts("║    Della and Operating System - DaOS    ║\r\n");
    daos_uart_puts("║           Version: ");
    daos_uart_puts(daos_get_version());
    daos_uart_puts("               ║\r\n");
    daos_uart_puts("╚══════════════════════════════════════════╝\r\n");
    daos_uart_puts("\r\n✨ Componentes:\r\n");
    daos_uart_puts("  [✓] Scheduler (RR + Aging)\r\n");
    daos_uart_puts("  [✓] Mutex & Semaphores\r\n");
    daos_uart_puts("  [✓] RAMFS Filesystem\r\n");
    daos_uart_puts("  [✓] Application Loader\r\n");
    daos_uart_puts("  [✓] Shell con edición\r\n");
    daos_uart_puts("\r\n📊 Info:\r\n");
    daos_uart_puts("  Apps: ");
    daos_uart_putint(daos_loader_get_app_count());
    daos_uart_puts(" | Uptime: ");
    daos_uart_putint(daos_get_uptime_seconds());
    daos_uart_puts("s\r\n\r\n");
}

static void cmd_revive(void) {
    daos_uart_puts("\r\n💫 Reiniciando...\r\n\r\n");
    daos_sleep_ms(1000);
    volatile uint32_t *AIRCR = (uint32_t *)0xE000ED0C;
    *AIRCR = 0x05FA0004;
    while(1);
}

// ============================================================================
// 🔥 MODIFICADO: EXIT automático sin botones
// ============================================================================
static void cmd_exit(void) {
    daos_uart_puts("\r\n");
    daos_uart_puts("╔═══════════════════════════════════════════════╗\r\n");
    daos_uart_puts("║          SALIENDO DE DELLA OS SHELL           ║\r\n");
    daos_uart_puts("╠═══════════════════════════════════════════════╣\r\n");
    daos_uart_puts("║  ✓ Terminando tareas del shell...            ║\r\n");
    daos_uart_puts("║  ✓ Limpiando recursos...                     ║\r\n");
    daos_uart_puts("║  ✓ Regresando al menú principal...           ║\r\n");
    daos_uart_puts("╚═══════════════════════════════════════════════╝\r\n");
    daos_uart_puts("\r\n");

    // 1️⃣ Desactivar flag del shell
    mutex_lock(&shell_sync_mutex);
    shell_mode_active = 0;
    mutex_unlock(&shell_sync_mutex);

    // 2️⃣ Pequeña pausa para que el mensaje se muestre
    for (volatile int i = 0; i < 1000000; i++);

    // 3️⃣ 🔥 DETENER EL SYSTICK (crítico)
    volatile uint32_t* SYST_CSR = (volatile uint32_t*)0xE000E010;
    *SYST_CSR = 0x00;  // Deshabilitar SysTick

    // 4️⃣ 🔥 FORZAR SALIDA: Matar TODAS las tareas
    extern void sched_kill_all_tasks(void);
    sched_kill_all_tasks();

    daos_uart_puts("[EXIT] All tasks killed, scheduler should stop now\r\n\r\n");

    // 5️⃣ Esta tarea debe terminar aquí
    // NO hacer loop infinito, dejar que la tarea termine naturalmente
}

/* ============================================================ */
/* PARSER DE COMANDOS                         */
/* ============================================================ */

static void parse_and_execute(char* cmd) {
    while (*cmd == ' ') cmd++;
    if (*cmd == '\0') return;

    char* args = strchr(cmd, ' ');
    if (args) {
        *args = '\0';
        args++;
        while (*args == ' ') args++;
    }

    for (char* p = cmd; *p; p++) *p = tolower(*p);

    if (strcmp(cmd, "help") == 0) cmd_help();
    else if (strcmp(cmd, "chlorine") == 0) cmd_chlorine();
    else if (strcmp(cmd, "bewitched") == 0) cmd_bewitched();
    else if (strcmp(cmd, "library") == 0) cmd_library();
    else if (strcmp(cmd, "invoke") == 0) cmd_invoke(args);
    else if (strcmp(cmd, "touch") == 0) cmd_touch(args);
    else if (strcmp(cmd, "edit") == 0) cmd_edit(args);
    else if (strcmp(cmd, "append") == 0) cmd_append(args);
    else if (strcmp(cmd, "remove") == 0) cmd_remove(args);
    else if (strcmp(cmd, "rename") == 0) cmd_rename(args);
    else if (strcmp(cmd, "hexdump") == 0) cmd_hexdump(args);
    else if (strcmp(cmd, "mingle") == 0) cmd_mingle(args);
    else if (strcmp(cmd, "fly") == 0) cmd_fly();
    else if (strcmp(cmd, "hourglass") == 0) cmd_hourglass();
    else if (strcmp(cmd, "cauldronver") == 0) cmd_cauldronVer();
    else if (strcmp(cmd, "revive") == 0) cmd_revive();
    else if (strcmp(cmd, "exit") == 0) cmd_exit();
    else {
        daos_uart_puts("\r\n❌ Comando desconocido: '");
        daos_uart_puts(cmd);
        daos_uart_puts("'\r\n");
        daos_uart_puts("💡 Usa 'help' para ver comandos\r\n\r\n");
    }
}

/* ============================================================ */
/* TAREA PRINCIPAL DEL SHELL                  */
/* ============================================================ */

void shell_task(void) {
    cmd_chlorine();
    cmd_cauldronVer();
    daos_uart_puts("Type 'help' to see available commands\r\n");
    daos_uart_puts("💡 TIP: Press 'exit' to return to menu\r\n\r\n");

    mutex_lock(&shell_sync_mutex);
    uint8_t active_state = shell_mode_active;
    mutex_unlock(&shell_sync_mutex);

    while (active_state) {
        daos_uart_puts(SHELL_PROMPT);
        input_pos = 0;
        memset(input_buffer, 0, SHELL_BUFFER_SIZE);

        while (1) {
            mutex_lock(&shell_sync_mutex);
            active_state = shell_mode_active;
            mutex_unlock(&shell_sync_mutex);

            if (!active_state) {
                break;
            }

            char c = uart_getc_nonblocking();

            if (c == 0) {
                daos_sleep_ms(10);
                continue;
            }

            if (c == '\r' || c == '\n') {
                daos_uart_puts("\r\n");
                input_buffer[input_pos] = '\0';
                if (input_pos > 0) {
                    parse_and_execute(input_buffer);
                }
                break;
            }
            else if (c == '\b' || c == 127) {
                if (input_pos > 0) {
                    input_pos--;
                    daos_uart_puts("\b \b");
                }
            }
            else if (c == 3) {  // Ctrl+C
                daos_uart_puts("^C\r\n");
                break;
            }
            else if (c >= 32 && c <= 126) {
                if (input_pos < SHELL_BUFFER_SIZE - 1) {
                    input_buffer[input_pos++] = c;
                    daos_uart_putc(c);
                }
            }
        }

        mutex_lock(&shell_sync_mutex);
        active_state = shell_mode_active;
        mutex_unlock(&shell_sync_mutex);

        if (!active_state) {
            break;
        }
    }

    daos_uart_puts("\r\n[SHELL] Task exiting...\r\n");
}

void shell_init(void) {
    mutex_init(&shell_sync_mutex);
    memset(history, 0, sizeof(history));
    history_count = 0;
    daos_uart_puts("[SHELL] Initialized with file editing support\r\n");
}
