/**
 * \file 	currentSink.h
 * \author 	Jan Kaeberich
 * \brief	Analog board hardware abstraction layer header file.
 *
 * 	        This file handles all communication with the analog
 * 	        power sink board.
 */
#ifndef CURRENTSINK_H_
#define CURRENTSINK_H_

#include <stdint.h>
#include "stm32f10x_conf.h"

/**
 * \def MAX_CURRENT
 * maximum settable current in mA
 */
#define MAX_CURRENT		20000

/**
 * \name rawADC_indices ADC channels
 * \{
 */
#define ADC_CURRENT_SENSE       0
#define ADC_VOLTAGE_SENSE       1
#define ADC_TEMPERATURE1        2
#define ADC_TEMPERATURE2        3
/** \} */

typedef enum {
    RANGE_LOW, RANGE_HIGH
} range_t;

struct {
    range_t currentRange;
    range_t voltageRange;
    range_t setRange;
    uint16_t rawADC[4];
} hal;

/**
 * \brief Initialises the current sink hardware
 *
 * Initialises GPIOs used for communication with the
 * analog board. Configures ADC and DMA to continuously
 * sample and store all four channels. Sets ranges to
 * default values (all low).
 */
void hal_currentSinkInit(void);

/**
 * \brief Sends a value to the DAC on the analog board
 *
 * \param dac 12-bit DAC value (1LSB equals 1mV)
 */
void hal_setDAC(uint16_t dac);

/**
 * \brief Sets the current set gain
 *
 * \param en 0: gain is set to 1, 1: gain is set to 10
 */
void hal_setGain(uint8_t en);

/**
 * \brief Sets the voltage measurement gain
 *
 * \param en 0: gain is set to 1, 1: gain is set to 10
 */
void hal_setVoltageGain(uint8_t en);

/**
 * \brief Sets the current measurement gain
 *
 * \param en 0: gain is set to 1, 1: gain is set to 10
 */
void hal_setCurrentGain(uint8_t en);

/**
 * \brief Controls the fans on the analog board
 *
 * \param en 0: fans disabled, 1: fans enabled
 */
void hal_setFan(uint8_t en);

/**
 * \brief Reads an ADC channel
 *
 * \param channel Channel selection. Can be any one of the ADC channels
 * \param nsamples Number of samples (result will be averaged)
 * \return 12-Bit ADC value
 */
uint16_t hal_getADC(uint8_t channel, uint8_t nsamples);

#endif
