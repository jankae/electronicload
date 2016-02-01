################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/system/core_cm3.c \
../src/system/system_stm32f10x.c 

S_UPPER_SRCS += \
../src/system/startup_stm32f10x_md.S 

OBJS += \
./src/system/core_cm3.o \
./src/system/startup_stm32f10x_md.o \
./src/system/system_stm32f10x.o 

C_DEPS += \
./src/system/core_cm3.d \
./src/system/system_stm32f10x.d 

S_UPPER_DEPS += \
./src/system/startup_stm32f10x_md.d 


# Each subdirectory must supply rules for building sources it contributes
src/system/%.o: ../src/system/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER -I"/home/jan/Projekte/elektronischeLast/Software/electronicLoad/src/hal" -I"/home/jan/Projekte/elektronischeLast/Software/electronicLoad/src" -I"/home/jan/Projekte/elektronischeLast/Software/electronicLoad/src/peripheral" -I"/home/jan/Projekte/elektronischeLast/Software/electronicLoad/src/system" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/system/%.o: ../src/system/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM GNU Assembler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


