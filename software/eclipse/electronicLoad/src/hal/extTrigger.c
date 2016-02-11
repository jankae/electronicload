/**
 * \file
 * \brief   External trigger hardware abstraction layer header file.
 *
 *          This file contains the hardware abstraction for the
 *          external trigger input and output.
 */
#include "extTrigger.h"

/**
 * \brief Initializes external trigger hardware
 *
 * Configures GPIOs and sets up external interrupt
 */
void hal_triggerInit(void) {
    GPIO_InitTypeDef gpio;

    // initialize display GPIO
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    // external trigger out
    gpio.GPIO_Pin = GPIO_Pin_6;
    GPIO_Init(GPIOA, &gpio);
    hal_setTriggerOut(0);
    // external trigger in
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    gpio.GPIO_Pin = GPIO_Pin_4;
    GPIO_Init(GPIOA, &gpio);
}

/**
 * \brief Sets the external trigger output
 *
 * \param state 1: trigger output high (3,3V), 0: trigger output low (0V)
 */
void hal_setTriggerOut(uint8_t state) {
    if (state) {
        EXT_TRIGGER_HIGH;
    } else {
        EXT_TRIGGER_LOW;
    }
}

uint8_t hal_getTriggerIn(void){
    if(EXT_TRIGGER_IN)
        return 1;
    else
        return 0;
}
