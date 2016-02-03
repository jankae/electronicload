/**
 * \file
 * \brief	Frontpanel hardware abstraction layer header file.
 *
 * 		This file contains all function connected to the
 * 		frontpanel user inputs
 */

#ifndef FRONTPANEL_H_
#define FRONTPANEL_H_

#include <stdint.h>
#include <stm32f10x_conf.h>
#include "timer.h"

/**
 * \name Macros for the frontpanel switches GPIOs
 * \{
 */
#define HAL_FRONTPANEL_SWOUT1_HIGH  GPIOB->BSRR = GPIO_Pin_12
#define HAL_FRONTPANEL_SWOUT1_LOW   GPIOB->BRR = GPIO_Pin_12
#define HAL_FRONTPANEL_SWOUT2_HIGH  GPIOC->BSRR = GPIO_Pin_6
#define HAL_FRONTPANEL_SWOUT2_LOW   GPIOC->BRR = GPIO_Pin_6
#define HAL_FRONTPANEL_SWOUT3_HIGH  GPIOB->BSRR = GPIO_Pin_15
#define HAL_FRONTPANEL_SWOUT3_LOW   GPIOB->BRR = GPIO_Pin_15

#define HAL_FRONTPANEL_SWIN1        (GPIOB->IDR & GPIO_Pin_9)
#define HAL_FRONTPANEL_SWIN2        (GPIOB->IDR & GPIO_Pin_8)
#define HAL_FRONTPANEL_SWIN3        (GPIOB->IDR & GPIO_Pin_7)
#define HAL_FRONTPANEL_SWIN4        (GPIOB->IDR & GPIO_Pin_6)
#define HAL_FRONTPANEL_SWIN5        (GPIOB->IDR & GPIO_Pin_2)
#define HAL_FRONTPANEL_SWIN6        (GPIOB->IDR & GPIO_Pin_1)

#define HAL_FRONTPANEL_ENCA         (GPIOC->IDR & GPIO_Pin_4)
#define HAL_FRONTPANEL_ENCB         (GPIOC->IDR & GPIO_Pin_5)
#define HAL_FRONTPANEL_ENCSWITCH    (GPIOB->IDR & GPIO_Pin_0)
/** \} */

/**
 * \name Frontpanel Button Codes
 * \{
 */
#define HAL_BUTTON_0		0x00000001
#define HAL_BUTTON_1		0x00000002
#define HAL_BUTTON_2		0x00000004
#define HAL_BUTTON_UP       0x00000004
#define HAL_BUTTON_3		0x00000008
#define HAL_BUTTON_4		0x00000010
#define HAL_BUTTON_LEFT     0x00000010
#define HAL_BUTTON_5		0x00000020
#define HAL_BUTTON_6		0x00000040
#define HAL_BUTTON_RIGHT    0x00000040
#define HAL_BUTTON_7		0x00000080
#define HAL_BUTTON_8		0x00000100
#define HAL_BUTTON_DOWN     0x00000100
#define HAL_BUTTON_9		0x00000200
#define HAL_BUTTON_ESC		0x00000400
#define HAL_BUTTON_DOT		0x00000800
#define HAL_BUTTON_CC		0x00001000
#define HAL_BUTTON_CV		0x00002000
#define HAL_BUTTON_CR		0x00004000
#define HAL_BUTTON_CP		0x00008000
#define HAL_BUTTON_ENTER	0x00010000
#define HAL_BUTTON_ONOFF	0x00020000
#define HAL_BUTTON_ENCODER	0x00040000

#define HAL_BUTTON_ISDIGIT  0x000003ff
/** \} */

struct {
    uint32_t buttonState;
    int32_t encoderCounter;
} frontpanel;

/**
 * \brief Initialises the frontpanel hardware
 *
 * Initialises GPIOs used for the buttons and the encoder.
 * Also registers the frontPanelUpdate function with Timer4
 * in a 10ms interval
 * @see hal_frontPanelUpdate
 */
void hal_frontPanelInit(void);

/**
 * \brief Reads and saves the status from the user inputs
 *
 * Should be called regularly (e.g. at a 10ms interval) in a
 * low priority interrupt. Performs a user-button multiplex
 * cycle and handles the encoder counter
 */
void hal_frontPanelUpdate(void);

/**
 * \brief Returns the pattern of pressed buttons
 *
 * \return Or'ed combination of \ref Button_MaskBits
 */
uint32_t hal_getButton(void);

/**
 * \brief Returns the movement of the encoder
 *
 * \return encoder steps since last call
 */
int32_t hal_getEncoderMovement(void);

#endif
