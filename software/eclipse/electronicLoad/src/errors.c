#include "errors.h"

const char *errorDescr[] = { "Load draws current despite being turned off",
        "Load current deviates from set current",
        "Load voltage deviates from set voltage",
        "Load power deviates from set power" };

void error_Menu(void) {
    if (error.code) {
        // convert error codes from uint32_t bitfield into array
        // with individual error codes
        uint8_t errCodes[32];
        uint32_t errCode = error.code;
        uint8_t i;
        uint8_t numErrors = 0;
        for (i = 0; i < 32; i++) {
            if (errCode & 0x01) {
                errCodes[numErrors++] = i;
            }
            errCode >>= 1;
        }
        uint32_t button;
        uint8_t selectedErrorNum = 0;
        do {
            uint8_t selectedError = errCodes[selectedErrorNum];
            screen_Clear();
            // display error code
            screen_FastString6x8("ErrCode", 0, 0);
            screen_FastChar6x8((selectedError + 1) / 10 + '0', 48, 0);
            screen_FastChar6x8((selectedError + 1) % 10 + '0', 54, 0);
            screen_FastChar6x8(':', 60, 0);
            // display error number + number of errors
            screen_FastChar6x8((selectedErrorNum + 1) / 10 + '0', 98, 0);
            screen_FastChar6x8((selectedErrorNum + 1) % 10 + '0', 104, 0);
            screen_FastChar6x8('/', 110, 0);
            screen_FastChar6x8(numErrors / 10 + '0', 116, 0);
            screen_FastChar6x8(numErrors % 10 + '0', 122, 0);
            // display error explanation
            screen_Text6x8(errorDescr[selectedError], 0, 1);
            if (selectedErrorNum > 0) {
                screen_SetSoftButton("Prev", 0);
            }
            if (selectedErrorNum < numErrors - 1) {
                screen_SetSoftButton("Next", 2);
            }
            screen_SetSoftButton("Clear", 1);
            while (!(button = hal_getButton()))
                ;
            while (hal_getButton())
                ;
            if ((button & HAL_BUTTON_SOFT0) && selectedErrorNum > 0) {
                selectedErrorNum--;
            }
            if ((button & HAL_BUTTON_SOFT2)
                    && selectedErrorNum < numErrors - 1) {
                selectedErrorNum++;
            }
            if (button & HAL_BUTTON_ESC) {
                return;
            }
        } while (!(button & HAL_BUTTON_SOFT1));

    }
    // reset all errors
    error.code = 0;
    screen_Clear();
    screen_FastString6x8("No errors", 0, 0);
    screen_SetSoftButton("OK", 2);
    while (!(hal_getButton() & (HAL_BUTTON_ESC | HAL_BUTTON_SOFT2)))
        ;

    while (hal_getButton())
        ;
}

void errors_Check(void) {
    /******************************************************************
     * Load drawing current while input is switched off
     *****************************************************************/
    if (!load.powerOn && (load.state.current > 100)) {
        // input is switched off but load is still drawing more than 100uA
        if (error.Duration[LOAD_ERROR_OFF_CURRENT] < 254)
            error.Duration[LOAD_ERROR_OFF_CURRENT] += 2;
    }
    /******************************************************************
     * Load drawing wrong amount of current
     *****************************************************************/
    if (load.powerOn && (load.mode == FUNCTION_CC)
            && (load.state.voltage > 500000)) {
        int32_t minCurrent = (load.current - load.current / 10) - 10000;
        int32_t maxCurrent = (load.current + load.current / 10) + 10000;
        if (load.state.current < minCurrent
                || load.state.current > maxCurrent) {
            if (error.Duration[LOAD_ERROR_WRONG_CURRENT] < 254)
                error.Duration[LOAD_ERROR_WRONG_CURRENT] += 2;
        }
    }
    /******************************************************************
     * Load not regulating to correct voltage
     *****************************************************************/
    if (load.powerOn && (load.mode == FUNCTION_CV)
            && (load.state.current > 1000)) {
        int32_t minVoltage = (load.voltage - load.voltage / 10) - 100000;
        int32_t maxVoltage = (load.voltage + load.voltage / 10) + 100000;
        if (load.state.voltage < minVoltage
                || load.state.voltage > maxVoltage) {
            if (error.Duration[LOAD_ERROR_WRONG_VOLTAGE] < 254)
                error.Duration[LOAD_ERROR_WRONG_VOLTAGE] += 2;
        }
    }
    /******************************************************************
     * Load not regulating to correct power
     *****************************************************************/
    if (load.powerOn && (load.mode == FUNCTION_CP)
            && (load.state.current > 1000 || load.state.voltage > 500000)) {
        int32_t minPower = (load.power - load.power / 5) - 100000;
        int32_t maxPower = (load.power + load.power / 5) + 100000;
        if (load.state.power < minPower || load.state.power > maxPower) {
            if (error.Duration[LOAD_ERROR_WRONG_POWER] < 254)
                error.Duration[LOAD_ERROR_WRONG_POWER] += 2;
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
        if (error.Duration[i]) {
            error.Duration[i]--;
        }
        if (error.Duration[i] > LOAD_MAX_ERROR_DURATION) {
            error.code |= bitmask;
        }
        bitmask <<= 1;
    }
}
