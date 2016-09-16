/*
 * multimeter.h
 *
 *  Created on: Sep 10, 2016
 *      Author: jan
 */

#ifndef HAL_MULTIMETER_H_
#define HAL_MULTIMETER_H_

#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "timer.h"

#define UT61E_FUNCTION_VOLTAGE      0x3B
#define UT61E_FUNCTION_CURRENT_UA   0x3D
#define UT61E_FUNCTION_CURRENT_MA   0x3F
#define UT61E_FUNCTION_CURRENT_A    0x30

struct {
    uint8_t function;
    uint8_t DC :1;
    uint8_t AUTO :1;
    // in uV/uA
    int32_t value;
    uint8_t bytecount;
    uint8_t data[13];
    uint32_t timeout;
} meter;

void multimeter_Init();

void multimeter_EvaluateDate();

#endif /* HAL_MULTIMETER_H_ */
