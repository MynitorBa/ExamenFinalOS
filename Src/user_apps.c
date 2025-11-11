	/**
	 * ============================================================================
	 * DaOS v2.0 - User Applications (usando API pública)
	 * ============================================================================
	 * Aplicaciones de ejemplo que demuestran el uso de la API de DaOS
	 * ============================================================================
	 */

	#include "api.h"

	/* ============================================================ */
	/*       VARIABLES COMPARTIDAS (DEBEN ESTAR EN MAIN)          */
	/* ============================================================ */

	// NOTA IMPORTANTE: counter_mutex y shared_counter deben estar
	// declarados en main.c como extern para ser compartidos
	extern uint8_t mutex_storage[32];  // Storage del mutex (en main.c)
	extern uint32_t shared_counter;    // Contador compartido (en main.c)

	// Crear puntero al mutex compartido
	static daos_mutex_t app_counter_mutex = NULL;

	/* ============================================================ */
	/*                   APP 1: HELLO WORLD                        */
	/* ============================================================ */

	void user_app_hello(void) {
		static uint32_t count = 0;

		daos_uart_puts(" [USER:hello] Hello from user space! count=");
		daos_uart_putint(count++);
		daos_uart_puts(" @");
		daos_uart_putint(daos_millis());
		daos_uart_puts("\r\n");

		daos_sleep_ms(1000);
	}

	/* ============================================================ */
	/*           APP 2: COUNTER (usa mutex compartido)             */
	/* ============================================================ */

	void user_app_counter(void) {
		static uint32_t local_count = 0;
		static uint8_t first_run = 1;

		// Inicializar puntero al mutex en la primera ejecución
		if (first_run) {
			extern uint8_t mutex_storage[32];
			app_counter_mutex = (daos_mutex_t)mutex_storage;
			first_run = 0;
		}

		// Acceso protegido al recurso compartido
		if (app_counter_mutex != NULL) {
			daos_mutex_lock(app_counter_mutex);
			shared_counter += 100;
			uint32_t snapshot = shared_counter;
			daos_mutex_unlock(app_counter_mutex);

			daos_uart_puts(" [USER:counter] local=");
			daos_uart_putint(local_count++);
			daos_uart_puts(" shared=");
			daos_uart_putint(snapshot);
			daos_uart_puts(" @");
			daos_uart_putint(daos_millis());
			daos_uart_puts("\r\n");
		} else {
			daos_uart_puts(" [USER:counter] ERROR: mutex not initialized\r\n");
		}

		daos_sleep_ms(700);
	}

	/* ============================================================ */
	/*          APP 3: BLINK (monitorea botón)                     */
	/* ============================================================ */

	void user_app_blink(void) {
		static uint32_t count = 0;
		static uint32_t last_btn_count = 0;

		uint32_t current_btn = daos_btn_get_press_count();

		daos_uart_puts(" [USER:blink] tick=");
		daos_uart_putint(count++);
		daos_uart_puts(" BTN=");
		daos_uart_putint(current_btn);

		if (current_btn != last_btn_count) {
			daos_uart_puts(" ** BUTTON PRESSED! **");
			last_btn_count = current_btn;
		}

		daos_uart_puts("\r\n");

		daos_sleep_ms(1500);
	}
