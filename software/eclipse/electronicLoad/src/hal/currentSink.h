
#ifndef CURRENTSINK_H_
#define CURRENTSINK_H_

#include <stdint.h>
#include "stm32f10x_conf.h"

#define MAX_CURRENT		20000

#define ADC_CURRENT_SENSE		0
#define ADC_VOLTAGE_SENSE		1
#define ADC_TEMPERATURE1		2
#define ADC_TEMPERATURE2		3

typedef enum {RANGE_LOW, RANGE_HIGH} range_t;

struct {
	range_t currentRange;
	range_t voltageRange;
	range_t setRange;
	uint16_t rawADC[4];
} hal;

void hal_currentSinkInit(void);

void hal_setDAC(uint16_t dac);

void hal_setGain(uint8_t en);

void hal_setVoltageGain(uint8_t en);

void hal_setCurrentGain(uint8_t en);

void hal_setFan(uint8_t en);

uint16_t hal_getADC(uint8_t channel, uint8_t nsamples);

#endif
