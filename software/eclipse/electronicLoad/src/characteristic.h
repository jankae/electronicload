#ifndef CHARACTERISTIC_H_
#define CHARACTERISTIC_H_

#include "GUI/gui.h"
#include "common.h"

struct {
    uint8_t active;
    uint8_t resultValid;
    int32_t currentStart;
    int32_t currentStop;
    uint32_t abortVoltage;
    uint32_t deltaT;
    uint32_t voltageResponse[120];
    volatile uint8_t pointCount;
    uint32_t timeCount;
} characteristic;

void characteristic_Init(void);

widget_t* characteristic_getWidget(void);

void characteristic_Run(void);

void characteristic_ViewResult(void);

void characteristic_TransmitResult(void);

void characteristic_Update(void);

uint32_t characteristic_DatapointToCurrent(uint8_t point);

#endif
