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

#define LOAD_MAXVOLTAGE			50000
#define LOAD_MAXCURRENT			20000
#define LOAD_MAXRESISTANCE		1000000
#define LOAD_MAXPOWER			100000

typedef enum {
	FUNCTION_CC, FUNCTION_CV, FUNCTION_CR, FUNCTION_CP
} loadType_t;

struct {
	loadType_t type;
	// values for the different modes
	// constant current in mA
	uint32_t current;
	// constant voltage in mV
	uint32_t voltage;
	// constant resistance in mOhm
	uint32_t resistance;
	// constant power in mW
	uint32_t power;
} loadFunctions;

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
 * \brief Updates the current drawn by load according to selected load function
 *
 * This function is called from an interrupt (using timer 2) every millisecond
 */
void load_update(void);

#endif
