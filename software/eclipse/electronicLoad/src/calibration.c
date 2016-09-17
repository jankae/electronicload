/**
 * \file
 * \brief   Calibration source file.
 *
 * This file contains calibration values and functions to calibrate
 * the load and store the calibration values
 */
#include "calibration.h"

/**
 * \brief Transfers the calibration values from the end of the FLASH
 *
 * (see linker script, section '.config') into the RAM
 * \return 0 on success, 1 on failure
 */
uint8_t cal_readFromFlash(void) {
    // check whether there is any calibration data in FLASH
    if (*(uint32_t*) FLASH_VALID_CALIB_INDICATOR == CAL_INDICATOR) {
        // copy memory section from FLASH into RAM
        // (depends on both sections being identically)
        uint8_t i;
        uint32_t *from = (uint32_t*) FLASH_CALIBRATION_DATA;
        uint32_t *to = (uint32_t*) &calibration;
        uint8_t words = (sizeof(calibration) + 3) / 4;
        for (i = 0; i < words; i++) {
            *to = *from;
            to++;
            from++;
        }
        calibration.active = 0;
        return 0;
    }
    return 1;
}

/**
 * \brief Writes the calibration values from the RAM into the end
 * of the FLASH.
 *
 * Use sparsely to preserve FLASH
 */
void cal_writeToFlash(void) {
    FLASH_Unlock();
    FLASH_ClearFlag(
    FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
    FLASH_ErasePage(0x0807F000);
    if (sizeof(calibration) >= 0x400)
        FLASH_ErasePage(0x0807F400);
    if (sizeof(calibration) >= 0x800)
        FLASH_ErasePage(0x0807F800);
    if (sizeof(calibration) >= 0xC00)
        FLASH_ErasePage(0x0807FC00);
    // FLASH is ready to be written at this point
    uint8_t i;
    uint32_t *from = (uint32_t*) &calibration;
    uint32_t *to = (uint32_t*) FLASH_CALIBRATION_DATA;
    uint8_t words = (sizeof(calibration) + 3) / 4;
    for (i = 0; i < words; i++) {
        FLASH_ProgramWord((uint32_t) to, *from);
        to++;
        from++;
    }
    // set valid data indicator
    FLASH_ProgramWord((uint32_t) FLASH_VALID_CALIB_INDICATOR, 0x01);
    FLASH_Lock();
}

int32_t cal_sampleMeter(uint8_t samples) {
    uint8_t meterStable = 0;
    int8_t direction = 0;
    uint32_t lastTimeout = 0;
    int32_t lastData = INT32_MIN;
    do {
        // wait for new data from meter
        while (meter.timeout == lastTimeout) {
            if (timer_TimeoutElapsed(meter.timeout))
                // abort sampling on meter error
                return 0;
        }
        lastTimeout = meter.timeout;
        if (meter.value == lastData) {
            meterStable = 1;
        } else if (lastData != INT32_MIN) {
            // this was not the first sampled value
            // -> detect direction
            if (meter.value > lastData) {
                if (direction == -1) {
                    // direction has changed
                    // -> assume meter has stabilized
                    meterStable = 1;
                } else {
                    direction = 1;
                }
            } else {
                if (direction == 1) {
                    // direction has changed
                    // -> assume meter has stabilized
                    meterStable = 1;
                } else {
                    direction = -1;
                }
            }
        }
        lastData = meter.value;
    } while (!meterStable);
    // meter has stabilized
    // -> sample values
    int32_t valueSum = meter.value;
    uint8_t i;
    for (i = 1; i < samples; i++) {
        // wait for new data from meter
        while (meter.timeout == lastTimeout) {
            if (timer_TimeoutElapsed(meter.timeout))
                // abort sampling on meter error
                return 0;
        }
        lastTimeout = meter.timeout;
        valueSum += meter.value;
    }
    valueSum /= samples;
    return valueSum;
}

/**
 * \brief Sets the calibration values to the default values.
 *
 * Should be used in case of missing calibration data.
 */
void cal_setDefaultCalibration(void) {
    calibration.currentSetTable[0][0] = 0;
    calibration.currentSetTable[0][1] = 0;
    calibration.currentSetTable[1][0] = 65536;
    calibration.currentSetTable[1][1] = 204088;

    calibration.currentSenseTable[0][0] = 0;
    calibration.currentSenseTable[0][1] = 0;
    calibration.currentSenseTable[1][0] = 65536;
    calibration.currentSenseTable[1][1] = 198759;

    calibration.voltageSetTable[0][0] = 0;
    calibration.voltageSetTable[0][1] = 0;
    calibration.voltageSetTable[1][0] = 65535;
    calibration.voltageSetTable[1][1] = 105025641; //4.096V/(3.9V/100V)

    calibration.voltageSenseTable[0][0] = 0;
    calibration.voltageSenseTable[0][1] = 0;
    calibration.voltageSenseTable[1][0] = 65535;
    calibration.voltageSenseTable[1][1] = 105025641; //4.096V/(3.9V/100V)

    calibration.shuntFactor = 10000;
}

void calibrationMenu(void) {
    char *entries[4];
    int8_t sel;
    do {
        char automatic[21] = "Automatic Cal.";
        entries[0] = automatic;

        char manual[21] = "Manual Cal.";
        entries[1] = manual;

        char info[21] = "Multimeter info";
        entries[2] = info;

        char hardware[21] = "Hardware Cal.";
        entries[3] = hardware;

        sel = menu_ItemChooseDialog("\xCD\xCD" "CALIBRATIONS MENU\xCD\xCD",
                entries, 4);
        switch (sel) {
        case 0:
            calibrationProcessAutomatic();
            break;
        case 1:
            calibrationProcessManual();
            break;
        case 2:
            calibrationDisplayMultimeterInfo();
            break;
        case 3:
            calibrationProcessHardware();
            break;
        }
    } while (sel >= 0);
}

void calibrationProcessAutomatic(void) {
    uint32_t button;
    uint8_t setupOK;
    while (hal_getButton())
        ;
    calibration.active = 1;

    uint8_t errorIndicator = 0;

    /****************************************
     * Step 1: low range current calibration
     * Two point calibration, calibrate
     * ADC + DAC by comparing with multimeter
     ***************************************/
    do {
        hal_SetControlMode(HAL_MODE_CC);
        hal_SelectShunt(HAL_SHUNT_1R);
        hal_setDAC(0);
        // show setup
        screen_Clear();
        screen_FastString6x8("Connect a 10V 300mA", 0, 0);
        screen_FastString6x8("PSU and the meter in", 0, 1);
        screen_FastString6x8("series in the mA", 0, 2);
        screen_FastString6x8("range.", 0, 3);

        screen_SetSoftButton("Abort", 0);

        do {
            button = hal_getButton();
            if (button & HAL_BUTTON_ESC) {
                calibration.active = 0;
                return;
            }
            setupOK = 1;
            if (!timer_TimeoutElapsed(meter.timeout)) {
                int32_t voltage = cal_getUncalibVoltage();
                if (meter.function != UT61E_FUNCTION_CURRENT_MA) {
                    screen_FastString6x8("!Switch to mA range!", 0, 5);
                    setupOK = 0;
                } else if (!meter.DC) {
                    screen_FastString6x8("!Switch to DC!      ", 0, 5);
                    setupOK = 0;
                } else if (!meter.AUTO) {
                    screen_FastString6x8("!Switch to AUTO!    ", 0, 5);
                    setupOK = 0;
                } else if (voltage > 11000 || voltage < 9000) {
                    screen_FastString6x8("!Apply 10V!         ", 0, 5);
                    setupOK = 0;
                }
            } else {
                // no meter connected
                screen_FastString6x8("!No meter connected!", 0, 5);
                setupOK = 0;
            }
            if (setupOK) {
                screen_SetSoftButton("Start", 2);
            }
            timer_waitms(100);
        } while (!(button & HAL_BUTTON_ENTER) || !setupOK);

        screen_Clear();
        hal_SelectADCChannel(HAL_ADC_CURRENT);
        uint32_t i;
        errorIndicator = 0;
        // set DAC to first calibration point
        screen_FastString6x8("Setting to 10%...", 0, 0);
        hal_setDAC(HAL_DAC_MAX * 0.1);
        calibration.currentSetTable[0][0] = HAL_DAC_MAX * 0.1;
        screen_FastString6x8("Sampling meter...", 0, 1);
        calibration.currentSetTable[0][1] = cal_sampleMeter(10);
        screen_FastString6x8("Sampling ADC...", 0, 2);
        calibration.currentSenseTable[0][0] = hal_getADC(100);
        calibration.currentSenseTable[0][1] = calibration.currentSetTable[0][1];
        // set DAC to second calibration point
        screen_FastString6x8("Setting to 90%...", 0, 3);
        hal_setDAC(HAL_DAC_MAX * 0.9);
        calibration.currentSetTable[1][0] = HAL_DAC_MAX * 0.9;
        screen_FastString6x8("Sampling meter...", 0, 4);
        calibration.currentSetTable[1][1] = cal_sampleMeter(10);
        screen_FastString6x8("Sampling ADC...", 0, 5);
        calibration.currentSenseTable[1][0] = hal_getADC(100);
        calibration.currentSenseTable[1][1] = calibration.currentSetTable[1][1];
        // check values for plausibility
        if (calibration.currentSetTable[0][1]
                >= calibration.currentSetTable[1][1]) {
            errorIndicator = CAL_ERROR_METER_MONOTONIC;
            cal_DisplayError(errorIndicator);
            break;
        }
        if (calibration.currentSenseTable[0][0]
                >= calibration.currentSenseTable[1][0]) {
            errorIndicator = CAL_ERROR_ADC_MONOTONIC;
            cal_DisplayError(errorIndicator);
            break;
        }
        // set current back to zero
        hal_setDAC(0);

    } while (errorIndicator);

    /****************************************
     * Step 2: measure factor between the
     * two shunts by applying the same DAC
     * value to both of them
     ***************************************/
    errorIndicator = 0;
    do {
        screen_Clear();
        screen_FastString6x8("set 1% of high shunt", 0, 0);
        // set current to about 2mA
        uint16_t dac1percent = common_Map(2000,
                calibration.currentSetTable[0][1],
                calibration.currentSetTable[1][1],
                calibration.currentSetTable[0][0],
                calibration.currentSetTable[1][1]);
        hal_setDAC(dac1percent);
        screen_FastString6x8("Sampling meter...", 0, 1);
        int32_t currentLow = cal_sampleMeter(10);
        hal_setDAC(0);
        //switch to different shunt
        hal_SelectShunt(HAL_SHUNT_R01);
        // set DAC to same value as before (which is about 200mA now)
        screen_FastString6x8("set 1% of low shunt", 0, 2);
        hal_setDAC(dac1percent);
        screen_FastString6x8("Sampling meter...", 0, 3);
        int32_t currentHigh = cal_sampleMeter(10);
        hal_setDAC(0);
        //switch to different shunt
        hal_SelectShunt(HAL_SHUNT_1R);

        // calculate factor between the shunts
        calibration.shuntFactor = currentHigh * 100 / currentLow;
        // check for plausibility
        if (calibration.shuntFactor < 8000 || calibration.shuntFactor > 12000) {
            errorIndicator = CAL_ERROR_SHUNTFACTOR;
            cal_DisplayError(errorIndicator);
            break;
        }
    } while (errorIndicator);

    /****************************************
     * Step 3: voltage calibration
     * Two point calibration, calibrate
     * ADC + DAC by comparing with multimeter
     ***************************************/
    errorIndicator = 0;
    do {
        hal_SetControlMode(HAL_MODE_CV);
        hal_SelectShunt(HAL_SHUNT_1R);
        hal_setDAC(HAL_DAC_MAX);
        // show setup
        screen_Clear();
        screen_FastString6x8("Connect a PSU with", 0, 0);
        screen_FastString6x8("at least 30V via", 0, 1);
        screen_FastString6x8("a 1k resistor. Use", 0, 2);
        screen_FastString6x8("the meter in the 'V'", 0, 3);
        screen_FastString6x8("range at the load.", 0, 4);

        screen_SetSoftButton("Abort", 0);

        do {
            button = hal_getButton();
            if (button & HAL_BUTTON_ESC) {
                calibration.active = 0;
                return;
            }
            setupOK = 1;
            if (!timer_TimeoutElapsed(meter.timeout)) {
                int32_t voltage = cal_getUncalibVoltage();
                if (meter.function != UT61E_FUNCTION_VOLTAGE) {
                    screen_FastString6x8("!Switch to V range! ", 0, 5);
                    setupOK = 0;
                } else if (!meter.DC) {
                    screen_FastString6x8("!Switch to DC!      ", 0, 5);
                    setupOK = 0;
                } else if (!meter.AUTO) {
                    screen_FastString6x8("!Switch to AUTO!    ", 0, 5);
                    setupOK = 0;
                } else if (voltage < 28000) {
                    screen_FastString6x8("!Apply at least 30V!", 0, 5);
                    setupOK = 0;
                }
            } else {
                // no meter connected
                screen_FastString6x8("!No meter connected!", 0, 5);
                setupOK = 0;
            }
            if (setupOK) {
                screen_SetSoftButton("Start", 2);
            }
            timer_waitms(100);
        } while (!(button & HAL_BUTTON_ENTER) || !setupOK);

        screen_Clear();
        hal_SelectADCChannel(HAL_ADC_VOLTAGE);
        uint32_t i;
        errorIndicator = 0;
        // set DAC to first calibration point
        screen_FastString6x8("Setting to 20V...", 0, 0);
        hal_setDAC(12480);
        calibration.voltageSetTable[0][0] = 12480;
        screen_FastString6x8("Sampling meter...", 0, 1);
        calibration.voltageSetTable[0][1] = cal_sampleMeter(10);
        screen_FastString6x8("Sampling ADC...", 0, 2);
        calibration.voltageSenseTable[0][0] = hal_getADC(100);
        calibration.voltageSenseTable[0][1] = calibration.voltageSetTable[1][1];
        // set DAC to second calibration point
        screen_FastString6x8("Setting to 10V...", 0, 3);
        hal_setDAC(6240);
        calibration.voltageSetTable[1][0] = 6240;
        screen_FastString6x8("Sampling meter...", 0, 4);
        calibration.voltageSetTable[1][1] = cal_sampleMeter(10);
        screen_FastString6x8("Sampling ADC...", 0, 5);
        calibration.voltageSenseTable[1][0] = hal_getADC(100);
        calibration.voltageSenseTable[1][1] = calibration.voltageSetTable[1][1];
        // check values for plausibility
        if (calibration.voltageSetTable[0][1]
                <= calibration.voltageSetTable[1][1]) {
            errorIndicator = CAL_ERROR_METER_MONOTONIC;
            cal_DisplayError(errorIndicator);
            break;
        }
        if (calibration.voltageSenseTable[0][0]
                <= calibration.voltageSenseTable[1][0]) {
            errorIndicator = CAL_ERROR_ADC_MONOTONIC;
            cal_DisplayError(errorIndicator);
            break;
        }
        // set current back to zero
        hal_SetControlMode(HAL_MODE_CC);
        hal_setDAC(0);

    } while (errorIndicator);

//    // save calibration values in FLASH
//    cal_writeToFlash();

    while (hal_getButton())
        ;

    calibration.active = 0;
}

void cal_DisplayError(uint8_t error) {
    screen_Clear();
    screen_FastString12x16("ERROR", 34, 0);
    switch (error) {
    case CAL_ERROR_ADC_MONOTONIC:
        screen_FastString6x8("ADC values not", 0, 3);
        screen_FastString6x8("strictly monotonic.", 0, 4);
        break;
    case CAL_ERROR_METER_MONOTONIC:
        screen_FastString6x8("Meter values not", 0, 2);
        screen_FastString6x8("strictly monotonic.", 0, 3);
        screen_FastString6x8("(Probable a DAC or", 0, 4);
        screen_FastString6x8("int. control problem)", 0, 5);
        break;
    case CAL_ERROR_SHUNTFACTOR:
        screen_FastString6x8("High deviation from", 0, 2);
        screen_FastString6x8("expected value.", 0, 2);
        break;
    }
    screen_SetSoftButton("Retry", 0);
    while (!(hal_getButton() & HAL_BUTTON_SOFT0))
        ;
    while (hal_getButton())
        ;
}

/**
 * \brief Starts and executes the calibration process.
 *
 * Stand-alone function, start from main thread, depends on interrupts
 * to update display and  to get user inputs.
 * IMPORTANT: disable all functions dealing with the DAC, this is all
 * done internally in this function
 */
void calibrationProcessManual(void) {

}

void calibrationProcessHardware(void) {
    calibration.active = 1;
    uint32_t button;
    while (hal_getButton())
        ;
    hal_SetControlMode(HAL_MODE_CC);
    hal_SelectShunt(HAL_SHUNT_1R);
    hal_setDAC(0);
    /****************************************
     * Step 1: Preparation
     ***************************************/
    screen_Clear();
    screen_FastString6x8("Disconnect everything", 0, 0);
    screen_FastString6x8("from the input and", 0, 1);
    screen_FastString6x8("open the top cover.", 0, 2);
    screen_SetSoftButton("Abort", 0);
    screen_SetSoftButton("OK", 2);
    do {
        button = hal_getButton();
        if (button & HAL_BUTTON_SOFT0)
            return;
    } while (!(button & HAL_BUTTON_SOFT2));
    while (hal_getButton())
        ;
    /****************************************
     * Step 2: Mosfet driver bias current
     ***************************************/
    screen_Clear();
    screen_FastString6x8("Adjust R11 for 10mV", 0, 0);
    screen_FastString6x8("across TRIM header on", 0, 1);
    screen_FastString6x8("analogBoard.", 0, 2);
    screen_SetSoftButton("Abort", 0);
    screen_SetSoftButton("OK", 2);
    do {
        button = hal_getButton();
        if (button & HAL_BUTTON_SOFT0)
            return;
    } while (!(button & HAL_BUTTON_SOFT2));
    while (hal_getButton())
        ;
    /****************************************
     * Step 3: Offset voltage of control amp
     ***************************************/
    screen_Clear();
    screen_FastString6x8("Adjust R26 on", 0, 0);
    screen_FastString6x8("analogControlBoard", 0, 1);
    screen_FastString6x8("for 0V at TRIM header", 0, 2);
    screen_FastString6x8("on analogBoard with", 0, 3);
    screen_FastString6x8("respect to ground.", 0, 4);
    screen_SetSoftButton("Abort", 0);
    screen_SetSoftButton("OK", 2);
    do {
        button = hal_getButton();
        if (button & HAL_BUTTON_SOFT0)
            return;
    } while (!(button & HAL_BUTTON_SOFT2));
    while (hal_getButton())
        ;
    /****************************************
     * Step 4: Offset Y at multiplier
     ***************************************/
    screen_Clear();
    screen_FastString6x8("Adjust R18 on", 0, 0);
    screen_FastString6x8("analogControlBoard", 0, 1);
    screen_FastString6x8("for minimal Vpp at", 0, 2);
    screen_FastString6x8("M_OUT on", 0, 3);
    screen_FastString6x8("analogControlBoard.", 0, 4);
    screen_SetSoftButton("Abort", 0);
    screen_SetSoftButton("OK", 2);
    hal_SetControlMode(HAL_MODE_CR);
    uint32_t DACtoggle = timer_SetTimeout(10);
    uint16_t DACvalue = 0;
    do {
        if (timer_TimeoutElapsed(DACtoggle)) {
            DACtoggle = timer_SetTimeout(10);
            DACvalue ^= 0xFFFF;
            hal_setDAC(DACvalue);
        }
        button = hal_getButton();
        if (button & HAL_BUTTON_SOFT0)
            return;
    } while (!(button & HAL_BUTTON_SOFT2));
    hal_setDAC(0);
    while (hal_getButton())
        ;
    /****************************************
     * Step 5: Offset X at multiplier
     ***************************************/
    screen_Clear();
    screen_FastString6x8("Apply a signal at the", 0, 0);
    screen_FastString6x8("input with a large", 0, 1);
    screen_FastString6x8("Vpp. Adjust R2 on the", 0, 2);
    screen_FastString6x8("analogControlBoard", 0, 3);
    screen_FastString6x8("for minimal Vpp at", 0, 4);
    screen_FastString6x8("M_OUT.", 0, 5);
    screen_SetSoftButton("Abort", 0);
    screen_SetSoftButton("OK", 2);
    do {
        button = hal_getButton();
        if (button & HAL_BUTTON_SOFT0)
            return;
    } while (!(button & HAL_BUTTON_SOFT2));
    while (hal_getButton())
        ;
    /****************************************
     * Step 6: Offset Z at multiplier
     ***************************************/
    screen_Clear();
    screen_FastString6x8("Adjust R25 on", 0, 0);
    screen_FastString6x8("analogControlBoard", 0, 1);
    screen_FastString6x8("for minimal voltage", 0, 2);
    screen_FastString6x8("at M_OUT with", 0, 3);
    screen_FastString6x8("respect to ground.", 0, 4);
    screen_SetSoftButton("Abort", 0);
    screen_SetSoftButton("OK", 2);
    do {
        button = hal_getButton();
        if (button & HAL_BUTTON_SOFT0)
            return;
    } while (!(button & HAL_BUTTON_SOFT2));
    // set default mode
    hal_SetControlMode(HAL_MODE_CC);
    hal_SelectShunt(HAL_SHUNT_1R);
    hal_setDAC(0);
    while (hal_getButton())
        ;
    calibration.active = 0;
}

void calibrationDisplayMultimeterInfo(void) {
// loop while ESC is not pressed
    do {
        screen_Clear();
        screen_FastString6x8("\xCD\xCD\xCDMULTIMETER INFO\xCD\xCD\xCD", 0, 0);
        if (timer_TimeoutElapsed(meter.timeout)) {
            // no data received for some time
            screen_FastString6x8("No Meter detected", 6, 2);
        } else {
            screen_FastString6x8("Meter detected:", 6, 1);
            screen_FastString6x8("Chipset: ES51922", 6, 2);
            screen_FastString6x8("Mode:", 6, 3);
            uint8_t validMode = 1;
            switch (meter.function) {
            case UT61E_FUNCTION_VOLTAGE:
                screen_FastString6x8("Voltage", 42, 3);
                break;
            case UT61E_FUNCTION_CURRENT_UA:
                screen_FastString6x8("Current uA", 42, 3);
                break;
            case UT61E_FUNCTION_CURRENT_MA:
                screen_FastString6x8("Current mA", 42, 3);
                break;
            case UT61E_FUNCTION_CURRENT_A:
                screen_FastString6x8("Current A", 42, 3);
                break;
            default:
                screen_FastString6x8("Unknown", 42, 3);
                validMode = 0;
                break;
            }
            if (validMode) {
                if (meter.DC) {
                    screen_FastString6x8("DC", 6, 4);
                } else {
                    screen_FastString6x8("AC", 6, 4);
                }
                if (meter.AUTO) {
                    screen_FastString6x8("Auto", 60, 4);
                } else {
                    screen_FastString6x8("Manual", 60, 4);
                }
                char buf[11];
                uint32_t value = meter.value > 0 ? meter.value : -meter.value;
                string_fromUint(value, buf, 9, 6);
                screen_FastString12x16(buf, 8, 5);
                if (meter.value < 0) {
                    screen_HorizontalLine(0, 46, 7);
                    screen_HorizontalLine(0, 47, 7);
                }
            }
            screen_FastString6x8("ESC: Back", 0, 7);
        }
        timer_waitms(100);
    } while (!(hal_getButton() & HAL_BUTTON_ESC));
// wait for all buttons to be released
    while (hal_getButton())
        ;
}

/**
 * \brief Sets the 'should be'-current
 *
 * \param mA Current the load should draw
 */
void cal_setCurrent(uint32_t uA) {
    if (uA >= settings.maxCurrent[settings.powerMode]) {
        uA = settings.maxCurrent[settings.powerMode];
    }
    if (settings.powerMode) {
        // low shunt active (high power mode)
        uA = ((int64_t) uA * 100) / calibration.shuntFactor;
    }
    int32_t dac = common_Map(uA, calibration.currentSetTable[1][0],
            calibration.currentSetTable[1][1],
            calibration.currentSetTable[0][0],
            calibration.currentSetTable[0][1]);
    if (dac < 0)
        dac = 0;
    else if (dac > HAL_DAC_MAX)
        dac = HAL_DAC_MAX;
    hal_setDAC(dac);
}

/**
 * \brief Returns the current being drawn
 *
 * \return Current in mA
 */
int32_t cal_getCurrent(void) {
    hal_SelectADCChannel(HAL_ADC_CURRENT);
    int32_t raw = hal_getADC(1);
    int32_t current = common_Map(raw, calibration.currentSenseTable[0][0],
            calibration.currentSenseTable[0][1],
            calibration.currentSenseTable[1][0],
            calibration.currentSenseTable[1][1]);
    if (settings.powerMode) {
        current = ((int64_t) current * calibration.shuntFactor) / 100;
    }
    return current;
}

/**
 * \brief Returns the voltage at the terminals
 *
 * \return Voltage in mV
 */
int32_t cal_getVoltage(void) {
    hal_SelectADCChannel(HAL_ADC_VOLTAGE);
    int32_t raw = hal_getADC(1);
    int32_t voltage = common_Map(raw, calibration.voltageSenseTable[0][0],
            calibration.voltageSenseTable[0][1],
            calibration.voltageSenseTable[1][0],
            calibration.voltageSenseTable[1][1]);
    return voltage;
}

int32_t cal_getUncalibVoltage(void) {
    // voltage is provided to ADC at 3.9V/100V
    // reference voltage is 4.096V, ADC resolution is 16bits
    // -> multiplying by 1602 roughly results in uV
    hal_SelectADCChannel(HAL_ADC_VOLTAGE);
    return (int32_t) hal_getADC(1) * 1602;
}
int32_t cal_getUncalibCurrent(void) {
    hal_SelectADCChannel(HAL_ADC_VOLTAGE);

    switch (settings.powerMode) {
    case 0:
        // current is provided to ADC at 2V/100mA
        // reference voltage is 4.096V, ADC resolution is 16bits
        // -> multiplying by 3.1875 roughly results in uV
        return ((int32_t) hal_getADC(1) * 51) / 16;
        break;
    case 1:
        // current is provided to ADC at 2V/10A
        // reference voltage is 4.096V, ADC resolution is 16bits
        // -> multiplying by 318.75 roughly results in uV
        return ((int32_t) hal_getADC(1) * 1275) / 4;
        break;
    default:
        return 0;
        break;
    }
}

uint8_t cal_getTemp1(void) {
    return hal_ReadTemperature(HAL_TEMP1);
}

uint8_t cal_getTemp2(void) {
    return hal_ReadTemperature(HAL_TEMP2);
}
