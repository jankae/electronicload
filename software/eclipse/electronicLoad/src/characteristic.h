#ifndef CHARACTERISTIC_H_
#define CHARACTERISTIC_H_

#include "menu.h"
#include "common.h"

struct {
    uint8_t active;
    uint32_t currentStart;
    uint32_t currentStop;
    uint32_t abortVoltage;
    uint32_t deltaT;
    uint32_t voltageResponse[120];
    volatile uint8_t pointCount;
    uint32_t timeCount;
} characteristic;

void characteristic_Menu(void);

void characteristic_Run(void);

void characteristic_ViewResult(void);

void characteristic_Update(void);

uint32_t characteristic_DatapointToCurrent(uint8_t point);

#endif
