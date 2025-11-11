################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/aleatorio.c \
../Src/api.c \
../Src/binario.c \
../Src/button.c \
../Src/buzzer.c \
../Src/context.c \
../Src/demo_prem.c \
../Src/demo_scheduler_rr.c \
../Src/disco.c \
../Src/fat.c \
../Src/fs.c \
../Src/herenciaprioridad.c \
../Src/loader.c \
../Src/main.c \
../Src/pantalla.c \
../Src/ramfs.c \
../Src/reconocedor.c \
../Src/sched.c \
../Src/shell.c \
../Src/snake.c \
../Src/sync.c \
../Src/syscalls.c \
../Src/sysmem.c \
../Src/tanque.c \
../Src/tron.c \
../Src/tron2.c \
../Src/troncancion.c \
../Src/uart.c \
../Src/user_apps.c 

OBJS += \
./Src/aleatorio.o \
./Src/api.o \
./Src/binario.o \
./Src/button.o \
./Src/buzzer.o \
./Src/context.o \
./Src/demo_prem.o \
./Src/demo_scheduler_rr.o \
./Src/disco.o \
./Src/fat.o \
./Src/fs.o \
./Src/herenciaprioridad.o \
./Src/loader.o \
./Src/main.o \
./Src/pantalla.o \
./Src/ramfs.o \
./Src/reconocedor.o \
./Src/sched.o \
./Src/shell.o \
./Src/snake.o \
./Src/sync.o \
./Src/syscalls.o \
./Src/sysmem.o \
./Src/tanque.o \
./Src/tron.o \
./Src/tron2.o \
./Src/troncancion.o \
./Src/uart.o \
./Src/user_apps.o 

C_DEPS += \
./Src/aleatorio.d \
./Src/api.d \
./Src/binario.d \
./Src/button.d \
./Src/buzzer.d \
./Src/context.d \
./Src/demo_prem.d \
./Src/demo_scheduler_rr.d \
./Src/disco.d \
./Src/fat.d \
./Src/fs.d \
./Src/herenciaprioridad.d \
./Src/loader.d \
./Src/main.d \
./Src/pantalla.d \
./Src/ramfs.d \
./Src/reconocedor.d \
./Src/sched.d \
./Src/shell.d \
./Src/snake.d \
./Src/sync.d \
./Src/syscalls.d \
./Src/sysmem.d \
./Src/tanque.d \
./Src/tron.d \
./Src/tron2.d \
./Src/troncancion.d \
./Src/uart.d \
./Src/user_apps.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su Src/%.cyclo: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F4 -DSTM32F446RETx -DNUCLEO_F446RE -c -I../Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/aleatorio.cyclo ./Src/aleatorio.d ./Src/aleatorio.o ./Src/aleatorio.su ./Src/api.cyclo ./Src/api.d ./Src/api.o ./Src/api.su ./Src/binario.cyclo ./Src/binario.d ./Src/binario.o ./Src/binario.su ./Src/button.cyclo ./Src/button.d ./Src/button.o ./Src/button.su ./Src/buzzer.cyclo ./Src/buzzer.d ./Src/buzzer.o ./Src/buzzer.su ./Src/context.cyclo ./Src/context.d ./Src/context.o ./Src/context.su ./Src/demo_prem.cyclo ./Src/demo_prem.d ./Src/demo_prem.o ./Src/demo_prem.su ./Src/demo_scheduler_rr.cyclo ./Src/demo_scheduler_rr.d ./Src/demo_scheduler_rr.o ./Src/demo_scheduler_rr.su ./Src/disco.cyclo ./Src/disco.d ./Src/disco.o ./Src/disco.su ./Src/fat.cyclo ./Src/fat.d ./Src/fat.o ./Src/fat.su ./Src/fs.cyclo ./Src/fs.d ./Src/fs.o ./Src/fs.su ./Src/herenciaprioridad.cyclo ./Src/herenciaprioridad.d ./Src/herenciaprioridad.o ./Src/herenciaprioridad.su ./Src/loader.cyclo ./Src/loader.d ./Src/loader.o ./Src/loader.su ./Src/main.cyclo ./Src/main.d ./Src/main.o ./Src/main.su ./Src/pantalla.cyclo ./Src/pantalla.d ./Src/pantalla.o ./Src/pantalla.su ./Src/ramfs.cyclo ./Src/ramfs.d ./Src/ramfs.o ./Src/ramfs.su ./Src/reconocedor.cyclo ./Src/reconocedor.d ./Src/reconocedor.o ./Src/reconocedor.su ./Src/sched.cyclo ./Src/sched.d ./Src/sched.o ./Src/sched.su ./Src/shell.cyclo ./Src/shell.d ./Src/shell.o ./Src/shell.su ./Src/snake.cyclo ./Src/snake.d ./Src/snake.o ./Src/snake.su ./Src/sync.cyclo ./Src/sync.d ./Src/sync.o ./Src/sync.su ./Src/syscalls.cyclo ./Src/syscalls.d ./Src/syscalls.o ./Src/syscalls.su ./Src/sysmem.cyclo ./Src/sysmem.d ./Src/sysmem.o ./Src/sysmem.su ./Src/tanque.cyclo ./Src/tanque.d ./Src/tanque.o ./Src/tanque.su ./Src/tron.cyclo ./Src/tron.d ./Src/tron.o ./Src/tron.su ./Src/tron2.cyclo ./Src/tron2.d ./Src/tron2.o ./Src/tron2.su ./Src/troncancion.cyclo ./Src/troncancion.d ./Src/troncancion.o ./Src/troncancion.su ./Src/uart.cyclo ./Src/uart.d ./Src/uart.o ./Src/uart.su ./Src/user_apps.cyclo ./Src/user_apps.d ./Src/user_apps.o ./Src/user_apps.su

.PHONY: clean-Src

