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
     * Step through current range, calibrate
     * ADC + DAC by comparing with multimeter
     ***************************************/
    do {
        hal_SetControlMode(HAL_MODE_CC);
        hal_SelectShunt(HAL_SHUNT_1R);
        hal_setDAC(0);
        // show setup
        screen_Clear();
        screen_FastString6x8("Connect like this:", 0, 0);
        // load
        screen_Rectangle(0, 14, 40, 25);
        screen_Rectangle(2, 16, 14, 23);
        // power supply
        screen_Rectangle(75, 14, 96, 25);
        screen_FastString6x8("PSU", 77, 2);
        screen_FastString6x8("10V", 98, 2);
        screen_FastString6x8("300mA", 98, 3);
        // meter
        screen_FastChar6x8(57, 4, 'A');
        screen_Circle(60, 36, 6);
        // leads
        screen_VerticalLine(37, 22, 6);
        screen_VerticalLine(82, 25, 3);
        screen_HorizontalLine(37, 28, 46);

        screen_VerticalLine(30, 22, 14);
        screen_VerticalLine(89, 25, 11);
        screen_HorizontalLine(30, 36, 24);
        screen_HorizontalLine(66, 36, 24);

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

        hal_SelectADCChannel(HAL_ADC_CURRENT);
        uint32_t i;
        errorIndicator = 0;
        for (i = 1; i <= CAL_POINTS_FULLSCALE; i++) {
            // set DAC to next calibration point
            uint16_t DACvalue = i * HAL_DAC_MAX / CAL_POINTS_FULLSCALE;
            hal_setDAC(DACvalue);
            uint32_t meterAvg = cal_sampleMeter(3);
            calibration.currentSetTableLow[i][0] = DACvalue;
            calibration.currentSetTableLow[i][1] = meterAvg;
            calibration.currentSenseTableLow[i][0] = hal_getADC(100);
            calibration.currentSenseTableLow[i][1] = meterAvg;
            if (i > 1) {
                if (calibration.currentSetTableLow[i][1]
                        <= calibration.currentSetTableLow[i - 1][1]) {
                    errorIndicator = CAL_ERROR_METER_MONOTONIC;
                    break;
                }
                if (calibration.currentSenseTableLow[i][0]
                        <= calibration.currentSenseTableLow[i - 1][0]) {
                    errorIndicator = CAL_ERROR_ADC_MONOTONIC;
                    break;
                }
            }
        }
        // set current back to zero
        hal_setDAC(0);

        cal_DisplayError(errorIndicator);

    } while (errorIndicator);
    // calculate offset by extrapolating the first two data points
    // TODO is it possible to measure this?
    calibration.currentSetTableLow[0][0] = common_Map(0,
            calibration.currentSetTableLow[1][1],
            calibration.currentSetTableLow[2][1],
            calibration.currentSetTableLow[1][0],
            calibration.currentSetTableLow[2][0]);

    calibration.currentSenseTableLow[0][1] = 0;
    calibration.currentSenseTableLow[0][0] = common_Map(0,
            calibration.currentSenseTableLow[1][1],
            calibration.currentSenseTableLow[2][1],
            calibration.currentSenseTableLow[1][0],
            calibration.currentSenseTableLow[2][0]);
    calibration.currentSenseTableLow[0][1] = 0;

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
    uint8_t i;
    switch (settings.powerMode) {
    case 0:
        // low-power mode active
        // search appropriate calibration points
        for (i = 1; i <= CAL_POINTS_FULLSCALE; i++) {
            if (calibration.currentSetTableLow[i][1] >= uA)
                break;
        }
        if (i > CAL_POINTS_FULLSCALE) {
            // value is off scale
            hal_setDAC(HAL_DAC_MAX);
        } else {
            // interpolate between calibration points
            hal_setDAC(
                    common_Map(uA, calibration.currentSetTableLow[i - 1][1],
                            calibration.currentSetTableLow[i][1],
                            calibration.currentSetTableLow[i - 1][0],
                            calibration.currentSetTableLow[i][0]));
        }
        break;
    case 1:
        // high-power mode active
        // search appropriate calibration points
        for (i = 1; i <= CAL_POINTS_FULLSCALE; i++) {
            if (calibration.currentSetTableHigh[i][1] >= uA)
                break;
        }
        if (i > CAL_POINTS_FULLSCALE) {
            // value is off scale
            hal_setDAC(HAL_DAC_MAX);
        } else {
            // interpolate between calibration points
            hal_setDAC(
                    common_Map(uA, calibration.currentSetTableHigh[i - 1][1],
                            calibration.currentSetTableHigh[i][1],
                            calibration.currentSetTableHigh[i - 1][0],
                            calibration.currentSetTableHigh[i][0]));
        }
        break;
    }
}

/**
 * \brief Returns the current being drawn
 *
 * \return Current in mA
 */
int32_t cal_getCurrent(void) {
    uint8_t i;
    hal_SelectADCChannel(HAL_ADC_CURRENT);
    int32_t raw = hal_getADC(1);
    switch (settings.powerMode) {
    case 0:
        // low-power mode active
        // search appropriate calibration points
        for (i = 1; i <= CAL_POINTS_FULLSCALE; i++) {
            if (calibration.currentSenseTableLow[i][0] >= raw)
                break;
        }
        if (i > CAL_POINTS_FULLSCALE) {
            // value is off scale
            // extrapolate from full scale
            return common_Map(raw, calibration.currentSenseTableLow[0][0],
                    calibration.currentSenseTableLow[CAL_POINTS_FULLSCALE][0],
                    calibration.currentSenseTableLow[0][1],
                    calibration.currentSenseTableLow[CAL_POINTS_FULLSCALE][1]);
        } else {
            // interpolate between calibration points
            return common_Map(raw, calibration.currentSenseTableLow[i - 1][0],
                    calibration.currentSenseTableLow[i][0],
                    calibration.currentSenseTableLow[i - 1][1],
                    calibration.currentSenseTableLow[i][1]);
        }
        break;
    case 1:
        // high-power mode active
        // search appropriate calibration points
        for (i = 1; i <= CAL_POINTS_FULLSCALE; i++) {
            if (calibration.currentSenseTableHigh[i][0] >= raw)
                break;
        }
        if (i > CAL_POINTS_FULLSCALE) {
            // value is off scale
            // extrapolate from full scale
            return common_Map(raw, calibration.currentSenseTableHigh[0][0],
                    calibration.currentSenseTableHigh[CAL_POINTS_FULLSCALE][0],
                    calibration.currentSenseTableHigh[0][1],
                    calibration.currentSenseTableHigh[CAL_POINTS_FULLSCALE][1]);
        } else {
            // interpolate between calibration points
            return common_Map(raw, calibration.currentSenseTableHigh[i - 1][0],
                    calibration.currentSenseTableHigh[i][0],
                    calibration.currentSenseTableHigh[i - 1][1],
                    calibration.currentSenseTableHigh[i][1]);
        }
        return 0;
        break;
    default:
        return 0;
        break;
    }
}

/**
 * \brief Returns the voltage at the terminals
 *
 * \return Voltage in mV
 */
int32_t cal_getVoltage(void) {
//    uint16_t biased = hal_getADC(ADC_VOLTAGE_SENSE, 1);
//    int32_t ret = 0;
//    if (hal.voltageRange == RANGE_LOW) {
//        int16_t unbiased = biased - calibration.voltageSenseOffsetLowRange;
//        ret = unbiased * calibration.voltageSenseScaleLowRange;
//    } else {
//        int16_t unbiased = biased - calibration.voltageSenseOffsetHighRange;
//        ret = unbiased * calibration.voltageSenseScaleHighRange;
//    }
//    if (biased >= 3500 && hal.voltageRange == RANGE_LOW) {
//        // low range is near limit -> switch to high range
//        hal_setVoltageGain(0);
//    } else if (biased <= 300 && hal.voltageRange == RANGE_HIGH) {
//        // high range is near limit -> switch to low range
//        hal_setVoltageGain(1);
//    }
//    if (ret < 0)
//        ret = 0;
//    return ret;
    return 10000000UL;
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
