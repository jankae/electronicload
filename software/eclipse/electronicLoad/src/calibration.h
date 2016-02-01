
#ifndef CALIBRATION_H_
#define CALIBRATION_H_

#include "currentSink.h"
#include "frontPanel.h"
#include "displayRoutines.h"

#define LM35_ADC_TO_TEMP(adc)		((adc*3300)/4096)

struct {
	uint8_t active;

	int16_t currentSenseOffsetLowRange;
	float currentSenseScaleLowRange;

	int16_t currentSenseOffsetHighRange;
	float currentSenseScaleHighRange;

	int16_t voltageSenseOffsetLowRange;
	float voltageSenseScaleLowRange;

	int16_t voltageSenseOffsetHighRange;
	float voltageSenseScaleHighRange;

	int16_t currentSetOffsetLowRange;
	float currentSetScaleLowRange;

	int16_t currentSetOffsetHighRange;
	float currentSetScaleHighRange;
} calibration;

/*
 * Starts and executes the calibration process. Stand-alone function,
 * start from main thread, depends on interrupts to update display and
 * to get user inputs.
 * IMPORTANT: disable all functions dealing with the DAC, this is all
 * done internally in this function
 */
void calibrationProcess(void);

/*
 * sets the 'should be'-current
 */
void cal_setCurrent(uint32_t mA);

/*
 * returns the actual sinked current in mA
 */
int32_t cal_getCurrent(void);

/*
 * returns the measured voltage at the terminals in mV
 */
int32_t cal_getVoltage(void);

/*
 * returns the temperature of heatsink1 in 0.1°C
 */
uint16_t cal_getTemp1(void);

/*
 * returns the temperature of heatsink2 in 0.1°C
 */
uint16_t cal_getTemp2(void);

#endif
