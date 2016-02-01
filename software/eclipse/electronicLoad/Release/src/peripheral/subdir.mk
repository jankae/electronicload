################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/peripheral/misc.c \
../src/peripheral/stm32f10x_adc.c \
../src/peripheral/stm32f10x_bkp.c \
../src/peripheral/stm32f10x_can.c \
../src/peripheral/stm32f10x_cec.c \
../src/peripheral/stm32f10x_crc.c \
../src/peripheral/stm32f10x_dac.c \
../src/peripheral/stm32f10x_dbgmcu.c \
../src/peripheral/stm32f10x_dma.c \
../src/peripheral/stm32f10x_exti.c \
../src/peripheral/stm32f10x_flash.c \
../src/peripheral/stm32f10x_fsmc.c \
../src/peripheral/stm32f10x_gpio.c \
../src/peripheral/stm32f10x_i2c.c \
../src/peripheral/stm32f10x_iwdg.c \
../src/peripheral/stm32f10x_pwr.c \
../src/peripheral/stm32f10x_rcc.c \
../src/peripheral/stm32f10x_rtc.c \
../src/peripheral/stm32f10x_sdio.c \
../src/peripheral/stm32f10x_spi.c \
../src/peripheral/stm32f10x_tim.c \
../src/peripheral/stm32f10x_usart.c \
../src/peripheral/stm32f10x_wwdg.c 

OBJS += \
./src/peripheral/misc.o \
./src/peripheral/stm32f10x_adc.o \
./src/peripheral/stm32f10x_bkp.o \
./src/peripheral/stm32f10x_can.o \
./src/peripheral/stm32f10x_cec.o \
./src/peripheral/stm32f10x_crc.o \
./src/peripheral/stm32f10x_dac.o \
./src/peripheral/stm32f10x_dbgmcu.o \
./src/peripheral/stm32f10x_dma.o \
./src/peripheral/stm32f10x_exti.o \
./src/peripheral/stm32f10x_flash.o \
./src/peripheral/stm32f10x_fsmc.o \
./src/peripheral/stm32f10x_gpio.o \
./src/peripheral/stm32f10x_i2c.o \
./src/peripheral/stm32f10x_iwdg.o \
./src/peripheral/stm32f10x_pwr.o \
./src/peripheral/stm32f10x_rcc.o \
./src/peripheral/stm32f10x_rtc.o \
./src/peripheral/stm32f10x_sdio.o \
./src/peripheral/stm32f10x_spi.o \
./src/peripheral/stm32f10x_tim.o \
./src/peripheral/stm32f10x_usart.o \
./src/peripheral/stm32f10x_wwdg.o 

C_DEPS += \
./src/peripheral/misc.d \
./src/peripheral/stm32f10x_adc.d \
./src/peripheral/stm32f10x_bkp.d \
./src/peripheral/stm32f10x_can.d \
./src/peripheral/stm32f10x_cec.d \
./src/peripheral/stm32f10x_crc.d \
./src/peripheral/stm32f10x_dac.d \
./src/peripheral/stm32f10x_dbgmcu.d \
./src/peripheral/stm32f10x_dma.d \
./src/peripheral/stm32f10x_exti.d \
./src/peripheral/stm32f10x_flash.d \
./src/peripheral/stm32f10x_fsmc.d \
./src/peripheral/stm32f10x_gpio.d \
./src/peripheral/stm32f10x_i2c.d \
./src/peripheral/stm32f10x_iwdg.d \
./src/peripheral/stm32f10x_pwr.d \
./src/peripheral/stm32f10x_rcc.d \
./src/peripheral/stm32f10x_rtc.d \
./src/peripheral/stm32f10x_sdio.d \
./src/peripheral/stm32f10x_spi.d \
./src/peripheral/stm32f10x_tim.d \
./src/peripheral/stm32f10x_usart.d \
./src/peripheral/stm32f10x_wwdg.d 


# Each subdirectory must supply rules for building sources it contributes
src/peripheral/%.o: ../src/peripheral/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER -I"/home/jan/Projekte/elektronischeLast/Software/electronicLoad/src/hal" -I"/home/jan/Projekte/elektronischeLast/Software/electronicLoad/src" -I"/home/jan/Projekte/elektronischeLast/Software/electronicLoad/src/peripheral" -I"/home/jan/Projekte/elektronischeLast/Software/electronicLoad/src/system" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


