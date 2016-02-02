/**
 * \file
 * \brief   Load control source file.
 *
 * This file handles all high level load control functions
 */
#include "loadFunctions.h"

/**
 * \brief Initializes loadregulation
 *
 * Sets all modes to minimum power consumption
 * and activates constant current mode.
 * Also registers a callback to periodically
 * call load_update()
 */
void load_Init(void) {
    loadFunctions.mode = FUNCTION_CC;
    loadFunctions.current = 0;
    loadFunctions.voltage = LOAD_MAXVOLTAGE;
    loadFunctions.resistance = LOAD_MAXRESISTANCE;
    loadFunctions.power = 0;
    timer_SetupPeriodicFunction(2, MS_TO_TICKS(1), load_update, 4);
}

/**
 * \brief Sets constant current mode
 *
 * \param c Current in mA
 */
void load_set_CC(uint32_t c) {
    loadFunctions.mode = FUNCTION_CC;
    loadFunctions.current = c;
}

/**
 * \brief Sets constant voltage mode
 *
 * \param v Voltage in mV
 */
void load_set_CV(uint32_t v) {
    loadFunctions.mode = FUNCTION_CV;
    loadFunctions.voltage = v;
}

/**
 * \brief Sets constant resistance mode
 *
 * \param r Resistance in mOhm
 */
void load_set_CR(uint32_t r) {
    loadFunctions.mode = FUNCTION_CR;
    loadFunctions.resistance = r;
}

/**
 * \brief Sets constant power mode
 *
 * \param p Power in mW
 */
void load_set_CP(uint32_t p) {
    loadFunctions.mode = FUNCTION_CP;
    loadFunctions.power = p;
}

/**
 * \brief Updates the current drawn by load according to selected load function
 *
 * This function is called from an interrupt (using timer 2) every millisecond
 */
void load_update(void) {
    uint32_t voltage = cal_getVoltage();
    uint32_t current = 0;
    switch (loadFunctions.mode) {
    case FUNCTION_CC:
        current = loadFunctions.current;
        break;
    case FUNCTION_CV:
        //TODO
        break;
    case FUNCTION_CR:
        current = (voltage * 1000) / loadFunctions.resistance;
        break;
    case FUNCTION_CP:
        if (voltage > 0)
            current = (loadFunctions.power * 1000) / voltage;
        else
            current = MAX_CURRENT;
        break;
    }
    cal_setCurrent(current);
}
