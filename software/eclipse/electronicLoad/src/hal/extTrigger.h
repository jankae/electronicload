/**
 * \file
 * \brief   External trigger hardware abstraction layer header file.
 *
 *          This file contains the hardware abstraction for the
 *          external trigger input and output.
 */
#ifndef HAL_EXTTRIGGER_H_
#define HAL_EXTTRIGGER_H_

#include <stdlib.h>
#include "stm32f10x.h"

#define EXT_TRIGGER_HIGH        GPIOA->BSRR = GPIO_Pin_6
#define EXT_TRIGGER_LOW         GPIOA->BRR = GPIO_Pin_6

#define EXT_TRIGGER_IN          (GPIOA->IDR & GPIO_Pin_4)

/**
 * \brief Initializes external trigger hardware
 *
 * Configures GPIOs and sets up external interrupt
 */
void hal_triggerInit(void);

/**
 * \brief Sets the external trigger output
 *
 * \param state 1: trigger output high (3,3V), 0: trigger output low (0V)
 */
void hal_setTriggerOut(uint8_t state);

/**
 * \brief reads the status of the trigger input
 *
 * \return 0 if trigger in is low, 1 otherwise
 */
uint8_t hal_getTriggerIn(void);

#endif
