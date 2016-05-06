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

void load_ConstrainSettings(void) {
    if (load.current < 0)
        load.current = 0;
    else if (load.current > LOAD_MAXCURRENT)
        load.current = LOAD_MAXCURRENT;

    if (load.voltage < LOAD_MINVOLTAGE)
        load.voltage = LOAD_MINVOLTAGE;
    else if (load.voltage > LOAD_MAXVOLTAGE)
        load.voltage = LOAD_MAXVOLTAGE;

    if (load.resistance < LOAD_MINRESISTANCE)
        load.resistance = LOAD_MINRESISTANCE;
    else if (load.resistance > LOAD_MAXRESISTANCE)
        load.resistance = LOAD_MAXRESISTANCE;

    if (load.power < 0)
        load.power = 0;
    else if (load.power > LOAD_MAXPOWER)
        load.power = LOAD_MAXPOWER;
}

/**
 * \brief Updates the current drawn by load according to selected load function
 *
 * This function is called from an interrupt (using timer 2) every millisecond
 */
void load_update(void) {
    hal_frontPanelUpdate();
    if (calibration.active)
        return;
    load.state.voltage = cal_getVoltage();
    load.state.current = cal_getCurrent();
    load.state.power = load.state.voltage * load.state.current / 1000;
    load.state.voltageSum += load.state.voltage;
    load.state.currentSum += load.state.current;
    load.state.powerSum += load.state.power;
    load.state.nsamples++;

    load.state.temp1 = cal_getTemp1();
    load.state.temp2 = cal_getTemp2();
    uint16_t highTemp = load.state.temp1;
    if (load.state.temp2 > load.state.temp1)
        highTemp = load.state.temp2;
    // convert to °C
    highTemp /= 10;

    // switch fan
    if (highTemp >= LOAD_FANON_TEMP)
        hal_setFan(1);
    else if (highTemp <= LOAD_FANOFF_TEMP)
        hal_setFan(0);

    uint32_t current = 0;

    uint8_t triggerIn = hal_getTriggerIn();
    events.triggerInState = triggerIn - load.triggerInOld;
    load.triggerInOld = triggerIn;

    events_decrementTimers();
    events_HandleEvents();

    waveform_Update();
    load_ConstrainSettings();

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
    if (load.powerOn && highTemp <= LOAD_MAX_TEMP
            && load.state.voltage >= 2700) {
        cal_setCurrent(current);
    } else {
        hal_setDAC(0);
        //cal_setCurrent(0);
    }
    stats_Update();
}
