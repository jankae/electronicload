/**
 * \file
 * \brief   Calibration header file.
 *
 * This file contains calibration values and functions to calibrate
 * the load and store the calibration values
 */
#ifndef CALIBRATION_H_
#define CALIBRATION_H_

#include <screen.h>
#include "currentSink.h"
#include "frontPanel.h"
#include "multimeter.h"


struct {

    uint8_t active;
} calibration;

/**
 * \brief Transfers the calibration values from the end of the FLASH
 *
 * (see linker script, section '.config') into the RAM
 * \return 0 on success, 1 on failure
 */
uint8_t cal_readFromFlash(void);

/**
 * \brief Writes the calibration values from the RAM into the end
 * of the FLASH.
 *
 * Use sparsely to preserve FLASH
 */
void cal_writeToFlash(void);

/**
 * \brief Samples an ADC channel
 *
 * The specified channel is sampled 200 times, averaged and returned.
 * Additionally, this function displays a progress bar.
 *
 * \param channel   ADC channel to be sampled
 * \return          averaged ADC channel value
 */
uint32_t cal_sampleADC(uint8_t channel);

/**
 * \brief Sets the calibration values to the default values.
 *
 * Should be used in case of missing calibration data.
 */
void cal_setDefaultCalibration(void);

void calibrationMenu(void);

void calibrationProcessAutomatic(void);

/**
 * \brief Starts and executes the calibration process.
 *
 * Stand-alone function, start from main thread, depends on interrupts
 * to update display and  to get user inputs.
 * IMPORTANT: disable all functions dealing with the DAC, this is all
 * done internally in this function
 */
void calibrationProcessManual(void);

void calibrationProcessHardware(void);

void calibrationDisplayMultimeterInfo(void);

/**
 * \brief Sets the 'should be'-current
 *
 * \param mA Current the load should draw
 */
void cal_setCurrent(uint32_t mA);

/**
 * \brief Returns the current being drawn
 *
 * \return Current in mA
 */
int32_t cal_getCurrent(void);

/**
 * \brief Returns the voltage at the terminals
 *
 * \return Voltage in mV
 */
int32_t cal_getVoltage(void);

int32_t cal_getUncalibVoltage(void);
int32_t cal_getUncalibCurrent(void);

/**
 * \brief Returns the temperature at heatsink1
 *
 * \return Temperature in °C
 */
uint8_t cal_getTemp1(void);

/**
 * \brief Returns the temperature at heatsink2
 *
 * \return Temperature in °C
 */
uint8_t cal_getTemp2(void);



#endif
