/**
 * \file
 * \brief	Frontpanel hardware abstraction layer source file.
 *
 * 		This file contains all function connected to the
 * 		frontpanel user inputs
 */

#include "frontPanel.h"

const int8_t frontPanel_encoderTable[16] = { 0, 0, -1, 0, 0, 0, 0, 1, 1, 0, 0,
        0, 0, -1, 0, 0 };

/**
 * \brief Initialises the frontpanel hardware
 *
 * Initialises GPIOs used for the buttons and the encoder.
 * Also registers the frontPanelUpdate function with Timer4
 * in a 10ms interval
 * @see hal_frontPanelUpdate
 */
void hal_frontPanelInit(void) {
    GPIO_InitTypeDef gpio;

    // initialize display GPIO
    gpio.GPIO_Speed = GPIO_Speed_50MHz;

    // PORT B
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_15;
    GPIO_Init(GPIOB, &gpio);
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    gpio.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_6
            | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_13;
    GPIO_Init(GPIOB, &gpio);

    // PORT C
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Pin = GPIO_Pin_6;
    GPIO_Init(GPIOC, &gpio);
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    gpio.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_Init(GPIOC, &gpio);

    HAL_FRONTPANEL_SWOUT1_HIGH;
    HAL_FRONTPANEL_SWOUT2_HIGH;
    HAL_FRONTPANEL_SWOUT3_HIGH;

    frontpanel.encoderSensitivity = HAL_DEFAULT_ENCODER_SENSITIVITY;

    // moved function call into load_update to free timer 4
//    timer_SetupPeriodicFunction(4, MS_TO_TICKS(1), hal_frontPanelUpdate, 8);
}

/**
 * \brief Reads and saves the status from the user inputs
 *
 * Should be called regularly (e.g. at a 10ms interval) in a
 * low priority interrupt. Performs a user-button multiplex
 * cycle and handles the encoder counter
 */
void hal_frontPanelUpdate(void) {
    // crude debouncing (only check buttons on every 20th call)
    static uint8_t cnt = 0;
    cnt++;
    if (cnt == 20) {
        cnt = 0;
        uint32_t state = 0;
        // multiplex button matrix

        // first row:
        HAL_FRONTPANEL_SWOUT1_LOW;
        timer_waitus(2);
        if (!HAL_FRONTPANEL_SWIN1)
            state |= HAL_BUTTON_1;
        if (!HAL_FRONTPANEL_SWIN2)
            state |= HAL_BUTTON_2;
        if (!HAL_FRONTPANEL_SWIN3)
            state |= HAL_BUTTON_3;
        if (!HAL_FRONTPANEL_SWIN4)
            state |= HAL_BUTTON_ESC;
        if (!HAL_FRONTPANEL_SWIN5)
            state |= HAL_BUTTON_CC;
        if (!HAL_FRONTPANEL_SWIN6)
            state |= HAL_BUTTON_CV;
        if (!HAL_FRONTPANEL_SWINSOFT)
            state |= HAL_BUTTON_SOFT0;
        // second row:
        HAL_FRONTPANEL_SWOUT1_HIGH;
        HAL_FRONTPANEL_SWOUT2_LOW;
        timer_waitus(2);
        if (!HAL_FRONTPANEL_SWIN1)
            state |= HAL_BUTTON_4;
        if (!HAL_FRONTPANEL_SWIN2)
            state |= HAL_BUTTON_5;
        if (!HAL_FRONTPANEL_SWIN3)
            state |= HAL_BUTTON_6;
        if (!HAL_FRONTPANEL_SWIN4)
            state |= HAL_BUTTON_0;
        if (!HAL_FRONTPANEL_SWIN5)
            state |= HAL_BUTTON_CR;
        if (!HAL_FRONTPANEL_SWIN6)
            state |= HAL_BUTTON_CP;
        if (!HAL_FRONTPANEL_SWINSOFT)
            state |= HAL_BUTTON_SOFT1;

        // third row:
        HAL_FRONTPANEL_SWOUT2_HIGH;
        HAL_FRONTPANEL_SWOUT3_LOW;
        timer_waitus(2);
        if (!HAL_FRONTPANEL_SWIN1)
            state |= HAL_BUTTON_7;
        if (!HAL_FRONTPANEL_SWIN2)
            state |= HAL_BUTTON_8;
        if (!HAL_FRONTPANEL_SWIN3)
            state |= HAL_BUTTON_9;
        if (!HAL_FRONTPANEL_SWIN4)
            state |= HAL_BUTTON_DOT;
        if (!HAL_FRONTPANEL_SWIN5)
            state |= HAL_BUTTON_ENTER;
        if (!HAL_FRONTPANEL_SWIN6)
            state |= HAL_BUTTON_ONOFF;
        if (!HAL_FRONTPANEL_SWINSOFT)
            state |= HAL_BUTTON_SOFT2;

        HAL_FRONTPANEL_SWOUT3_HIGH;

        if (!HAL_FRONTPANEL_ENCSWITCH)
            state |= HAL_BUTTON_ENCODER;

        // all buttons read
        // -> update status
        frontpanel.buttonState = state;
    }

    // read encoder
    // see www.mikrocontroller.net/articles/Drehgeber
    static int8_t last = 0;
    last = (last << 2) & 0x0F;
    if (HAL_FRONTPANEL_ENCA)
        last |= 2;
    if (HAL_FRONTPANEL_ENCB)
        last |= 1;
    frontpanel.encoderCounter -= frontPanel_encoderTable[last];
}

void hal_setEncoderSensitivity(uint8_t n){
    if(!n)
        return;
    frontpanel.encoderSensitivity = n;
}

/**
 * \brief Returns the pattern of pressed buttons
 *
 * \return bitmask representing pressed buttons
 */
uint32_t hal_getButton(void) {
    return frontpanel.buttonState;
}

/**
 * \brief Returns the movement of the encoder
 *
 * \return encoder steps since last call
 */
int32_t hal_getEncoderMovement(void) {
    int32_t buf = frontpanel.encoderCounter / frontpanel.encoderSensitivity;
    frontpanel.encoderCounter -= buf * frontpanel.encoderSensitivity;
    return buf;
}
