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

/**
 * \def MAX_CURRENT
 * maximum settable current in mA
 */
#define MAX_CURRENT		20000

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

struct {
    uint8_t ADCchannel;
    volatile uint16_t rawADC[4];
} hal;

/**
 * \brief Initialises the current sink hardware
 *
 * Initialises GPIOs used for communication with the
 * analog board.
 */
void hal_currentSinkInit(void);

void hal_SetChipSelect(uint8_t cs);

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

#endif
