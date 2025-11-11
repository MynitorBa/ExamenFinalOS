#include "loader.h"
#include "fs.h"
#include "uart.h"
#include <string.h>

/* Aplicaciones de usuario (definidas externamente) */
extern void user_app_hello(void);
extern void user_app_counter(void);
extern void user_app_blink(void);

/* Tabla de aplicaciones */
static AppDescriptor app_table[] = {
{"hello", user_app_hello, PRIO_NORMAL},
{"counter", user_app_counter, PRIO_LOW},
{"blink", user_app_blink, PRIO_NORMAL},
};

static const int num_apps = sizeof(app_table) / sizeof(AppDescriptor);

void loader_init(void) {
uart_puts("[LOADER] Initialized with ");
uart_putint(num_apps);
uart_puts(" applications\r\n");
}

void loader_list_apps(void) {
uart_puts("\r\n=== Available Applications ===\r\n");
for (int i = 0; i < num_apps; i++) {
uart_puts(" [");
uart_putint(i);
uart_puts("] ");
uart_puts(app_table[i].name);
uart_puts(" (prio: ");
uart_putint(app_table[i].default_priority);
uart_puts(")\r\n");
}
}

int loader_exec(const char *app_name, TaskPriority priority) {
uart_puts("[LOADER] Loading app: ");
uart_puts(app_name);
uart_puts("\r\n");

// Buscar aplicación
for (int i = 0; i < num_apps; i++) {
if (strcmp(app_table[i].name, app_name) == 0) {
uart_puts("[LOADER] Found app, creating task...\r\n");

// Crear tarea con la función
task_create(app_table[i].entry_point, priority);

uart_puts("[LOADER] Task created successfully\r\n");
return 0;
}
}

uart_puts("[LOADER] Error: App not found\r\n");
return -1;
}

int loader_get_app_count(void) {
return num_apps;
}
