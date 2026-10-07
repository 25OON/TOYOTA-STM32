################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/App/app_game.c \
../Src/App/app_input.c \
../Src/App/app_main.c \
../Src/App/app_render.c \
../Src/App/app_snake.c 

OBJS += \
./Src/App/app_game.o \
./Src/App/app_input.o \
./Src/App/app_main.o \
./Src/App/app_render.o \
./Src/App/app_snake.o 

C_DEPS += \
./Src/App/app_game.d \
./Src/App/app_input.d \
./Src/App/app_main.d \
./Src/App/app_render.d \
./Src/App/app_snake.d 


# Each subdirectory must supply rules for building sources it contributes
Src/App/%.o Src/App/%.su Src/App/%.cyclo: ../Src/App/%.c Src/App/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DNUCLEO_F411RE -DSTM32 -DSTM32F4 -DSTM32F411RETx -c -I../Inc -I../../Library/CMSIS/Core/Include -I../../Library/CMSIS-DEVICE-F4/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src-2f-App

clean-Src-2f-App:
	-$(RM) ./Src/App/app_game.cyclo ./Src/App/app_game.d ./Src/App/app_game.o ./Src/App/app_game.su ./Src/App/app_input.cyclo ./Src/App/app_input.d ./Src/App/app_input.o ./Src/App/app_input.su ./Src/App/app_main.cyclo ./Src/App/app_main.d ./Src/App/app_main.o ./Src/App/app_main.su ./Src/App/app_render.cyclo ./Src/App/app_render.d ./Src/App/app_render.o ./Src/App/app_render.su ./Src/App/app_snake.cyclo ./Src/App/app_snake.d ./Src/App/app_snake.o ./Src/App/app_snake.su

.PHONY: clean-Src-2f-App

