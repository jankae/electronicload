/**
 * \file
 * \brief   Load control source file.
 *
 * This file handles all high level load control functions
 */
#include "loadFunctions.h"

const char loadModeNames[4][3] = { "CC", "CV", "CR", "CP" };

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
    load.voltage = settings.maxVoltage;
    load.resistance = LOAD_MAXRESISTANCE_LOWP;
    load.power = 0;
    load.triggerInOld = hal_getTriggerIn();
    timer_SetupPeriodicFunction(2, MS_TO_TICKS(1), load_update, 4);
}

void load_GetAverageAndReset(uint32_t *current, uint32_t *voltage,
        uint32_t *power) {
    *current = load.state.currentSum / load.state.nsamples;
    *voltage = load.state.voltageSum / load.state.nsamples;
    *power = load.state.powerSum / load.state.nsamples;
    load.state.currentSum = 0;
    load.state.voltageSum = 0;
    load.state.powerSum = 0;
    load.state.nsamples = 0;
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
    else if (load.current > settings.maxCurrent)
        load.current = settings.maxCurrent;

    if (load.voltage < settings.minVoltage)
        load.voltage = settings.minVoltage;
    else if (load.voltage > settings.maxVoltage)
        load.voltage = settings.maxVoltage;

    if (load.resistance < settings.minResistance)
        load.resistance = settings.minResistance;
    else if (load.resistance > settings.maxResistance)
        load.resistance = settings.maxResistance;

    if (load.power < 0)
        load.power = 0;
    else if (load.power > settings.maxPower)
        load.power = settings.maxPower;
}

/**
 * \brief Updates the current drawn by load according to selected load function
 *
 * This function is called from an interrupt (using timer 2) every millisecond
 */
void load_update(void) {
    hal_frontPanelUpdate();

    if (load.disableIOcontrol)
        return;

    if (settings.turnOffOnError && error.code) {
        load.powerOn = 0;
        hal_SelectShunt(HAL_SHUNT_NONE);
    } else if (settings.powerMode) {
        hal_SelectShunt(HAL_SHUNT_R01);
    } else {
        hal_SelectShunt(HAL_SHUNT_1R);
    }

    load.state.voltage = cal_getVoltage();

    load.state.current = cal_getCurrent();

    load.state.power = (uint64_t) load.state.voltage * load.state.current
            / 1000000UL;

    // update average values
    load.state.voltageSum += load.state.voltage;
    load.state.currentSum += load.state.current;
    load.state.powerSum += load.state.power;
    load.state.nsamples++;

    load.state.temp1 = cal_getTemp1();
    load.state.temp2 = cal_getTemp2();
    uint16_t highTemp = load.state.temp1;
    if (load.state.temp2 > load.state.temp1)
        highTemp = load.state.temp2;

    // switch fan
    if (highTemp >= LOAD_FANON_TEMP)
        hal_setFan(1);
    else if (highTemp <= LOAD_FANOFF_TEMP)
        hal_setFan(0);

    uint8_t triggerIn = hal_getTriggerIn();
    events.triggerInState = triggerIn - load.triggerInOld;
    load.triggerInOld = triggerIn;

    if (!cal.active) {
        // only run function that can potentially change settings
        // while calibration is not active
        events_decrementTimers();
        events_updateWaveformPhase();
        events_HandleEvents();

        arb_Update();
        waveform_Update();
        characteristic_Update();
        load_ConstrainSettings();

        uint32_t current = 0;
//    uint32_t currentLimit = ((uint64_t) settings.maxPower
//            * 1000000) / load.state.voltage;

        uint8_t enableInput = load.powerOn;
        if (highTemp > LOAD_MAX_TEMP) {
            // disable input if temperature too high
            enableInput = 0;
        }
        switch (load.mode) {
        case FUNCTION_CC:
//        if (load.current > currentLimit)
//            current = currentLimit;
//        else
            current = load.current;
            hal_SetControlMode(HAL_MODE_CC);
            if (enableInput) {
                cal_setCurrent(current);
            } else {
                hal_setDAC(0);
            }
            break;
        case FUNCTION_CV:
            hal_SetControlMode(HAL_MODE_CV);
            if (enableInput) {
                cal_setVoltage(load.voltage);
            } else {
                hal_setDAC(HAL_DAC_MAX);
            }
            break;
        case FUNCTION_CR:
            // control resistance in digital mode: set current depending on voltage
            hal_SetControlMode(HAL_MODE_CC);
            if (enableInput) {
                // calculate necessary current
                current = ((uint64_t) load.state.voltage * 1000)
                        / load.resistance;
                cal_setCurrent(current);
            } else {
                hal_setDAC(0);
            }
            break;
        case FUNCTION_CP:
            // control control in digital mode: set current depending on voltage
            hal_SetControlMode(HAL_MODE_CC);
            if (enableInput) {
                // calculate necessary current
                current = ((uint64_t) load.power * 1000000)
                        / load.state.voltage;
                cal_setCurrent(current);
            } else {
                hal_setDAC(0);
            }
            break;
        }
        // update trigger out
        if (events.triggerOutState == 1) {
            hal_setTriggerOut(1);
        } else if (events.triggerOutState == -1) {
            hal_setTriggerOut(0);
        }
        errors_Check();
    } else {
        // calibration is active
        switch (load.mode) {
        case FUNCTION_CC:
            hal_SetControlMode(HAL_MODE_CC);
            break;
        case FUNCTION_CV:
            hal_SetControlMode(HAL_MODE_CV);
            break;
        case FUNCTION_CR:
            hal_SetControlMode(HAL_MODE_CC);
            break;
        case FUNCTION_CP:
            hal_SetControlMode(HAL_MODE_CC);
            break;
        }
        hal_setDAC(load.DACoverride);
    }

    stats_Update();
}
