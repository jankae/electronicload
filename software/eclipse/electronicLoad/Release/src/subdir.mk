################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/calibration.c \
../src/displayRoutines.c \
../src/loadFunctions.c \
../src/main.c \
../src/string.c 

OBJS += \
./src/calibration.o \
./src/displayRoutines.o \
./src/loadFunctions.o \
./src/main.o \
./src/string.o 

C_DEPS += \
./src/calibration.d \
./src/displayRoutines.d \
./src/loadFunctions.d \
./src/main.d \
./src/string.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER -I"/home/jan/Projekte/elektronischeLast/Software/electronicLoad/src/hal" -I"/home/jan/Projekte/elektronischeLast/Software/electronicLoad/src" -I"/home/jan/Projekte/elektronischeLast/Software/electronicLoad/src/peripheral" -I"/home/jan/Projekte/elektronischeLast/Software/electronicLoad/src/system" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


