################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/hal/currentSink.c \
../src/hal/display.c \
../src/hal/extTrigger.c \
../src/hal/frontPanel.c \
../src/hal/timer.c 

OBJS += \
./src/hal/currentSink.o \
./src/hal/display.o \
./src/hal/extTrigger.o \
./src/hal/frontPanel.o \
./src/hal/timer.o 

C_DEPS += \
./src/hal/currentSink.d \
./src/hal/display.d \
./src/hal/extTrigger.d \
./src/hal/frontPanel.d \
./src/hal/timer.d 


# Each subdirectory must supply rules for building sources it contributes
src/hal/%.o: ../src/hal/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER -I"/home/jan/Projekte/elektronischeLast/Software/electronicLoad/src/hal" -I"/home/jan/Projekte/elektronischeLast/Software/electronicLoad/src" -I"/home/jan/Projekte/elektronischeLast/Software/electronicLoad/src/peripheral" -I"/home/jan/Projekte/elektronischeLast/Software/electronicLoad/src/system" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


