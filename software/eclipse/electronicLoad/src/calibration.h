
#ifndef CALIBRATION_H_
#define CALIBRATION_H_

#include "currentSink.h"
#include "frontPanel.h"
#include "displayRoutines.h"

#define LM35_ADC_TO_TEMP(adc)		((adc*3300)/4096)

// default calibration values (assuming perfect parts and resistor values)
#define CAL_DEF_CURSENS_OFFSET_LOW	0
#define CAL_DEF_CURSENS_SCALE_LOW	5.000 // TODO adjust to shunt

#define CAL_DEF_CURSENS_OFFSET_HIGH	0
#define CAL_DEF_CURSENS_SCALE_HIGH	50.00 // TODO adjust to shunt

#define CAL_DEF_VOLSENS_OFFSET_LOW	0
#define CAL_DEF_VOLSENS_SCALE_LOW	3.4375

#define CAL_DEF_VOLSENS_OFFSET_HIGH	0
#define CAL_DEF_VOLSENS_SCALE_HIGH	34.375

#define CAL_DEF_CURSET_OFFSET_LOW	0
#define CAL_DEF_CURSET_SCALE_LOW	0.2

#define CAL_DEF_CURSET_OFFSET_HIGH	0
#define CAL_DEF_CURSET_SCALE_HIGH	0.02

struct {
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

	uint8_t active;
} calibration;

/*
 * Transfers the calibration values from the end of the FLASH
 * (see linker script, section '.config') into the RAM
 * @return 0 on success, 1 on failure
 */
uint8_t cal_readFromFlash(void);

/*
 * Writes the calibration values from the RAM into the end
 * of the FLASH. Use sparsely to preserve FLASH
 */
void cal_writeToFlash(void);

/*
 * Sets the calibration values to the default values.
 * Should be used in case of missing calibration data.
 */
void cal_setDefaultCalibration(void);

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
