#include "errors.h"

void errors_Check(void){
    static uint8_t errorDuration[32];
    /******************************************************************
     * Load drawing current while input is switched off
     *****************************************************************/
    if (!load.powerOn && load.state.current > 100) {
        // input is switched off but load is still drawing more than 100uA
        if (errorDuration[LOAD_ERROR_OFF_CURRENT] < 254)
            errorDuration[LOAD_ERROR_OFF_CURRENT] += 2;
    }
    /******************************************************************
     * Load drawing wrong amount of current
     *****************************************************************/
    if (load.powerOn && load.mode == FUNCTION_CC
            && load.state.voltage > 500000) {
        uint32_t minCurrent = (load.current - load.current >> 3) - 10000;
        uint32_t maxCurrent = (load.current + load.current >> 3) + 10000;
        if (load.state.current < minCurrent
                || load.state.current > maxCurrent) {
            if (errorDuration[LOAD_ERROR_WRONG_CURRENT] < 254)
                errorDuration[LOAD_ERROR_WRONG_CURRENT] += 2;
        }
    }
    /******************************************************************
     * Load not regulating to correct voltage
     *****************************************************************/
    if (load.powerOn && load.mode == FUNCTION_CV && load.state.current > 1000) {
        uint32_t minVoltage = (load.voltage - load.voltage >> 3) - 100000;
        uint32_t maxVoltage = (load.voltage + load.voltage >> 3) + 100000;
        if (load.state.voltage < minVoltage
                || load.state.voltage > maxVoltage) {
            if (errorDuration[LOAD_ERROR_WRONG_VOLTAGE] < 254)
                errorDuration[LOAD_ERROR_WRONG_VOLTAGE] += 2;
        }
    }
    /******************************************************************
     * Load not regulating to correct power
     *****************************************************************/
    if (load.powerOn && load.mode == FUNCTION_CP
            && (load.state.current > 1000 || load.state.voltage > 500000)) {
        uint32_t minPower = (load.power - load.power >> 2) - 100000;
        uint32_t maxPower = (load.power + load.power >> 2) + 100000;
        if (load.state.power < minPower || load.state.power > maxPower) {
            if (errorDuration[LOAD_ERROR_WRONG_POWER] < 254)
                errorDuration[LOAD_ERROR_WRONG_POWER] += 2;
        }
    }
    /******************************************
     * Check error durations and set error code
     *****************************************/
    uint8_t i;
    uint32_t bitmask = 0x01;
    for (i = 1; i < 32; i++) {
        // reduce error slowly (if the error is present the errorDuration is
        // increased twice as fast thus still showing an error)
        if (errorDuration[i]) {
            errorDuration[i]--;
        }
        if (errorDuration[i] > LOAD_MAX_ERROR_DURATION) {
            error.code |= bitmask;
        }
        bitmask <<= 1;
    }
}
