#include "multimeter.h"

void multimeter_Init() {
    GPIO_InitTypeDef gpio;
    USART_InitTypeDef usart;
    NVIC_InitTypeDef nvic;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    // uart pin is PB11
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    gpio.GPIO_Pin = GPIO_Pin_11;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpio);

    usart.USART_BaudRate = 19200;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart.USART_Mode = USART_Mode_Rx;
    usart.USART_Parity = USART_Parity_No;
    usart.USART_StopBits = USART_StopBits_1;
    usart.USART_WordLength = USART_WordLength_8b;

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    USART_Init(USART3, &usart);

    nvic.NVIC_IRQChannel = USART3_IRQn;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    nvic.NVIC_IRQChannelSubPriority = 0;
    nvic.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_Init(&nvic);
    USART_Cmd(USART3, ENABLE);
}

void multimeter_EvaluateData() {
    meter.function = meter.data[6];
    if (meter.data[10] & 0x02)
        meter.AUTO = 1;
    else
        meter.AUTO = 0;
    if (meter.data[10] & 0x08)
        meter.DC = 1;
    else
        meter.DC = 0;

    // convert range + digits from ascii to binary
    uint8_t range = meter.data[0] - '0';
    meter.data[1] -= '0';
    meter.data[2] -= '0';
    meter.data[3] -= '0';
    meter.data[4] -= '0';
    meter.data[5] -= '0';

    // extract raw value
    int32_t value = meter.data[1] * 10000 + meter.data[2] * 1000
            + meter.data[3] * 100 + meter.data[4] * 10 + meter.data[5];

    // shift value according to current range
    switch (meter.function) {
    case UT61E_FUNCTION_VOLTAGE:
        if (range == 0) {
            value *= 100;
        } else if (range == 1) {
            value *= 1000;
        } else if (range == 2) {
            value *= 10000;
        } else if (range == 3) {
            value *= 100000;
        } else if (range == 4) {
            value *= 10;
        }
        break;
    case UT61E_FUNCTION_CURRENT_UA:
        if (range == 0) {
            value /= 100;
        } else if (range == 1) {
            value /= 10;
        }
        break;
    case UT61E_FUNCTION_CURRENT_MA:
        /*if (range == 0) {
         value *= 1;
         } else*/
        if (range == 1) {
            value *= 10;
        }
        break;
    case UT61E_FUNCTION_CURRENT_A:
        value *= 1000;
        break;
    }

    if(meter.data[7] & 0x04){
        // sign bit set
        value *= -1;
    }
    meter.value = value;
    meter.timeout = timer_SetTimeout(2000);
}

void USART3_IRQHandler(void) {
    if (USART_GetITStatus(USART3, USART_IT_RXNE)) {
        uint8_t data = USART_ReceiveData(USART3);
        // mask MSB as the multimeter is actually sending in 7O1 format
        // (STM just doesn't support this)
        data &= 0x7F;
//        uart_writeByte(data);
        if (data == '\n') {
            // end of string
            if (meter.bytecount == 13) {
                // string is complete
                // -> evaluate
                multimeter_EvaluateData();
            }
            meter.bytecount = 0;
        } else {
            if (meter.bytecount < 13) {
                meter.data[meter.bytecount] = data;
            }
            meter.bytecount++;
        }
    }
}

