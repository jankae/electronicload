
#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <stdint.h>
#include "stm32f10x_conf.h"
#include "displayRoutines.h"

// display control pins
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

// display commands
#define DISPLAY_ON_CMD			0b00111111
#define DISPLAY_OFF_CMD			0b00111110
#define DISPLAY_SET_ADDRESS		0b01000000
#define DISPLAY_SET_PAGE		0b10111000
#define DISPLAY_SET_STARTLINE	0b11000000

void hal_displayInit(void);

void hal_updateDisplay(void);

void hal_DisplaySetDatabus(uint8_t data);
void hal_DisplayCommand(uint8_t command);
void hal_DisplayOn(void);
void hal_DisplayOff(void);
void hal_DisplaySetAddress(uint8_t address);
void hal_DisplaySetPage(uint8_t page);
void hal_DisplaySetStartline(uint8_t startline);
void hal_DisplayWriteData(uint8_t data);

void hal_SelectDisplay1(void);
void hal_SelectDisplay2(void);


#endif
