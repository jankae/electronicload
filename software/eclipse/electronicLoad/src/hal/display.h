/**
 * \file
 * \brief   Hardware abstraction layer header file for a KS0108
 *          display controller.
 */
#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <stdint.h>
#include "stm32f10x_conf.h"
#include "timer.h"

/**
 * \name Macros for the display port GPIOs
 * \{
 */
#define HAL_DISPLAY_RST_LOW		GPIO_WriteBit(GPIOC, GPIO_Pin_9, Bit_RESET)
#define HAL_DISPLAY_RST_HIGH	GPIO_WriteBit(GPIOC, GPIO_Pin_9, Bit_SET)
#define HAL_DISPLAY_CS1_LOW		GPIO_WriteBit(GPIOA, GPIO_Pin_9, Bit_RESET)
#define HAL_DISPLAY_CS1_HIGH	GPIO_WriteBit(GPIOA, GPIO_Pin_9, Bit_SET)
#define HAL_DISPLAY_CS2_LOW		GPIO_WriteBit(GPIOA, GPIO_Pin_8, Bit_RESET)
#define HAL_DISPLAY_CS2_HIGH	GPIO_WriteBit(GPIOA, GPIO_Pin_8, Bit_SET)
#define HAL_DISPLAY_E_LOW		GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_RESET)
#define HAL_DISPLAY_E_HIGH		GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_SET)
#define HAL_DISPLAY_RW_LOW		GPIO_WriteBit(GPIOB, GPIO_Pin_4, Bit_RESET)
#define HAL_DISPLAY_RW_HIGH		GPIO_WriteBit(GPIOB, GPIO_Pin_4, Bit_SET)
#define HAL_DISPLAY_DI_LOW		GPIO_WriteBit(GPIOB, GPIO_Pin_5, Bit_RESET)
#define HAL_DISPLAY_DI_HIGH		GPIO_WriteBit(GPIOB, GPIO_Pin_5, Bit_SET)

#define HAL_DISPLAY_DB0_LOW		GPIO_WriteBit(GPIOB, GPIO_Pin_3, Bit_RESET)
#define HAL_DISPLAY_DB0_HIGH	GPIO_WriteBit(GPIOB, GPIO_Pin_3, Bit_SET)
#define HAL_DISPLAY_DB1_LOW		GPIO_WriteBit(GPIOA, GPIO_Pin_15, Bit_RESET)
#define HAL_DISPLAY_DB1_HIGH	GPIO_WriteBit(GPIOA, GPIO_Pin_15, Bit_SET)
#define HAL_DISPLAY_DB2_LOW		GPIO_WriteBit(GPIOC, GPIO_Pin_10, Bit_RESET)
#define HAL_DISPLAY_DB2_HIGH	GPIO_WriteBit(GPIOC, GPIO_Pin_10, Bit_SET)
#define HAL_DISPLAY_DB3_LOW		GPIO_WriteBit(GPIOC, GPIO_Pin_11, Bit_RESET)
#define HAL_DISPLAY_DB3_HIGH	GPIO_WriteBit(GPIOC, GPIO_Pin_11, Bit_SET)
#define HAL_DISPLAY_DB4_LOW		GPIO_WriteBit(GPIOC, GPIO_Pin_12, Bit_RESET)
#define HAL_DISPLAY_DB4_HIGH	GPIO_WriteBit(GPIOC, GPIO_Pin_12, Bit_SET)
#define HAL_DISPLAY_DB5_LOW		GPIO_WriteBit(GPIOA, GPIO_Pin_12, Bit_RESET)
#define HAL_DISPLAY_DB5_HIGH	GPIO_WriteBit(GPIOA, GPIO_Pin_12, Bit_SET)
#define HAL_DISPLAY_DB6_LOW		GPIO_WriteBit(GPIOA, GPIO_Pin_11, Bit_RESET)
#define HAL_DISPLAY_DB6_HIGH	GPIO_WriteBit(GPIOA, GPIO_Pin_11, Bit_SET)
#define HAL_DISPLAY_DB7_LOW		GPIO_WriteBit(GPIOA, GPIO_Pin_10, Bit_RESET)
#define HAL_DISPLAY_DB7_HIGH	GPIO_WriteBit(GPIOA, GPIO_Pin_10, Bit_SET)
/** \} */

/**
 * \name Display commands
 * \{
 */
#define DISPLAY_ON_CMD			0b00111111
#define DISPLAY_OFF_CMD			0b00111110
#define DISPLAY_SET_ADDRESS		0b01000000
#define DISPLAY_SET_PAGE		0b10111000
#define DISPLAY_SET_STARTLINE	0b11000000
/** \} */

struct {
    /**
     * \brief Contains the display data
     *
     * Local copy of the display RAM (controller is
     * not able to read from display. Also, it is faster).
     * Each byte represents an 8-bit vertical line. First
     * row (page) is mapped to buffer[0-127], second row
     * to buffer[128-255] ...
     */
    uint8_t buffer[1024];
} display;

/**
 * \brief Initializes display related hardware
 */
void hal_displayInit(void);

/**
 * \brief Transfers buffer content to display RAM
 *
 * This function transfers the complete buffer (1kB)
 * to the display. Should be called regularly to achieve
 * constant framerate
 */
void hal_updateDisplay(void);

/**
 * \brief Applies a data byte to the display data bus
 *
 * Since the display data bus is not mapped to one port,
 * each bit will be set individually in this function
 *
 * \param data Databyte for the display data bus
 */
void hal_DisplaySetDatabus(uint8_t data);

/**
 * \brief Transmits a command to the display
 *
 * \param command Display command
 */
void hal_DisplayCommand(uint8_t command);

/**
 * \brief Sets the display address pointer
 *
 * \param address RAM address [0-63]
 */
void hal_DisplaySetAddress(uint8_t address);

/**
 * \brief Sets the display page (row)
 *
 * \param page Page number [0-7]
 */
void hal_DisplaySetPage(uint8_t page);

/**
 * \brief Sets the display startline
 *
 * Shifts the display content vertically,
 * could be used for scrolling
 *
 * \param startline Linenumber of the topmost
 *                  line to be displayed
 */
void hal_DisplaySetStartline(uint8_t startline);

/**
 * \brief Writes one byte of data to the display RAM
 *
 * \param data Databyte to be transmitted
 */
void hal_DisplayWriteData(uint8_t data);

/**
 * \brief Selects the left half of the display
 */
void hal_SelectDisplay1(void);

/**
 * \brief Selects the left half of the display
 */
void hal_SelectDisplay2(void);

#endif
