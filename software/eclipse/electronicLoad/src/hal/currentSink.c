/**
 * \file
 * \brief	Analog board hardware abstraction layer source file.
 *
 * 			This file handles all communication with the analog
 * 			power sink board.
 */
#include "currentSink.h"

void hal_currentSinkInit(void) {
    GPIO_InitTypeDef gpio;

    // initialize SPI interface
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7 | GPIO_Pin_0;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    gpio.GPIO_Pin = GPIO_Pin_15;
    GPIO_Init(GPIOC, &gpio);

    gpio.GPIO_Mode = GPIO_Mode_IPD;
    gpio.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;
    GPIO_Init(GPIOC, &gpio);

    // initialize fan
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    gpio.GPIO_Pin = GPIO_Pin_7;
    GPIO_Init(GPIOC, &gpio);

    hal_setDAC(0);
}

void hal_SetChipSelect(uint8_t cs) {
    switch (cs) {
    default:
    case 0:
        HAL_CS_A_LOW;
        HAL_CS_B_LOW;
        break;
    case 1:
        HAL_CS_A_HIGH;
        HAL_CS_B_LOW;
        break;
    case 2:
        HAL_CS_A_LOW;
        HAL_CS_B_HIGH;
        break;
    case 3:
        HAL_CS_A_HIGH;
        HAL_CS_B_HIGH;
        break;
    }
}

void hal_SetAVRGPIO(uint8_t gpio) {
    hal.AVRgpio |= gpio;
}
void hal_ClearAVRGPIO(uint8_t gpio) {
    hal.AVRgpio &= ~gpio;
}

void hal_UpdateAVRGPIOs(void) {
    static uint8_t oldGPIOs = 0;
    if (hal.AVRgpio != oldGPIOs) {
        // only update when the is actually a pinchange
        oldGPIOs = hal.AVRgpio;
        HAL_CLK_LOW;
        hal_SetChipSelect(HAL_CS_AVR);
        uint8_t word = hal.AVRgpio & 0x7F;
        uint8_t i;
        for (i = 0; i < 8; i++) {
            if (word & 0x80) {
                HAL_DIN_HIGH;
            } else {
                HAL_DIN_LOW;
            }
            // generate clock pulse
            HAL_CLK_HIGH;
            word <<= 1;
            // delay clock (AVR isn't that fast)
            // Minimum high/low pulse length for AVR on 8MHz is 250ns.
            // This function has at least 300ns high/low pulses.
            asm volatile("NOP");
            asm volatile("NOP");
            asm volatile("NOP");
            asm volatile("NOP");
            asm volatile("NOP");
            asm volatile("NOP");
            asm volatile("NOP");
            asm volatile("NOP");
            asm volatile("NOP");
            asm volatile("NOP");
            asm volatile("NOP");
            asm volatile("NOP");
            asm volatile("NOP");
            asm volatile("NOP");
            asm volatile("NOP");
            asm volatile("NOP");
            asm volatile("NOP");
            asm volatile("NOP");
            HAL_CLK_LOW;
        }
        hal_SetChipSelect(HAL_CS_NONE);
    }
}

uint16_t hal_ReadAVRADC(uint8_t channel) {
    HAL_CLK_LOW;
    hal_SetChipSelect(HAL_CS_AVR);
    uint32_t word = 0x00C08080 | ((uint32_t) channel << 16);
    uint8_t i;
    uint32_t rec = 0;
    for (i = 0; i < 24; i++) {
        if (word & 0x00800000) {
            HAL_DIN_HIGH;
        } else {
            HAL_DIN_LOW;
        }
        // generate clock pulse
        HAL_CLK_HIGH;
        rec <<= 1;
        if (HAL_DOUT2)
            rec |= 0x01;

        // delay clock (AVR isn't that fast)
        // Minimum high/low pulse length for AVR on 8MHz is 250ns.
        // This function has at least 300ns high/low pulses.
        asm volatile("NOP");
        asm volatile("NOP");

        word <<= 1;
        HAL_CLK_LOW;
    }
    hal_SetChipSelect(HAL_CS_NONE);
    rec &= 0x000003ff;
    return rec;
}

uint8_t hal_ReadTemperature(uint8_t temp) {
    uint32_t raw;
    if (temp == HAL_TEMP1)
        raw = hal_ReadAVRADC(HAL_AVR_ADC_TEMP1);
    else
        raw = hal_ReadAVRADC(HAL_AVR_ADC_TEMP2);
    // temperature scale is 10mV/°C
    // ADC reference is (at least should be) 5V
    raw *= 125;
    raw /= 256;
    return raw & 0xFF;
}

int16_t hal_ReadVoltageRail(uint8_t rail) {
    int32_t result;
    switch (rail) {
    case HAL_RAIL_P5V:
        result = hal_ReadAVRADC(HAL_AVR_ADC_P5V);
        // ADC measures 1.1 bandgap against 5V reference
        result = 1126400 / result;
        break;
    case HAL_RAIL_P15V:
        result = hal_ReadAVRADC(HAL_AVR_ADC_P5V);
        // ADC measures 15V via a voltage divider with 10k/3k3
        // and a reference voltage of 5V
        result *= 492;
        result /= 25;
        break;
    case HAL_RAIL_N15V:
        result = hal_ReadAVRADC(HAL_AVR_ADC_P5V);
        // calculation assumes that the 15V rail is at 15V
        // ADC measures -15V via a voltage divider between +15V
        // and -15V (6k8/10k) against a reference voltage of 5V
        result = 1829 - result;
        result *= 603;
        result /= 50;
        result = -result;
        break;
    }
    return result;
}

void hal_setDAC(uint16_t dac) {
    HAL_CLK_HIGH;
    hal_SetChipSelect(HAL_CS_DAC);
    // set control bits to 01 (write through)
    uint32_t DACword = 0x00400000;
    DACword += dac << 6;
#ifndef HAL_USE_ASM_SPI
    // slightly slower C code
    // CLK_h is about 360ns (DAC minimum 8ns)
    // CLK_l is about 110ns (DAC minimum 8ns)
    uint8_t i;
    for (i = 0; i < 24; i++) {
        if (DACword & 0x00800000) {
            HAL_DIN_HIGH;
        } else {
            HAL_DIN_LOW;
        }
        // generate clock pulse
        HAL_CLK_LOW;
        HAL_CLK_HIGH;
        DACword <<= 1;
    }
#else
    // slightly faster assembler code (about 2 times faster than C code)
    // CLK_h is about 200ns (DAC minimum 8ns)
    // CLK_l is about 30ns (DAC minimum 8ns)
    uint32_t GPIOA_BSRR = 0x40010810;
    uint32_t GPIOC_BSRR = 0x40011010;
    asm(
            "ldr r5, =0x00800000\n\t" /* load constant to compare DAC word with */
            "lsl r1, %[dinpin], #16\n\t" /* prepare register to clear DIN pin */
            "lsl r2, %[clkpin], #16\n\t" /* prepare register to clear CLK pin */
            "1:\n\t" /* beginning of SPI sending loop */
            "ands r0, r5, %[dac]\n\t" /* currently sending a 1? */
            "ite ne\n\t" /* IF yes */
            "strne %[dinpin], [%[din]]\n\t" /* THEN set DIN high */
            "streq r1, [%[din]]\n\t" /* ELSE set DIN low */
            "lsrs r5, r5, #1\n\t" /* shift 'compare'-bit position by one */
            "str r2, [%[clk]]\n\t" /* set CLK low */
            "str %[clkpin], [%[clk]]\n\t" /* set CLK high */
            "bne 1b\n\t" /* repeat until finished */
            :
            : [dac] "r" (DACword),
            [clk] "r" (GPIOA_BSRR),
            [din] "r" (GPIOC_BSRR),
            [dinpin] "r" (GPIO_Pin_15),
            [clkpin] "r" (GPIO_Pin_0)
            : "r0", "r1", "r2", "r5");
#endif
    hal_SetChipSelect(HAL_CS_NONE);
}

void hal_setFan(uint8_t en) {
    if (en) {
        GPIO_WriteBit(GPIOC, GPIO_Pin_7, Bit_SET);
    } else {
        GPIO_WriteBit(GPIOC, GPIO_Pin_7, Bit_RESET);
    }
}

uint16_t hal_getADC(uint8_t nsamples) {
    HAL_CLK_LOW;
    uint8_t i;
    uint32_t buf = 0;
    for (i = 0; i < nsamples; i++) {
        uint32_t adc = 0;
        hal_SetChipSelect(HAL_CS_ADC);
#ifndef HAL_USE_ASM_SPI
        // slightly slower C code
        // CLK_h is about 110ns (ADC minimum 65ns)
        // CLK_l is about 375ns (DAC minimum 65ns)
        uint8_t p;
        for (p = 0; p < 24; p++) {
            if (HAL_DOUT1)
            adc |= 0x01;
            adc <<= 1;
            HAL_CLK_HIGH;
            HAL_CLK_LOW;
        }
#else
        // slightly faster assembler code (about 1.7 times faster than C code)
        // CLK_h is about 80ns (ADC minimum 65ns)
        // CLK_l is about 180ns (DAC minimum 65ns)
        uint32_t GPIOA_BSRR = 0x40010810;
        uint32_t GPIOC_IDR = 0x40011008;
        asm(
                "ldr %[adc], =0x1\n\t" /* initialize result with 1 to recognize loop end */
                "lsl r2, %[clkpin], #16\n\t" /* prepare register to clear CLK pin */
                "1:\n\t" /* beginning of the SPI loop */
                "lsl %[adc], %[adc], #1\n\t" /* left shift preliminary result to make room for next bit */
                "ldr r0, [%[dout]]\n\t" /* sample DOUT pins */
                "str %[clkpin], [%[clk]]\n\t" /* set CLK high */
                "ands r0, r0, #0x4000\n\t" /* extract specific pin from GPIO byte*/
                "it ne\n\t" /* IF bit is set */
                "addne %[adc], %[adc], #1\n\t" /* set LSB of preliminary result */
                "ands r0, %[adc], #0x01000000\n\t" /* has the first 1 in the result been passed all the way above the MSB? */
                "str r2, [%[clk]]\n\t" /* set CLK low */
                "beq 1b\n\t" /* repeat until bit above MSB is detected */
                "sub %[adc], %[adc], #0x01000000\n\t" /* remove bit above MSB */
                : [adc] "+r" (adc)
                : [dout] "r" (GPIOC_IDR),
                [clk] "r" (GPIOA_BSRR),
                [clkpin] "r" (GPIO_Pin_0)
                :"r2", "r0" );
#endif
        hal_SetChipSelect(HAL_CS_NONE);
        buf += adc;
    }
    return buf /= nsamples;
}

void hal_SetControlMode(uint8_t mode) {
    switch (mode) {
    case HAL_MODE_CC:
        hal_ClearAVRGPIO(HAL_GPIO_MODE_A);
        hal_ClearAVRGPIO(HAL_GPIO_MODE_B);
        break;
    case HAL_MODE_CV:
        hal_SetAVRGPIO(HAL_GPIO_MODE_A);
        hal_ClearAVRGPIO(HAL_GPIO_MODE_B);
        break;
    case HAL_MODE_CR:
        hal_SetAVRGPIO(HAL_GPIO_MODE_A);
        hal_SetAVRGPIO(HAL_GPIO_MODE_B);
        break;
    case HAL_MODE_CP:
        hal_ClearAVRGPIO(HAL_GPIO_MODE_A);
        hal_SetAVRGPIO(HAL_GPIO_MODE_B);
        break;
    }
    hal_UpdateAVRGPIOs();
}

void hal_SelectShunt(uint8_t shunt) {
    switch (shunt) {
    case HAL_SHUNT_NONE:
        hal_SetAVRGPIO(HAL_GPIO_SHUNT_EN1);
        hal_SetAVRGPIO(HAL_GPIO_SHUNT_EN2);
        hal_SetAVRGPIO(HAL_GPIO_SHUNTSEL);
        break;
    case HAL_SHUNT_R01:
        hal_ClearAVRGPIO(HAL_GPIO_SHUNT_EN1);
        hal_SetAVRGPIO(HAL_GPIO_SHUNT_EN2);
        hal_SetAVRGPIO(HAL_GPIO_SHUNTSEL);
        break;
    case HAL_SHUNT_1R:
        hal_SetAVRGPIO(HAL_GPIO_SHUNT_EN1);
        hal_ClearAVRGPIO(HAL_GPIO_SHUNT_EN2);
        hal_ClearAVRGPIO(HAL_GPIO_SHUNTSEL);
        break;
    }
    hal_UpdateAVRGPIOs();
}

void hal_SelectADCChannel(uint8_t channel) {
    switch (channel) {
    case HAL_ADC_CURRENT:
        hal_ClearAVRGPIO(HAL_GPIO_ANALOG_MUX);
        break;
    case HAL_ADC_VOLTAGE:
        hal_SetAVRGPIO(HAL_GPIO_ANALOG_MUX);
        break;
    }
    hal_UpdateAVRGPIOs();
}
