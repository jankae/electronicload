/**
 * \file
 * \brief   Load control header file.
 *
 * This file handles all high level load control functions
 */
#ifndef LOADFUNCTIONS_H_
#define LOADFUNCTIONS_H_

#include <stdint.h>
#include "currentSink.h"
#include "calibration.h"
#include "statistics.h"
#include "waveforms.h"
#include "settings.h"
#include "characteristic.h"

#define LOAD_MAX_TEMP           100
#define LOAD_FANON_TEMP         30
#define LOAD_FANOFF_TEMP        25

typedef enum {
    FUNCTION_CC = 0, FUNCTION_CV = 1, FUNCTION_CR = 2, FUNCTION_CP = 3
} loadMode_t;

#include "events.h"

struct {
    uint8_t highPower;
    loadMode_t mode;
    // values for the different modes
    // constant current in mA
    int32_t current;
    // constant voltage in mV
    int32_t voltage;
    // constant resistance in mOhm
    int32_t resistance;
    // constant power in mW
    int32_t power;

    uint8_t powerOn;

    uint8_t triggerInOld;

    struct {
        uint32_t current;
        uint64_t currentSum;
        uint32_t voltage;
        uint64_t voltageSum;
        uint32_t power;
        uint64_t powerSum;
        uint16_t temp1;
        uint16_t temp2;
        uint32_t nsamples;
    } state;
} load;

/**
 * \brief Initializes loadregulation
 *
 * Sets all modes to minimum power consumption
 * and activates constant current mode.
 * Also registers a callback to periodically
 * call load_update()
 */
void load_Init(void);

/**
 * \brief Sets constant current mode
 *
 * \param c Current in mA
 */
void load_set_CC(uint32_t c);

/**
 * \brief Sets constant voltage mode
 *
 * \param v Voltage in mV
 */
void load_set_CV(uint32_t v);

/**
 * \brief Sets constant resistance mode
 *
 * \param r Resistance in mOhm
 */
void load_set_CR(uint32_t r);

/**
 * \brief Sets constant power mode
 *
 * \param p Power in mW
 */
void load_set_CP(uint32_t p);

/**
 * \brief Changes the active load mode
 *
 * The load will use the already set mode parameter
 * (e.i. switching between modes without affecting
 * parameters)
 *
 * \param mode New load mode
 */
void load_setMode(loadMode_t mode);

/**
 * brief Limits the load settigns
 */
void load_ConstrainSettings(void);

/**
 * \brief Updates the current drawn by load according to selected load function
 *
 * This function is called from an interrupt (using timer 2) every millisecond
 */
void load_update(void);

#endif
