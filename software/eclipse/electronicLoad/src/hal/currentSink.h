/**
 * \file
 * \brief	Analog board hardware abstraction layer header file.
 *
 * 	        This file handles all communication with the analog
 * 	        power sink board.
 */
#ifndef CURRENTSINK_H_
#define CURRENTSINK_H_

#include <stdint.h>
#include "stm32f10x_conf.h"

#define HAL_CS_A_LOW        (GPIOA->BRR = GPIO_Pin_5)
#define HAL_CS_A_HIGH       (GPIOA->BSRR = GPIO_Pin_5)
#define HAL_CS_B_LOW        (GPIOA->BRR = GPIO_Pin_7)
#define HAL_CS_B_HIGH       (GPIOA->BSRR = GPIO_Pin_7)
#define HAL_CLK_LOW         (GPIOA->BRR = GPIO_Pin_0)
#define HAL_CLK_HIGH        (GPIOA->BSRR = GPIO_Pin_0)
#define HAL_DIN_LOW         (GPIOC->BRR = GPIO_Pin_15)
#define HAL_DIN_HIGH        (GPIOC->BSRR = GPIO_Pin_15)
#define HAL_DOUT1           (GPIOC->IDR & GPIO_Pin_14)
#define HAL_DOUT2           (GPIOC->IDR & GPIO_Pin_13)

#define HAL_CS_NONE         0
#define HAL_CS_DAC          1
#define HAL_CS_ADC          2
#define HAL_CS_AVR          3

#define HAL_GPIO_SHUNT_EN2  1
#define HAL_GPIO_SHUNT_EN1  2
#define HAL_GPIO_SHUNTSEL   4
#define HAL_GPIO3           8
#define HAL_GPIO_MODE_A     16
#define HAL_GPIO_MODE_B     32
#define HAL_GPIO_ANALOG_MUX 64

#define HAL_AVR_ADC_TEMP1   6
#define HAL_AVR_ADC_TEMP2   0
#define HAL_AVR_ADC_P15V    4
#define HAL_AVR_ADC_N15V    3
#define HAL_AVR_ADC_P5V     8

#define HAL_TEMP1           0
#define HAL_TEMP2           1

#define HAL_RAIL_P5V        0
#define HAL_RAIL_P15V       1
#define HAL_RAIL_N15V       2

#define HAL_MODE_CC         0
#define HAL_MODE_CV         1
#define HAL_MODE_CR         2
#define HAL_MODE_CP         3

#define HAL_SHUNT_NONE      0
#define HAL_SHUNT_R01       1
#define HAL_SHUNT_1R        2

#define HAL_ADC_CURRENT     0
#define HAL_ADC_VOLTAGE     1

struct {
    uint8_t ADCchannel;
    uint8_t AVRgpio;
} hal;

/**
 * \brief Initialises the current sink hardware
 *
 * Initialises GPIOs used for communication with the
 * analog board.
 */
void hal_currentSinkInit(void);

/**
 * \brief Selects the chip to communicate with
 *
 * \param cs Chip select line that will be driven low
 */
void hal_SetChipSelect(uint8_t cs);

/**
 * \brief Sets one or multiple of the control pins on the analog board
 *
 * Changes are only internal, hal_UpdateAVRGPIOs should be called afterwards
 */
void hal_SetAVRGPIO(uint8_t gpio);

/**
 * \brief Clears one or multiple of the control pins on the analog board
 *
 * Changes are only internal, hal_UpdateAVRGPIOs should be called afterwards
 */
void hal_ClearAVRGPIO(uint8_t gpio);

/**
 * \brief Synchronizes the AVR pins with hal.AVRgpio
 */
void hal_UpdateAVRGPIOs(void);


/**
 * \brief Retrieves an ADC result from the AVR
 *
 * \param channel ADC channel
 * \return 10bit ADC result
 */
uint16_t hal_ReadAVRADC(uint8_t channel);

/**
 * \brief Reads the corresponding ADC channel and does the temperature conversion
 *
 * \param temp Temperature sensor to read from
 * \return temperature in °C
 */
uint8_t hal_ReadTemperature(uint8_t temp);


/**
 * \brief Reads the corresponding ADC channel and does the voltage conversion
 *
 * \param rail Rail to read from
 * \return voltage in mV
 */
int16_t hal_ReadVoltageRail(uint8_t rail);

/**
 * \brief Sends a value to the DAC on the analog board
 *
 * \param dac 16-bit DAC value
 */
void hal_setDAC(uint16_t dac);

/**
 * \brief Controls the fans on the analog board
 *
 * \param en 0: fans disabled, 1: fans enabled
 */
void hal_setFan(uint8_t en);

/**
 * \brief Reads the ADC channel
 *
 * \param nsamples Number of samples (result will be averaged)
 * \return 16-Bit ADC value
 */
uint16_t hal_getADC(uint8_t nsamples);

/**
 * \brief Sets the mux at the op-amp to a specific control mode
 *
 * \param mode New control mode (0=CC, 1=CV, 2=CR, 3=CP)
 */
void hal_SetControlMode(uint8_t mode);

void hal_SelectShunt(uint8_t shunt);

void hal_SelectADCChannel(uint8_t channel);

#endif
