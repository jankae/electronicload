/**
 * \file    extTrigger.h
 * \author  Jan Kaeberich
 * \brief   External trigger hardware abstraction layer header file.
 *
 *          This file contains the hardware abstraction for the
 *          external trigger input and output.
 */
#ifndef HAL_EXTTRIGGER_H_
#define HAL_EXTTRIGGER_H_

#include <stdlib.h>
#include "stm32f10x.h"

typedef enum {TRIG_RISING, TRIG_FALLING} trigEdge_t;

struct {
	void (*callback)(trigEdge_t edge);
} trigger;

/**
 * \brief Initializes external trigger hardware
 *
 * Configures GPIOs and sets up external interrupt
 */
void hal_triggerInit(void);

/**
 * \brief Registers a callback function for the external trigger input
 *
 * The registered function will be called on every edge of the
 * external trigger input
 *
 * \param callback Function that will be called
 */
void hal_setTriggerInCallback(void (*callback)(trigEdge_t edge));

/**
 * \brief Sets the external trigger output
 *
 * \param state 1: trigger output high (3,3V), 0: trigger output low (0V)
 */
void hal_setTriggerOut(uint8_t state);

void EXTI4_IRQHandler(void);

#endif
