################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/Driver/drv_adc.c \
../Src/Driver/drv_critical.c \
../Src/Driver/drv_gpio.c \
../Src/Driver/drv_input.c \
../Src/Driver/drv_led.c \
../Src/Driver/drv_rand.c \
../Src/Driver/drv_seg.c \
../Src/Driver/drv_system.c \
../Src/Driver/drv_tick.c \
../Src/Driver/drv_uart.c \
../Src/Driver/drv_wdg.c 

OBJS += \
./Src/Driver/drv_adc.o \
./Src/Driver/drv_critical.o \
./Src/Driver/drv_gpio.o \
./Src/Driver/drv_input.o \
./Src/Driver/drv_led.o \
./Src/Driver/drv_rand.o \
./Src/Driver/drv_seg.o \
./Src/Driver/drv_system.o \
./Src/Driver/drv_tick.o \
./Src/Driver/drv_uart.o \
./Src/Driver/drv_wdg.o 

C_DEPS += \
./Src/Driver/drv_adc.d \
./Src/Driver/drv_critical.d \
./Src/Driver/drv_gpio.d \
./Src/Driver/drv_input.d \
./Src/Driver/drv_led.d \
./Src/Driver/drv_rand.d \
./Src/Driver/drv_seg.d \
./Src/Driver/drv_system.d \
./Src/Driver/drv_tick.d \
./Src/Driver/drv_uart.d \
./Src/Driver/drv_wdg.d 


# Each subdirectory must supply rules for building sources it contributes
Src/Driver/%.o Src/Driver/%.su Src/Driver/%.cyclo: ../Src/Driver/%.c Src/Driver/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DNUCLEO_F411RE -DSTM32 -DSTM32F4 -DSTM32F411RETx -c -I../Inc -I../../Library/CMSIS/Core/Include -I../../Library/CMSIS-DEVICE-F4/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src-2f-Driver

clean-Src-2f-Driver:
	-$(RM) ./Src/Driver/drv_adc.cyclo ./Src/Driver/drv_adc.d ./Src/Driver/drv_adc.o ./Src/Driver/drv_adc.su ./Src/Driver/drv_critical.cyclo ./Src/Driver/drv_critical.d ./Src/Driver/drv_critical.o ./Src/Driver/drv_critical.su ./Src/Driver/drv_gpio.cyclo ./Src/Driver/drv_gpio.d ./Src/Driver/drv_gpio.o ./Src/Driver/drv_gpio.su ./Src/Driver/drv_input.cyclo ./Src/Driver/drv_input.d ./Src/Driver/drv_input.o ./Src/Driver/drv_input.su ./Src/Driver/drv_led.cyclo ./Src/Driver/drv_led.d ./Src/Driver/drv_led.o ./Src/Driver/drv_led.su ./Src/Driver/drv_rand.cyclo ./Src/Driver/drv_rand.d ./Src/Driver/drv_rand.o ./Src/Driver/drv_rand.su ./Src/Driver/drv_seg.cyclo ./Src/Driver/drv_seg.d ./Src/Driver/drv_seg.o ./Src/Driver/drv_seg.su ./Src/Driver/drv_system.cyclo ./Src/Driver/drv_system.d ./Src/Driver/drv_system.o ./Src/Driver/drv_system.su ./Src/Driver/drv_tick.cyclo ./Src/Driver/drv_tick.d ./Src/Driver/drv_tick.o ./Src/Driver/drv_tick.su ./Src/Driver/drv_uart.cyclo ./Src/Driver/drv_uart.d ./Src/Driver/drv_uart.o ./Src/Driver/drv_uart.su ./Src/Driver/drv_wdg.cyclo ./Src/Driver/drv_wdg.d ./Src/Driver/drv_wdg.o ./Src/Driver/drv_wdg.su

.PHONY: clean-Src-2f-Driver

