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
        uint32_t *to = (uint32_t*) &calData;
        uint8_t words = (sizeof(calData) + 3) / 4;
        for (i = 0; i < words; i++) {
            *to = *from;
            to++;
            from++;
        }
        cal.active = 0;
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
    if (sizeof(calData) >= 0x400)
        FLASH_ErasePage(0x0807F400);
    if (sizeof(calData) >= 0x800)
        FLASH_ErasePage(0x0807F800);
    if (sizeof(calData) >= 0xC00)
        FLASH_ErasePage(0x0807FC00);
    // FLASH is ready to be written at this point
    uint8_t i;
    uint32_t *from = (uint32_t*) &calData;
    uint32_t *to = (uint32_t*) FLASH_CALIBRATION_DATA;
    uint8_t words = (sizeof(calData) + 3) / 4;
    for (i = 0; i < words; i++) {
        FLASH_ProgramWord((uint32_t) to, *from);
        to++;
        from++;
    }
    // set valid data indicator
    FLASH_ProgramWord((uint32_t) FLASH_VALID_CALIB_INDICATOR, CAL_INDICATOR);
    FLASH_Lock();
    cal.unsavedData = 0;
}

/**
 * \brief Retrieves the real current/voltage during calibration
 *
 * The function checks whether a multimeter in the correct range is
 * connected. If so, the multimeter value is returned. If not, the user
 * is prompted for the value.
 * Deviations from the approximate value results in an error.
 * \param unit Can be either CAL_VALUE_CURRENT or CAL_VALUE_VOLTAGE
 * \param approxValue guessed approximate value
 */
int32_t cal_GetRealValue(uint8_t unit, int32_t approxValue) {
    uint8_t useMeter = 0;
    int32_t returnValue = 0;
    if (!timer_TimeoutElapsed(meter.timeout)) {
        // meter connected, check for correct mode
        if (unit == CAL_VALUE_CURRENT && meter.function ==
        UT61E_FUNCTION_CURRENT_MA && meter.AUTO) {
            useMeter = 1;
        } else if (unit == CAL_VALUE_VOLTAGE
                && meter.function == UT61E_FUNCTION_VOLTAGE && meter.AUTO) {
            useMeter = 1;
        }
    }
    if (useMeter) {
        returnValue = cal_sampleMeter(CAL_METER_NSAMPLES);
    } else {
        if (unit == CAL_VALUE_CURRENT) {
            menu_getInputValue(&returnValue, "Actual current", 0,
                    approxValue * 2, "uA", "mA", "A");
        } else {
            menu_getInputValue(&returnValue, "Actual voltage", 0,
                    approxValue * 2, "uV", "mV", "V");
        }
    }
    if (returnValue < approxValue * 0.8 || returnValue > approxValue * 1.2) {
        // high deviation from expected value
        screen_Clear();
        screen_FastString12x16("WARNING", 0, 0);
        screen_Text6x8("Actual value deviates from expected value."
                " Calibration might be off.", 0, 2);
        screen_SetSoftButton("OK", 2);
        while (!(hal_getButton() & HAL_BUTTON_SOFT2))
            ;
        while (hal_getButton())
            ;
    }
    return returnValue;
}

int32_t cal_sampleMeter(uint8_t samples) {
    uint8_t meterStable = 0;
    int8_t direction = 0;
    uint32_t lastTimeout = 0;
    int32_t lastData = INT32_MIN;
    screen_Clear();
    screen_FastString6x8("Sampling meter...", 0, 0);
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
    screen_FastString6x8("Meter stabilized.", 0, 1);
    screen_FastString6x8("Taking samples...", 0, 2);
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
    screen_FastString6x8("...done", 0, 3);
    screen_FastString12x16("Result:", 0, 4);
    char buf[11];
    if (meter.function == UT61E_FUNCTION_VOLTAGE) {
        string_fromUintUnit(valueSum, buf, 5, 6, 'V');
    } else {
        // meter must be in current mode (only voltage and current supported)
        string_fromUintUnit(valueSum, buf, 5, 6, 'A');
    }
    screen_FastString12x16(buf, 0, 6);
    timer_waitms(1500);
    return valueSum;
}

int32_t cal_sampleADC(uint16_t samples, uint16_t *ADCdata) {
    uint16_t i;
    uint32_t buf = *ADCdata;
    for (i = 1; i < samples; i++) {
        timer_waitms(1);
        buf += *ADCdata;
    }
    return buf / samples;
}

/**
 * \brief Sets the calibration values to the default values.
 *
 * Should be used in case of missing calibration data.
 */
void cal_setDefaultCalibration(void) {
    calData.currentSetTable[0][0] = 0;
    calData.currentSetTable[0][1] = 0;
    calData.currentSetTable[1][0] = 65536;
    calData.currentSetTable[1][1] = 204088;

    calData.currentSenseTable[0][0] = 0;
    calData.currentSenseTable[0][1] = 0;
    calData.currentSenseTable[1][0] = 65536;
    calData.currentSenseTable[1][1] = 217212;

    calData.voltageSetTable[0][0] = 0;
    calData.voltageSetTable[0][1] = 0;
    calData.voltageSetTable[1][0] = 65536;
    calData.voltageSetTable[1][1] = 105025641; //4.096V/(3.9V/100V)

    calData.voltageSenseTable[0][0] = 0;
    calData.voltageSenseTable[0][1] = 0;
    calData.voltageSenseTable[1][0] = 65536;
    calData.voltageSenseTable[1][1] = 105025641; //4.096V/(3.9V/100V)

    calData.powerSetTable[0][0] = 0;
    calData.powerSetTable[0][1] = 0;
    calData.powerSetTable[1][0] = 65536;
    calData.powerSetTable[1][1] = 2038557;

    calData.conductanceSetTable[0][0] = 0;
    calData.conductanceSetTable[0][1] = 0;
    calData.conductanceSetTable[1][0] = 65536;
    calData.conductanceSetTable[1][1] = 199680;

    calData.shuntFactor = 10000;
    cal.unsavedData = 0;
}

void calibrationMenu(void) {
    char *entries[6];
    uint8_t nentries;
    int8_t sel = 0;
    do {
        char current[21] = "Current Calibration";
        entries[0] = current;

        char shunt[21] = "Shunt Calibration";
        entries[1] = shunt;

        char voltage[21] = "Voltage Calibration";
        entries[2] = voltage;

        char info[21] = "Multimeter info";
        entries[3] = info;

        char hardware[21] = "Hardware Cal.";
        entries[4] = hardware;

        char save[21] = "Save data in Flash";
        if (cal.unsavedData) {
            entries[5] = save;
            nentries = 6;
        } else {
            nentries = 5;
        }

        sel = menu_ItemChooseDialog("\xCD\xCD" "CALIBRATIONS MENU\xCD\xCD",
                entries, nentries, sel);
        switch (sel) {
        case 0:
            cal_CurrentCalibration();
            break;
        case 1:
            cal_ShuntCalibration();
            break;
        case 2:
            cal_VoltageCalibration();
            break;
        case 3:
            calibrationDisplayMultimeterInfo();
            break;
        case 4:
            calibrationProcessHardware();
            break;
        case 5:
            cal_writeToFlash();
        }
    } while (sel >= 0);
}

void cal_CurrentCalibration(void) {
    uint32_t button;
    while (hal_getButton())
        ;

    cal.active = 1;
    settings.powerMode = 0;
    load.powerOn = 0;
    load.DACoverride = 0;
    load.mode = FUNCTION_CC;
// show setup
    screen_Clear();
    screen_Text6x8("Connect a 10V 300mA"
            " PSU and the meter in"
            " series in the mA range.", 0, 0);

    screen_SetSoftButton("Abort", 0);
    screen_SetSoftButton("Start", 2);

    do {
        button = hal_getButton();
        if (button & (HAL_BUTTON_SOFT0 | HAL_BUTTON_ESC)) {
            cal.active = 0;
            return;
        }
    } while (!(button & HAL_BUTTON_SOFT2));
    while (hal_getButton())
        ;

    screen_Clear();
    hal_SelectADCChannel(HAL_ADC_CURRENT);
    uint32_t i;
// set DAC to first calibration point
    screen_FastString6x8("Setting to 1mA...", 0, 0);
    load.DACoverride = 321;
    timer_waitms(100);
    calData.currentSetTable[0][0] = 321;
    screen_FastString6x8("Sampling ADC...", 0, 1);
    calData.currentSenseTable[0][0] = cal_sampleADC(CAL_ADC_NSAMPLES,
            &cal.rawADCcurrent);
    calData.currentSetTable[0][1] = cal_GetRealValue(CAL_VALUE_CURRENT, 1000);
    calData.currentSenseTable[0][1] = calData.currentSetTable[0][1];
    screen_Clear();
// set DAC to second calibration point
    screen_FastString6x8("Setting to 180mA...", 0, 0);
    load.DACoverride = 57800;
    timer_waitms(100);
    calData.currentSetTable[1][0] = 57800;
    screen_FastString6x8("Sampling ADC...", 0, 1);
    calData.currentSenseTable[1][0] = cal_sampleADC(CAL_ADC_NSAMPLES,
            &cal.rawADCcurrent);
    calData.currentSetTable[1][1] = cal_GetRealValue(CAL_VALUE_CURRENT, 180000);
    calData.currentSenseTable[1][1] = calData.currentSetTable[1][1];
// set current back to zero
    load.DACoverride = 0;
// check values for plausibility
    if (calData.currentSetTable[0][1] >= calData.currentSetTable[1][1]) {
        cal_DisplayError(CAL_ERROR_METER_MONOTONIC);
    }
    if (calData.currentSenseTable[0][0] >= calData.currentSenseTable[1][0]) {
        cal_DisplayError(CAL_ERROR_ADC_MONOTONIC);
    }
    cal.unsavedData = 1;
    cal.active = 0;
}

void cal_ShuntCalibration(void) {
    uint32_t button;
    while (hal_getButton())
        ;

    cal.active = 1;
    settings.powerMode = 0;
    load.powerOn = 0;
    load.DACoverride = 0;
    load.mode = FUNCTION_CC;
// show setup
    screen_Clear();
    screen_Text6x8("Connect a 10V 300mA"
            " PSU and the meter in"
            " series in the mA range.", 0, 0);

    screen_SetSoftButton("Abort", 0);
    screen_SetSoftButton("Start", 2);

    do {
        button = hal_getButton();
        if (button & (HAL_BUTTON_SOFT0 | HAL_BUTTON_ESC)) {
            cal.active = 0;
            return;
        }
    } while (!(button & HAL_BUTTON_SOFT2));
    while (hal_getButton())
        ;

    screen_Clear();
    hal_SelectADCChannel(HAL_ADC_CURRENT);
    uint32_t i;
// set DAC to first calibration point
    screen_FastString6x8("Setting to 2mA...", 0, 0);
    int32_t dac = common_Map(2000, calData.currentSetTable[0][1],
            calData.currentSetTable[1][1], calData.currentSetTable[0][0],
            calData.currentSetTable[1][0]);
    if (dac < 0)
        dac = 0;
    load.DACoverride = dac;
    timer_waitms(100);
    int32_t currentLow = cal_GetRealValue(CAL_VALUE_CURRENT, 2000);
    screen_Clear();
// set DAC to second calibration point
    screen_FastString6x8("Setting to 200mA...", 0, 0);
    settings.powerMode = 1;
    timer_waitms(100);
    int32_t currentHigh = cal_GetRealValue(CAL_VALUE_CURRENT, 200000);
// set current back to zero
    load.DACoverride = 0;
// check values for plausibility
    if (currentLow >= currentHigh) {
        cal_DisplayError(CAL_ERROR_METER_MONOTONIC);
    }
// calculate factor between the shunts
    calData.shuntFactor = currentHigh * 100 / currentLow;
    cal.unsavedData = 1;
    settings.powerMode = 0;
    cal.active = 0;
}

void cal_VoltageCalibration(void) {
    uint32_t button;
    while (hal_getButton())
        ;

    cal.active = 1;
    settings.powerMode = 0;
    load.powerOn = 0;
    load.DACoverride = 0;
    load.mode = FUNCTION_CC;
// show setup
    screen_Clear();
    screen_Text6x8("Connect a 30V 40mA"
            " PSU with a 1k resistor in series"
            " and the meter in parallel"
            " in the V range.", 0, 0);

    screen_SetSoftButton("Abort", 0);
    screen_SetSoftButton("Start", 2);

    do {
        button = hal_getButton();
        if (button & (HAL_BUTTON_SOFT0 | HAL_BUTTON_ESC)) {
            cal.active = 0;
            return;
        }
    } while (!(button & HAL_BUTTON_SOFT2));
    while (hal_getButton())
        ;

// check voltage
    if (load.state.voltage < 29500000 || load.state.voltage > 32000000) {
        screen_Clear();
        screen_FastString12x16("ERROR", 34, 0);
        screen_Text6x8("Incorrect voltage applied."
                " Check setup and repeat", 0, 2);
        screen_SetSoftButton("OK", 2);
        while (!(hal_getButton() & HAL_BUTTON_SOFT2))
            ;
        while (hal_getButton())
            ;
        cal.active = 0;
        return;
    }

// try to draw 50mA
// if this is possible, the setup is incorrect
    load.DACoverride = 16055;
    timer_waitms(25);
    if (load.state.current > 45000) {
        // actual current is more than 45mA
        // -> error in setup
        load.DACoverride = 0;
        screen_Clear();
        screen_FastString12x16("ERROR", 34, 0);
        screen_Text6x8("Load is able to draw at least 45mA."
                " Check setup and repeat", 0, 2);
        screen_SetSoftButton("OK", 2);
        while (!(hal_getButton() & HAL_BUTTON_SOFT2))
            ;
        while (hal_getButton())
            ;
        cal.active = 0;
        return;
    }

// set DAC to first voltage point
    screen_Clear();
    screen_FastString6x8("Setting to 1V...", 0, 0);
    load.DACoverride = 624;
    load.mode = FUNCTION_CV;
    calData.voltageSetTable[0][0] = 624;
    timer_waitms(500);
    screen_FastString6x8("Sampling ADC...", 0, 0);
    calData.voltageSenseTable[0][0] = cal_sampleADC(CAL_ADC_NSAMPLES,
            &cal.rawADCvoltage);
    calData.voltageSetTable[0][1] = cal_GetRealValue(CAL_VALUE_VOLTAGE,
            1000000);
    calData.voltageSenseTable[0][1] = calData.voltageSetTable[0][1];

// set DAC to second voltage point
    screen_Clear();
    screen_Text6x8("Setting to 29V...", 0, 0);
    load.DACoverride = 18096;
    calData.voltageSetTable[1][0] = 18096;
    timer_waitms(500);
    screen_FastString6x8("Sampling ADC...", 0, 0);
    calData.voltageSenseTable[1][0] = cal_sampleADC(CAL_ADC_NSAMPLES,
            &cal.rawADCvoltage);
    calData.voltageSetTable[1][1] = cal_GetRealValue(CAL_VALUE_VOLTAGE,
            29000000);
    calData.voltageSenseTable[1][1] = calData.voltageSetTable[1][1];

// set load to 0mA
    load.mode = FUNCTION_CC;
    load.DACoverride = 0;

// check values for plausibility
    if (calData.voltageSenseTable[0][1] >= calData.voltageSenseTable[1][1]) {
        cal_DisplayError(CAL_ERROR_METER_MONOTONIC);
    }
    if (calData.voltageSenseTable[0][0] >= calData.voltageSenseTable[1][0]) {
        cal_DisplayError(CAL_ERROR_ADC_MONOTONIC);
    }
    cal.unsavedData = 1;
    cal.active = 0;
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

void calibrationProcessHardware(void) {
    cal.active = 1;
    uint32_t button;
    while (hal_getButton())
        ;
    load.mode = FUNCTION_CC;
    settings.powerMode = 0;
    load.DACoverride = 0;
    uint8_t calibrationStep = 0;
    do {
        switch (calibrationStep) {
        case 0:
            /****************************************
             * Step 1: Preparation
             ***************************************/
            load.mode = FUNCTION_CC;
            settings.powerMode = 0;
            load.DACoverride = 0;
            screen_Clear();
            screen_Text6x8("Disconnect everything"
                    " from the input and"
                    " open the top cover.", 0, 0);
            screen_SetSoftButton("Abort", 0);
            screen_SetSoftButton("Next", 2);
            do {
                button = hal_getButton();
            } while (!(button & (HAL_BUTTON_SOFT2 | HAL_BUTTON_SOFT0)));
            if (button & HAL_BUTTON_SOFT0) {
                // end calibration
                calibrationStep = 0xff;
            } else {
                // next calibration step
                calibrationStep++;
            }
            while (hal_getButton())
                ;
            break;
        case 1:
            /****************************************
             * Step 2: Offset voltage of control amp
             ***************************************/
            load.mode = FUNCTION_CC;
            settings.powerMode = 0;
            load.DACoverride = 0;
            screen_Clear();
            screen_Text6x8("Adjust R26 on"
                    " analogControlBoard"
                    " for 0V at FET pin on"
                    " analogBoard with"
                    " respect to ground.", 0, 0);
            screen_SetSoftButton("Abort", 0);
            screen_SetSoftButton("Prev", 1);
            screen_SetSoftButton("Next", 2);
            do {
                button = hal_getButton();
            } while (!(button
                    & (HAL_BUTTON_SOFT2 | HAL_BUTTON_SOFT1 | HAL_BUTTON_SOFT0)));
            if (button & HAL_BUTTON_SOFT0) {
                // end calibration
                calibrationStep = 0xff;
            } else if (button & HAL_BUTTON_SOFT1) {
                // previous calibration step
                calibrationStep--;
            } else {
                // next calibration step
                calibrationStep++;
            }
            break;
        case 2:
            /****************************************
             * Step 3: Offset Y at multiplier
             ***************************************/
            load.mode = FUNCTION_CR;
            settings.powerMode = 0;
            load.DACoverride = 0;
            screen_Clear();
            screen_Text6x8("Adjust R18 on"
                    " analogControlBoard"
                    " for minimal Vpp at"
                    " TRIM header on"
                    " analogControlBoard.", 0, 0);
            screen_SetSoftButton("Abort", 0);
            screen_SetSoftButton("Next", 2);
            uint32_t DACtoggle = timer_SetTimeout(10);
            uint16_t DACvalue = 0;
            do {
                if (timer_TimeoutElapsed(DACtoggle)) {
                    DACtoggle = timer_SetTimeout(10);
                    if (DACvalue) {
                        DACvalue = 0;
                    } else {
                        DACvalue = HAL_DAC_MAX;
                    }
                    load.DACoverride = DACvalue;
                }
                button = hal_getButton();
            } while (!(button
                    & (HAL_BUTTON_SOFT2 | HAL_BUTTON_SOFT1 | HAL_BUTTON_SOFT0)));
            if (button & HAL_BUTTON_SOFT0) {
                // end calibration
                calibrationStep = 0xff;
            } else if (button & HAL_BUTTON_SOFT1) {
                // previous calibration step
                calibrationStep--;
            } else {
                // next calibration step
                calibrationStep++;
            }
            load.DACoverride = 0;
            while (hal_getButton())
                ;
            break;
        case 3:
            /****************************************
             * Step 4: Offset X at multiplier
             ***************************************/
            load.mode = FUNCTION_CR;
            settings.powerMode = 0;
            load.DACoverride = 0;
            screen_Clear();
            screen_Text6x8("Apply a large Vpp"
                    " signal at the sense"
                    " input. Adjust R2 on"
                    " the analogCtrlBoard"
                    " for minimal Vpp at"
                    " TRIM header.", 0, 0);
            screen_SetSoftButton("Abort", 0);
            screen_SetSoftButton("Prev", 1);
            screen_SetSoftButton("Next", 2);
            do {
                button = hal_getButton();
            } while (!(button
                    & (HAL_BUTTON_SOFT2 | HAL_BUTTON_SOFT1 | HAL_BUTTON_SOFT0)));
            if (button & HAL_BUTTON_SOFT0) {
                // end calibration
                calibrationStep = 0xff;
            } else if (button & HAL_BUTTON_SOFT1) {
                // previous calibration step
                calibrationStep--;
            } else {
                // next calibration step
                calibrationStep++;
            }
            break;
        case 4:
            /****************************************
             * Step 5: Offset Z at multiplier
             ***************************************/
            load.mode = FUNCTION_CR;
            settings.powerMode = 0;
            load.DACoverride = 0;
            screen_Clear();
            screen_Text6x8("Adjust R25 on"
                    " analogControlBoard"
                    " for minimal voltage"
                    " at TRIM header on"
                    " analogControlBoard.", 0, 0);
            screen_SetSoftButton("Abort", 0);
            screen_SetSoftButton("Prev", 1);
            screen_SetSoftButton("Done", 2);
            do {
                button = hal_getButton();
            } while (!(button
                    & (HAL_BUTTON_SOFT2 | HAL_BUTTON_SOFT1 | HAL_BUTTON_SOFT0)));
            if (button & HAL_BUTTON_SOFT0) {
                // end calibration
                calibrationStep = 0xff;
            } else if (button & HAL_BUTTON_SOFT1) {
                // previous calibration step
                calibrationStep--;
            } else {
                // next calibration step
                calibrationStep++;
            }
            break;
        }
    } while (calibrationStep < 5);
// set default mode
    load.mode = FUNCTION_CC;
    settings.powerMode = 0;
    load.DACoverride = 0;
    while (hal_getButton())
        ;
    cal.active = 0;
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

void cal_setCurrent(uint32_t uA) {
    if (uA > settings.maxCurrent) {
        uA = settings.maxCurrent;
    }
    if (settings.powerMode) {
        // low shunt active (high power mode)
        uA = ((int64_t) uA * 100) / calData.shuntFactor;
    }
    int32_t dac = common_Map(uA, calData.currentSetTable[0][1],
            calData.currentSetTable[1][1], calData.currentSetTable[0][0],
            calData.currentSetTable[1][0]);
    if (dac < 0)
        dac = 0;
    else if (dac > HAL_DAC_MAX)
        dac = HAL_DAC_MAX;
    hal_setDAC(dac);
}

void cal_setVoltage(uint32_t uV) {
    if (uV > settings.maxVoltage) {
        uV = settings.maxVoltage;
    } else if (uV < settings.minVoltage) {
        uV = settings.minVoltage;
    }

    int32_t dac = common_Map(uV, calData.voltageSetTable[0][1],
            calData.voltageSetTable[1][1], calData.voltageSetTable[0][0],
            calData.voltageSetTable[1][0]);
    if (dac < 0)
        dac = 0;
    else if (dac > HAL_DAC_MAX)
        dac = HAL_DAC_MAX;
    hal_setDAC(dac);
}

//void cal_setPower(uint32_t uW) {
//    if (uW > settings.maxPower[settings.powerMode]) {
//        uW = settings.maxPower[settings.powerMode];
//    }
//    if (settings.powerMode) {
//        // low shunt active (high power mode)
//        uW = ((int64_t) uW * 100) / calData.shuntFactor;
//    }
//    int32_t dac = common_Map(uW, calData.powerSetTable[0][1],
//            calData.powerSetTable[1][1], calData.powerSetTable[0][0],
//            calData.powerSetTable[1][0]);
//    if (dac < 0)
//        dac = 0;
//    else if (dac > HAL_DAC_MAX)
//        dac = HAL_DAC_MAX;
//    hal_setDAC(dac);
//}
//
//void cal_setResistance(uint32_t uR) {
//    if (uR > settings.maxResistance[settings.powerMode]) {
//        uR = settings.maxResistance[settings.powerMode];
//    } else if (uR < settings.minResistance[settings.powerMode]) {
//        uR = settings.minResistance[settings.powerMode];
//    }
//    if (settings.powerMode) {
//        // low shunt active (high power mode)
//        uR = ((int64_t) uR * calData.shuntFactor) / 100;
//    }
//    // convert resistance in conductance
//    int32_t uS = ((int64_t) 1000000000LL) / uR;
//    int32_t dac = common_Map(uS, calData.conductanceSetTable[0][1],
//            calData.conductanceSetTable[1][1],
//            calData.conductanceSetTable[0][0],
//            calData.conductanceSetTable[1][0]);
//    if (dac < 0)
//        dac = 0;
//    else if (dac > HAL_DAC_MAX)
//        dac = HAL_DAC_MAX;
//    hal_setDAC(dac);
//}

/**
 * \brief Returns the current being drawn
 *
 * \return Current in mA
 */
int32_t cal_getCurrent(void) {
    hal_SelectADCChannel(HAL_ADC_CURRENT);
    timer_waitus(10);
    cal.rawADCcurrent = hal_getADC(16);
    int32_t current = common_Map(cal.rawADCcurrent,
            calData.currentSenseTable[0][0], calData.currentSenseTable[1][0],
            calData.currentSenseTable[0][1], calData.currentSenseTable[1][1]);
    if (settings.powerMode) {
        current = ((int64_t) current * calData.shuntFactor) / 100;
    }
    if (current < 0)
        current = 0;
    return current;
}

/**
 * \brief Returns the voltage at the terminals
 *
 * \return Voltage in mV
 */
int32_t cal_getVoltage(void) {
    hal_SelectADCChannel(HAL_ADC_VOLTAGE);
    timer_waitus(10);
    cal.rawADCvoltage = hal_getADC(16);
    int32_t voltage = common_Map(cal.rawADCvoltage,
            calData.voltageSenseTable[0][0], calData.voltageSenseTable[1][0],
            calData.voltageSenseTable[0][1], calData.voltageSenseTable[1][1]);
    if (voltage < 0)
        voltage = 0;
    return voltage;
}

int32_t cal_getUncalibVoltage(void) {
// voltage is provided to ADC at 3.9V/100V
// reference voltage is 4.096V, ADC resolution is 16bits
// -> multiplying by 1602 roughly results in uV
    hal_SelectADCChannel(HAL_ADC_VOLTAGE);
    timer_waitus(10);
    return (int32_t) hal_getADC(1) * 1602;
}
int32_t cal_getUncalibCurrent(void) {
    hal_SelectADCChannel(HAL_ADC_CURRENT);
    timer_waitus(10);
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
    hal_SetControlMode(HAL_MODE_CC);
    hal_SelectShunt(HAL_SHUNT_1R);
}

uint8_t cal_getTemp1(void) {
    return hal_ReadTemperature(HAL_TEMP1);
}

uint8_t cal_getTemp2(void) {
    return hal_ReadTemperature(HAL_TEMP2);
}
