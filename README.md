DaOS - Sistema Operativo en Tiempo Real

Autores: José González, José Navichoque y Mynor Barrios.



DaOS es un sistema operativo multitarea híbrido desarrollado para el microcontrolador STM32F446RE, que combina scheduling cooperativo y preemptivo. Incluye múltiples aplicaciones de demostración, sistema de archivos, sincronización de tareas y gestión de recursos. Proyecto final del curso de Sistemas Operativos.



El hardware requerido incluye la placa STM32F446RE (Nucleo-64), un cable USB para programación y depuración, y el ST-Link integrado en la placa Nucleo. El software necesario es STM32CubeIDE versión 1.19.0 o superior y Git para clonar el repositorio.



Para la instalación, primero se debe clonar el repositorio con los comandos:

git clone https://github.com/MynitorBa/ExamenFinalOS.git

cd ExamenFinalOS

Luego, abrir el proyecto en STM32CubeIDE seleccionando la opción “File -> Open Projects from File System”, eligiendo la carpeta del proyecto clonado y haciendo clic en “Finish”. Si el IDE solicita importar configuraciones, se deben aceptar las opciones por defecto.



Para compilar, dentro de STM32CubeIDE se selecciona el proyecto “DaOS”, se hace clic derecho y se elige “Build Project”. Una vez completada la compilación sin errores, el archivo ejecutable se generará en la carpeta Debug/DaOS.elf.



Para programar y ejecutar el sistema, se debe conectar la placa STM32F446RE mediante USB y presionar el botón “Run” o usar la opción “Run -> Debug As -> STM32 C/C++ Application”. El programa se cargará automáticamente en la placa y se iniciará al finalizar la carga. Dependiendo de la configuración, se ejecutarán distintas demostraciones.



La estructura del proyecto incluye varias carpetas:



“Inc” contiene los archivos de cabecera (.h).



“Src” contiene los archivos fuente (.c).



“Debug” almacena los archivos compilados.



“Startup” contiene el código de inicio del microcontrolador.



También incluye los scripts de enlace (\*.ld).



Los componentes principales del sistema son: un scheduler híbrido que combina modos cooperativo y preemptivo; mecanismos de sincronización mediante semáforos binarios y mutex con herencia de prioridad; un sistema de archivos con soporte para RAM File System (ramfs) y FAT File System (fat); y múltiples aplicaciones de demostración, como Snake, Tron, Tanque, Disco y Shell.



Las demostraciones incluidas se pueden activar o desactivar en el archivo main.c. Entre ellas están demo\_scheduler\_rr (scheduler round-robin), demo\_prem (scheduler preemptivo con prioridades) y herenciaprioridad (herencia de prioridad en mutex). Para cambiar entre demostraciones se debe abrir Src/main.c, localizar la función main(), comentar o descomentar las llamadas correspondientes, recompilar y cargar nuevamente. Por ejemplo:

int main(void) {

&nbsp;   init\_system();

&nbsp;   demo\_scheduler\_rr();  // Para modo round-robin

&nbsp;   // demo\_prem();       // Para modo preemptivo

&nbsp;   while(1);

}


El sistema puede operar en tres modos principales. En el modo cooperativo, las tareas ceden voluntariamente el control del CPU mediante la función yield(), ideal para procesos no críticos. En el modo preemptivo, las tareas se interrumpen automáticamente según su prioridad mediante interrupciones del temporizador, garantizando respuesta oportuna a procesos críticos. El modo híbrido combina ambos, asignando tareas cooperativas a baja prioridad y tareas preemptivas a alta prioridad, optimizando eficiencia y tiempo de respuesta.



Para depurar, se utiliza el debugger de STM32CubeIDE. Se puede iniciar el modo de depuración con el icono de “Debug”, lo que pausará el programa en la función main(). Desde allí se pueden usar breakpoints, ejecutar paso a paso o continuar la ejecución. Para monitorear variables en tiempo real, se abren las vistas “Variables” o “Expressions” desde el menú Window -> Show View. Para visualizar la salida de printf() en tiempo real se puede usar SWV (Serial Wire Viewer), habilitándolo desde Run -> Debug Configurations, pestaña “Debugger”, opción “Enable SWV”, y abriendo la vista “SWV ITM Data Console”.



En caso de problemas comunes, si hay errores de compilación se recomienda limpiar el proyecto con “Project -> Clean” y recompilar. Si la placa no es detectada, se debe revisar la conexión USB, instalar los drivers de ST-Link y verificar que el dispositivo aparezca en el administrador de dispositivos. Si el programa no inicia, se puede presionar el botón RESET en la placa, verificar la correcta generación del archivo .elf o intentar ejecutar en modo “Run” en lugar de “Debug”.



Notas técnicas: el sistema está diseñado para el microcontrolador STM32F446RET6, basado en arquitectura ARM Cortex-M4, con una frecuencia configurable de hasta 180 MHz, 512 KB de memoria Flash y 128 KB de RAM. El sistema operativo es híbrido, combinando modos cooperativo y preemptivo.



Este proyecto fue desarrollado como examen final del curso de Sistemas Operativos. Es un proyecto académico destinado exclusivamente para fines educativos.

