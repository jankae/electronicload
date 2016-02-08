/**
 * \file
 * \brief	Analog board hardware abstraction layer source file.
 *
 * 			This file handles all communication with the analog
 * 			power sink board.
 */
#include "currentSink.h"

/**
 * \brief Initialises the current sink hardware
 *
 * Initialises GPIOs used for communication with the
 * analog board. Configures ADC and DMA to continuously
 * sample and store all four channels. Sets ranges to
 * default values (all low).
 */
void hal_currentSinkInit(void) {
    GPIO_InitTypeDef gpio;

    // initialize range switches
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Pin = GPIO_Pin_0;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &gpio);
    GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_SET);
    gpio.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOC, &gpio);
    GPIO_WriteBit(GPIOC, GPIO_Pin_14, Bit_RESET);
    GPIO_WriteBit(GPIOC, GPIO_Pin_15, Bit_SET);

    hal.currentRange = RANGE_LOW;
    hal.voltageRange = RANGE_LOW;
    hal.setRange = RANGE_LOW;

    // initialize fan
    gpio.GPIO_Pin = GPIO_Pin_7;
    GPIO_Init(GPIOC, &gpio);

    // initialize DAC
    gpio.GPIO_Pin = GPIO_Pin_13;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &gpio);
    GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
    gpio.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
    GPIO_Init(GPIOA, &gpio);
    GPIO_WriteBit(GPIOA, GPIO_Pin_7, Bit_RESET);
    GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_RESET);

    hal_setDAC(0);

    // intialize ADC
    // configure pins
    gpio.GPIO_Mode = GPIO_Mode_AIN;
    gpio.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOC, &gpio);

    // setup DMA
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    DMA_InitTypeDef dma;
    dma.DMA_BufferSize = 4;
    dma.DMA_DIR = DMA_DIR_PeripheralSRC;
    dma.DMA_M2M = DMA_M2M_Disable;
    dma.DMA_MemoryBaseAddr = (uint32_t) &hal.rawADC;
    dma.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    dma.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dma.DMA_Mode = DMA_Mode_Circular;
    dma.DMA_PeripheralBaseAddr = (uint32_t) &ADC1->DR;
    dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dma.DMA_Priority = DMA_Priority_High;
    DMA_Init(DMA1_Channel1, &dma);
    DMA_Cmd(DMA1_Channel1, ENABLE);

    // setup ADC1
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    ADC_InitTypeDef adc;
    adc.ADC_Mode = ADC_Mode_Independent;
    adc.ADC_ContinuousConvMode = ENABLE;
    adc.ADC_ScanConvMode = ENABLE;
    adc.ADC_DataAlign = ADC_DataAlign_Right;
    adc.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    adc.ADC_NbrOfChannel = 4;
    ADC_Init(ADC1, &adc);

    // configure channels
    ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_28Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 2, ADC_SampleTime_28Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 3, ADC_SampleTime_28Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 4, ADC_SampleTime_28Cycles5);

    ADC_DMACmd(ADC1, ENABLE);
    ADC_Cmd(ADC1, ENABLE);
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1))
        ;
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1))
        ;
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

/**
 * \brief Sends a value to the DAC on the analog board
 *
 * \param dac 12-bit DAC value (1LSB equals 1mV)
 */
void hal_setDAC(uint16_t dac) {
    dac &= 0x0FFF;
    uint8_t i;
    for (i = 0; i < 12; i++) {
        if (dac & 0x0800) {
            HAL_DAC_DIN_HIGH;
        } else {
            HAL_DAC_DIN_LOW;
        }
        // generate clock pulse
        HAL_DAC_CLK_LOW;
        HAL_DAC_CLK_HIGH;
        dac <<= 1;
    }
    // generate load pulse
    HAL_DAC_LOAD_LOW;
    HAL_DAC_LOAD_HIGH;
}

/**
 * \brief Sets the current set gain
 *
 * \param en 0: gain is set to 1, 1: gain is set to 10
 */
void hal_setGain(uint8_t en) {
    if (en) {
        GPIO_WriteBit(GPIOC, GPIO_Pin_14, Bit_SET);
        hal.setRange = RANGE_HIGH;
    } else {
        GPIO_WriteBit(GPIOC, GPIO_Pin_14, Bit_RESET);
        hal.setRange = RANGE_LOW;
    }
}

/**
 * \brief Sets the voltage measurement gain
 *
 * \param en 0: gain is set to 1, 1: gain is set to 10
 */
void hal_setVoltageGain(uint8_t en) {
    if (en) {
        GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_SET);
        hal.voltageRange = RANGE_LOW;
    } else {
        GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_RESET);
        hal.voltageRange = RANGE_HIGH;
    }
}

/**
 * \brief Sets the current measurement gain
 *
 * \param en 0: gain is set to 1, 1: gain is set to 10
 */
void hal_setCurrentGain(uint8_t en) {
    if (en) {
        GPIO_WriteBit(GPIOC, GPIO_Pin_15, Bit_SET);
        hal.currentRange = RANGE_LOW;
    } else {
        GPIO_WriteBit(GPIOC, GPIO_Pin_15, Bit_RESET);
        hal.currentRange = RANGE_HIGH;
    }
}

/**
 * \brief Controls the fans on the analog board
 *
 * \param en 0: fans disabled, 1: fans enabled
 */
void hal_setFan(uint8_t en) {
    if (en) {
        GPIO_WriteBit(GPIOC, GPIO_Pin_7, Bit_SET);
    } else {
        GPIO_WriteBit(GPIOC, GPIO_Pin_7, Bit_RESET);
    }
}

/**
 * \brief Reads an ADC channel
 *
 * \param channel Channel selection. Can be any value of \ref rawADC_indices
 * \param nsamples Number of samples (result will be averaged)
 * \return 12-Bit ADC value
 */
uint16_t hal_getADC(uint8_t channel, uint8_t nsamples) {
    if (nsamples <= 1) {
        return hal.rawADC[channel];
    } else {
        uint32_t buf = hal.rawADC[channel];
        uint8_t i;
        for (i = 1; i < nsamples; i++) {
            // wait for next conversion to be finished
            while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC))
                ;
            buf += hal.rawADC[channel];
        }
        return buf /= nsamples;
    }

}
