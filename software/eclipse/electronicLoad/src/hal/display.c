/**
 * \file
 * \brief   Hardware abstraction layer header file for a KS0108
 *          display controller.
 */
#include "display.h"

/**
 * \brief Initializes display related hardware
 */
void hal_displayInit(void) {
    GPIO_InitTypeDef gpio;

    // initialize display GPIO
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;

    // PORT A
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    gpio.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11
            | GPIO_Pin_12 | GPIO_Pin_15;
    GPIO_Init(GPIOA, &gpio);

    // PORT B
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    gpio.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_11; /* RST temporarely change to PB11) */
    GPIO_Init(GPIOB, &gpio);

    // PORT C
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    gpio.GPIO_Pin = /*GPIO_Pin_9 (was RST wire) |*/ GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
    GPIO_Init(GPIOC, &gpio);

    // PORT D
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
    gpio.GPIO_Pin = GPIO_Pin_2;
    GPIO_Init(GPIOD, &gpio);

    HAL_DISPLAY_RST_HIGH;
}

/**
 * \brief Transfers buffer content to display RAM
 *
 * This function transfers the complete buffer (1kB)
 * to the display. Should be called regularly to achieve
 * constant framerate
 */
void hal_updateDisplay(void) {
    uint8_t page;
    uint8_t x;
    for (page = 0; page < 8; page++) {
        for (x = 0; x < 128; x++) {
            if (x == 0) {
                hal_SelectDisplay1();
                hal_DisplaySetPage(page);
                hal_DisplaySetAddress(x);
            }
            if (x == 64) {
                hal_SelectDisplay2();
                hal_DisplaySetPage(page);
                hal_DisplaySetAddress(x - 64);
            }
            hal_DisplayWriteData(display.buffer[x + page * 128]);
        }
    }

}

/**
 * \brief Applies a data byte to the display data bus
 *
 * Since the display data bus is not mapped to one port,
 * each bit will be set individually in this function
 *
 * \param data Databyte for the display data bus
 */
void hal_DisplaySetDatabus(uint8_t data) {
    if (data & 0x80)
        HAL_DISPLAY_DB7_HIGH;
    else
        HAL_DISPLAY_DB7_LOW;
    if (data & 0x40)
        HAL_DISPLAY_DB6_HIGH;
    else
        HAL_DISPLAY_DB6_LOW;
    if (data & 0x20)
        HAL_DISPLAY_DB5_HIGH;
    else
        HAL_DISPLAY_DB5_LOW;
    if (data & 0x10)
        HAL_DISPLAY_DB4_HIGH;
    else
        HAL_DISPLAY_DB4_LOW;
    if (data & 0x08)
        HAL_DISPLAY_DB3_HIGH;
    else
        HAL_DISPLAY_DB3_LOW;
    if (data & 0x04)
        HAL_DISPLAY_DB2_HIGH;
    else
        HAL_DISPLAY_DB2_LOW;
    if (data & 0x02)
        HAL_DISPLAY_DB1_HIGH;
    else
        HAL_DISPLAY_DB1_LOW;
    if (data & 0x01)
        HAL_DISPLAY_DB0_HIGH;
    else
        HAL_DISPLAY_DB0_LOW;
}

/**
 * \brief Transmits a command to the display
 *
 * \param command Display command
 */
void hal_DisplayCommand(uint8_t command) {
    timer_waitus(20); // TODO adjust to display
    hal_DisplaySetDatabus(command);
    HAL_DISPLAY_DI_LOW;
    HAL_DISPLAY_RW_LOW;
    HAL_DISPLAY_E_HIGH;
    HAL_DISPLAY_E_HIGH;
    timer_waitus(1);
    HAL_DISPLAY_E_LOW;
}

/**
 * \brief Sets the display address pointer
 *
 * \param address RAM address [0-63]
 */
void hal_DisplaySetAddress(uint8_t address) {
    hal_DisplayCommand(DISPLAY_SET_ADDRESS | (address & 0x3F));
}

/**
 * \brief Sets the display page (row)
 *
 * \param page Page number [0-7]
 */
void hal_DisplaySetPage(uint8_t page) {
    hal_DisplayCommand(DISPLAY_SET_PAGE | (page & 0x07));
}

/**
 * \brief Sets the display startline
 *
 * Shifts the display content vertically,
 * could be used for scrolling
 *
 * \param startline Linenumber of the topmost
 *                  line to be displayed
 */
void hal_DisplaySetStartline(uint8_t startline) {
    hal_DisplayCommand(DISPLAY_SET_STARTLINE | (startline & 0x3F));
}

/**
 * \brief Writes one byte of data to the display RAM
 *
 * \param data Databyte to be transmitted
 */
void hal_DisplayWriteData(uint8_t data) {
    timer_waitus(20); // TODO adjust to display
    hal_DisplaySetDatabus(data);
    HAL_DISPLAY_DI_HIGH;
    HAL_DISPLAY_RW_LOW;
    HAL_DISPLAY_E_HIGH;
    HAL_DISPLAY_E_HIGH;
    timer_waitus(1);
    HAL_DISPLAY_E_LOW;
}

/**
 * \brief Selects the left half of the display
 */
void hal_SelectDisplay1(void) {
    HAL_DISPLAY_CS2_LOW;
    HAL_DISPLAY_CS1_HIGH;
}

/**
 * \brief Selects the left half of the display
 */
void hal_SelectDisplay2(void) {
    HAL_DISPLAY_CS1_LOW;
    HAL_DISPLAY_CS2_HIGH;
}
