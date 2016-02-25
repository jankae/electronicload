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
    load.mode = FUNCTION_CC;
    load.current = 0;
    load.voltage = LOAD_MAXVOLTAGE;
    load.resistance = LOAD_MAXRESISTANCE;
    load.power = 0;
    load.triggerInOld = hal_getTriggerIn();
    timer_SetupPeriodicFunction(2, MS_TO_TICKS(1), load_update, 4);
}

/**
 * \brief Sets constant current mode
 *
 * \param c Current in mA
 */
void load_set_CC(uint32_t c) {
    load.mode = FUNCTION_CC;
    load.current = c;
}

/**
 * \brief Sets constant voltage mode
 *
 * \param v Voltage in mV
 */
void load_set_CV(uint32_t v) {
    load.mode = FUNCTION_CV;
    load.voltage = v;
}

/**
 * \brief Sets constant resistance mode
 *
 * \param r Resistance in mOhm
 */
void load_set_CR(uint32_t r) {
    load.mode = FUNCTION_CR;
    load.resistance = r;
}

/**
 * \brief Sets constant power mode
 *
 * \param p Power in mW
 */
void load_set_CP(uint32_t p) {
    load.mode = FUNCTION_CP;
    load.power = p;
}

/**
 * \brief Changes the active load mode
 *
 * The load will use the already set mode parameter
 * (e.i. switching between modes without affecting
 * parameters)
 *
 * \param mode New load mode
 */
void load_setMode(loadMode_t mode) {
    load.mode = mode;
}

/**
 * \brief Updates the current drawn by load according to selected load function
 *
 * This function is called from an interrupt (using timer 2) every millisecond
 */
void load_update(void) {
    if (calibration.active)
        return;
    load.state.voltage = cal_getVoltage();
    load.state.current = cal_getCurrent();
    load.state.power = load.state.voltage
            * load.state.current / 1000;

    uint32_t current = 0;

    uint8_t triggerIn = hal_getTriggerIn();
    events.triggerInState = triggerIn - load.triggerInOld;
    load.triggerInOld = triggerIn;

    events_decrementTimers();
    events_HandleEvents();

    switch (load.mode) {
    case FUNCTION_CC:
        current = load.current;
        break;
    case FUNCTION_CV:
        //TODO
        break;
    case FUNCTION_CR:
        current = (load.state.voltage * 1000) / load.resistance;
        break;
    case FUNCTION_CP:
        if (load.state.voltage > 0)
            current = (load.power * 1000) / load.state.voltage;
        else
            current = MAX_CURRENT;
        break;
    }
    if (load.powerOn) {
        cal_setCurrent(current);
    } else {
        cal_setCurrent(0);
    }
}
