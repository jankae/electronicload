/**
 * \file
 * \brief   Hardware abstraction layer header file for uart communication
 */
#ifndef HAL_UART_H_
#define HAL_UART_H_

#include "stm32f10x.h"
#include "stm32f10x_conf.h"

#define UART_BUF_OUT_SIZE       128
#define UART_BUF_IN_SIZE        64

struct {
    uint8_t outputBuffer[UART_BUF_OUT_SIZE];
    volatile uint32_t outReadPos;
    uint32_t outWritePos;
    uint8_t inputBuffer[UART_BUF_IN_SIZE];
    uint32_t inWritePos;
    uint8_t newDataFlag;
} uart;

void uart_Init(void);

void uart_writeByte(uint8_t b);

void uart_writeData(uint8_t *data, uint32_t length);

void uart_writeString(char *s);

uint32_t uart_dataAvailable(void);

void uart_retrieveData(uint8_t *dest);

#endif
