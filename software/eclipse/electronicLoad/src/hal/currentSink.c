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

void hal_setDAC(uint16_t dac) {
    HAL_CLK_HIGH;
    hal_SetChipSelect(HAL_CS_DAC);
    // set control bits to 01 (write through)
    uint32_t DACword = 0x00400000 + dac << 6;
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
        uint8_t p;
        hal_SetChipSelect(HAL_CS_ADC);
        for (p = 0; p < 24; p++) {
            if (HAL_DOUT1)
                adc |= 0x01;
            adc <<= 1;
            HAL_CLK_HIGH;
            // TODO might need delay here
            HAL_CLK_LOW;
        }
        hal_SetChipSelect(HAL_CS_NONE);
        buf += adc;
    }
    return buf /= nsamples;
}
