/**
 * \file
 * \brief   Hardware abstraction layer source file for uart communication
 */
#include "uart.h"

void uart_Init(void) {
    GPIO_InitTypeDef gpio;
    USART_InitTypeDef usart;
    NVIC_InitTypeDef nvic;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    // uart pins are PA2 and PA3
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    gpio.GPIO_Pin = GPIO_Pin_2;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);
    gpio.GPIO_Pin = GPIO_Pin_3;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio);

    usart.USART_BaudRate = 115200;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    usart.USART_Parity = USART_Parity_No;
    usart.USART_StopBits = USART_StopBits_1;
    usart.USART_WordLength = USART_WordLength_8b;

    USART_Init(USART2, &usart);

    nvic.NVIC_IRQChannel = USART2_IRQn;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    nvic.NVIC_IRQChannelSubPriority = 0;
    nvic.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_Init(&nvic);
    USART_Cmd(USART2, ENABLE);
}

void uart_writeByte(uint8_t b) {
    int32_t freeBufSpace;
    do {
        freeBufSpace = uart.outReadPos - uart.outWritePos - 1;
        if (freeBufSpace < 0)
            freeBufSpace += UART_BUF_OUT_SIZE;
    } while (freeBufSpace < 2);
    uart.outputBuffer[uart.outWritePos++] = b;
    uart.outWritePos %= UART_BUF_OUT_SIZE;
    USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
}

void uart_writeData(uint8_t *data, uint32_t length) {
    for (; length > 0; length--)
        uart_writeByte(*data++);
}

void uart_writeString(char *s) {
    while (*s)
        uart_writeByte(*s++);
}

void USART2_IRQHandler(void) {
    if (USART_GetITStatus(USART2, USART_IT_TXE)) {
        USART_ClearITPendingBit(USART2, USART_IT_TXE);
        if (uart.outReadPos != uart.outWritePos) {
            USART_SendData(USART2, uart.outputBuffer[uart.outReadPos++]);
            uart.outReadPos %= UART_BUF_OUT_SIZE;
        } else {
            USART_ClearITPendingBit(USART2, USART_IT_TXE);
            USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
        }
    }
}