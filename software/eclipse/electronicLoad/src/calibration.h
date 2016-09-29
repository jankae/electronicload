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

#define FLASH_CALIBRATION_DATA      0x0801F004
#define FLASH_VALID_CALIB_INDICATOR 0x0801F000

#define CAL_INDICATOR               0x02

#define CAL_ERROR_METER_MONOTONIC   1
#define CAL_ERROR_ADC_MONOTONIC     2
#define CAL_ERROR_SHUNTFACTOR       3

#define CAL_ADC_NSAMPLES            1000
#define CAL_METER_NSAMPLES          10

/*
 * This section must fit into four flash pages and thus
 * NEVER exceed 4kB
 */
struct {
    // ADC value[0] -> actual current[1]
    int32_t currentSenseTable[2][2];
    // DAC value[0] -> actual current[1]
    int32_t currentSetTable[2][2];

    // ADC value[0] -> actual voltage[1]
    int32_t voltageSenseTable[2][2];
    // DAC value[0] -> actual voltage[1]
    int32_t voltageSetTable[2][2];

    // DAC value[0] -> actual power[1]
    int32_t powerSetTable[2][2];

    // DAC value[0] -> actual conductance[1]
    int32_t conductanceSetTable[2][2];

// factor between the two current shunts (in %)
// should be about 10000 (R01:1R)
    uint32_t shuntFactor;
} calData;

struct {
    uint8_t active;
    uint8_t unsavedData;
    uint16_t rawADCcurrent;
    uint16_t rawADCvoltage;
} cal;



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
 * \brief Returns the (averaged) value reported by a connected multimeter
 *
 * The function waits until the meter value has stabilized (either remaining
 * the same or small variations in both directions). It the samples the meter
 * 'samples'-times and returns the average value.
 * \param samples Number of samples
 * \return Averaged raw meter value
 */
int32_t cal_sampleMeter(uint8_t samples);

/**
 * \brief Returns the (averaged) raw ADC value
 *
 * \param samples Number of samples
 * \param *ADCdata Pointer to ADC data (should be either &cal.rawADCcurrent or &cal.rawADCvoltage)
 */
int32_t cal_sampleADC(uint16_t samples, uint16_t *ADCdata);

/**
 * \brief Sets the calibration values to the default values.
 *
 * Should be used in case of missing calibration data.
 */
void cal_setDefaultCalibration(void);

void calibrationMenu(void);

void calibrationProcessAutomatic(void);

void cal_DisplayError(uint8_t error);

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
 * \param uA Current the load should draw
 */
void cal_setCurrent(uint32_t uA);

void cal_setVoltage(uint32_t uV);

void cal_setPower(uint32_t uW);

void cal_setResistance(uint32_t uR);

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
