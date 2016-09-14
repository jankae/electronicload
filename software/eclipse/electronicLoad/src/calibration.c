/**
 * \file
 * \brief   Calibration source file.
 *
 * This file contains calibration values and functions to calibrate
 * the load and store the calibration values
 */
#include "calibration.h"

/*
 * This section must fit into one flash page and thus
 * NEVER exceed 1kB
 */
//__attribute__ ((section(".config")))
//struct {
//    int16_t currentSenseOffsetLowRange;
//    float currentSenseScaleLowRange;
//
//    int16_t currentSenseOffsetHighRange;
//    float currentSenseScaleHighRange;
//
//    int16_t voltageSenseOffsetLowRange;
//    float voltageSenseScaleLowRange;
//
//    int16_t voltageSenseOffsetHighRange;
//    float voltageSenseScaleHighRange;
//
//    int16_t currentSetOffsetLowRange;
//    float currentSetScaleLowRange;
//
//    int16_t currentSetOffsetHighRange;
//    float currentSetScaleHighRange;
//}calibrationFlash;
//
//__attribute__ ((section(".config")))
//uint32_t calFlashValid;
#define FLASH_CALIBRATION_DATA          0x0801FC04
#define FLASH_VALID_CALIB_INDICATOR     0x0801FC00

/**
 * \brief Transfers the calibration values from the end of the FLASH
 *
 * (see linker script, section '.config') into the RAM
 * \return 0 on success, 1 on failure
 */
uint8_t cal_readFromFlash(void) {
    // check whether there is any calibration data in FLASH
    if (*(uint32_t*) FLASH_VALID_CALIB_INDICATOR == 0x01) {
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

uint32_t cal_sampleADC(uint8_t channel) {
//    screen_Clear();
//    screen_FastString12x16("Sampling..", 0, 0);
//    screen_Rectangle(12, 21, 115, 42);
//    screen_Rectangle(13, 22, 114, 41);
//    uint32_t sum = 0;
//    uint16_t cnt;
//    for (cnt = 0; cnt < 500; cnt++) {
//        sum += hal_getADC(channel, 1);
//        if (cnt % 5 == 0)
//            screen_VerticalLine(13 + cnt / 5, 23, 18);
//        timer_waitms(10);
//    }
//    return sum /= 500;
}

/**
 * \brief Sets the calibration values to the default values.
 *
 * Should be used in case of missing calibration data.
 */
void cal_setDefaultCalibration(void) {
    calibration.currentSenseOffsetLowRange = CAL_DEF_CURSENS_OFFSET_LOW;
    calibration.currentSenseScaleLowRange = CAL_DEF_CURSENS_SCALE_LOW;

    calibration.currentSenseOffsetHighRange = CAL_DEF_CURSENS_OFFSET_HIGH;
    calibration.currentSenseScaleHighRange = CAL_DEF_CURSENS_SCALE_HIGH;

    calibration.voltageSenseOffsetLowRange = CAL_DEF_VOLSENS_OFFSET_LOW;
    calibration.voltageSenseScaleLowRange = CAL_DEF_VOLSENS_SCALE_LOW;

    calibration.voltageSenseOffsetHighRange = CAL_DEF_VOLSENS_OFFSET_HIGH;
    calibration.voltageSenseScaleHighRange = CAL_DEF_VOLSENS_SCALE_HIGH;

    calibration.currentSetOffsetLowRange = CAL_DEF_CURSET_OFFSET_LOW;
    calibration.currentSetScaleLowRange = CAL_DEF_CURSET_SCALE_LOW;

    calibration.currentSetOffsetHighRange = CAL_DEF_CURSET_OFFSET_HIGH;
    calibration.currentSetScaleHighRange = CAL_DEF_CURSET_SCALE_HIGH;
}

void calibrationMenu(void) {
    char *entries[3];
    int8_t sel;
    do {
        char automatic[21] = "Automatic Cal.";
        entries[0] = automatic;

        char manual[21] = "Manual Cal.";
        entries[1] = manual;

        char info[21] = "Multimeter info";
        entries[2] = info;

        sel = menu_ItemChooseDialog("\xCD\xCD" "CALIBRATIONS MENU\xCD\xCD",
                entries, 3);
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
        }
    } while (sel >= 0);
}

void calibrationProcessAutomatic(void) {
    uint32_t button;
    uint8_t setupOK;
    while (hal_getButton())
        ;
    calibration.active = 1;

    // low range current calibration
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

//    // save calibration values in FLASH
//    cal_writeToFlash();

    while (hal_getButton())
        ;

    calibration.active = 0;
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
//    while (hal_getButton())
//        ;
//    calibration.active = 1;
//
//    uint16_t adc100mA;
//    uint16_t adc2A_lowRange;
//    uint16_t adc2A_highRange;
//    uint16_t adc10A;
//    uint16_t dac100mA;
//    uint16_t dac10A;
//
//    uint16_t adc1V;
//    uint16_t adc12V_lowRange;
//    uint16_t adc12V_highRange;
//    uint16_t adc30V;
//    uint32_t voltage1V;
//    uint32_t voltage12V;
//    uint32_t voltage30V;
//
//    uint32_t button = 0;
//
//    /*
//     * Step 2: calibrating low current range
//     */
//    hal_setGain(0);
//    hal_setCurrentGain(1);
//    hal_setDAC(0);
//
//    int32_t defaultValue100mA = 20;
//    int32_t defaultValue5A = 1000;
//    int32_t defaultValue2A = 400;
//    int32_t defaultValue10A = 2000;
//
//    screen_Clear();
//    screen_FastString12x16("Cal", 0, 0);
//    screen_FastChar12x16(33, 0, 'i');
//    screen_FastString12x16("brat", 43, 0);
//    screen_FastChar12x16(89, 0, 'i');
//    screen_FastString12x16("on", 99, 0);
//    screen_FastString6x8("Apply about 5V. Turn", 0, 2);
//    screen_FastString6x8("knob until load draws", 0, 3);
//    screen_FastString6x8("100mA", 0, 4);
//    screen_FastString6x8("ESC: Abort", 0, 6);
//    screen_FastString6x8("Enter: Continue", 0, 7);
//
//    do {
//        hal_setDAC(defaultValue100mA);
//        defaultValue100mA += hal_getEncoderMovement();
//        if (defaultValue100mA < 0)
//            defaultValue100mA = 0;
//        else if (defaultValue100mA > 4096)
//            defaultValue100mA = 4096;
//        button = hal_getButton();
//        timer_waitms(20);
//    } while (!(button & (HAL_BUTTON_ESC | HAL_BUTTON_ENTER)));
//    if (button & HAL_BUTTON_ESC) {
//        hal_setDAC(0);
//        calibration.active = 0;
//        return;
//    }
//
//    // save calibration values
//    dac100mA = defaultValue100mA;
//    adc100mA = cal_sampleADC(ADC_CURRENT_SENSE);
//    hal_setDAC(0);
//
//    while (hal_getButton())
//        ;
//
//    screen_Clear();
//    screen_FastString12x16("Cal", 0, 0);
//    screen_FastChar12x16(33, 0, 'i');
//    screen_FastString12x16("brat", 43, 0);
//    screen_FastChar12x16(89, 0, 'i');
//    screen_FastString12x16("on", 99, 0);
//    screen_FastString6x8("Apply about 5V. Turn", 0, 2);
//    screen_FastString6x8("knob until load draws", 0, 3);
//    screen_FastString6x8("2A", 0, 4);
//    screen_FastString6x8("ESC: Abort", 0, 6);
//    screen_FastString6x8("Enter: Continue", 0, 7);
//
//    do {
//        hal_setDAC(defaultValue2A);
//        defaultValue2A += hal_getEncoderMovement();
//        if (defaultValue2A < 0)
//            defaultValue2A = 0;
//        else if (defaultValue2A > 4096)
//            defaultValue2A = 4096;
//        button = hal_getButton();
//        timer_waitms(20);
//    } while (!(button & (HAL_BUTTON_ESC | HAL_BUTTON_ENTER)));
//    if (button & HAL_BUTTON_ESC) {
//        hal_setDAC(0);
//        calibration.active = 0;
//        return;
//    }
//
//    // save calibration values
//    adc2A_lowRange = cal_sampleADC(ADC_CURRENT_SENSE);
//    hal_setCurrentGain(0);
//    timer_waitms(20);
//    adc2A_highRange = cal_sampleADC(ADC_CURRENT_SENSE);
//    hal_setDAC(0);
//
//    while (hal_getButton())
//        ;
//
//    screen_Clear();
//    screen_FastString12x16("Cal", 0, 0);
//    screen_FastChar12x16(33, 0, 'i');
//    screen_FastString12x16("brat", 43, 0);
//    screen_FastChar12x16(89, 0, 'i');
//    screen_FastString12x16("on", 99, 0);
//    screen_FastString6x8("Apply about 5V. Turn", 0, 2);
//    screen_FastString6x8("knob until load draws", 0, 3);
//    screen_FastString6x8("10A", 0, 4);
//    screen_FastString6x8("ESC: Abort", 0, 6);
//    screen_FastString6x8("Enter: Continue", 0, 7);
//
//    do {
//        hal_setDAC(defaultValue10A);
//        defaultValue10A += hal_getEncoderMovement();
//        if (defaultValue10A < 0)
//            defaultValue10A = 0;
//        else if (defaultValue10A > 4096)
//            defaultValue10A = 4096;
//        button = hal_getButton();
//        timer_waitms(20);
//    } while (!(button & (HAL_BUTTON_ESC | HAL_BUTTON_ENTER)));
//    if (button & HAL_BUTTON_ESC) {
//        hal_setDAC(0);
//        calibration.active = 0;
//        return;
//    }
//
//    // save calibration values
//    dac10A = defaultValue10A;
//    adc10A = cal_sampleADC(ADC_CURRENT_SENSE);
//    hal_setDAC(0);
//
//    while (hal_getButton())
//        ;
//
//    /*
//     * Step 3: calibrating low voltage range
//     */
//    char input[] = "Actual voltage";
//    do {
//        hal_setVoltageGain(1);
//        screen_Clear();
//        screen_FastString12x16("Cal", 0, 0);
//        screen_FastChar12x16(33, 0, 'i');
//        screen_FastString12x16("brat", 43, 0);
//        screen_FastChar12x16(89, 0, 'i');
//        screen_FastString12x16("on", 99, 0);
//        screen_FastString6x8("Apply about 1V.", 0, 2);
//        screen_FastString6x8("ESC: Abort", 0, 6);
//        screen_FastString6x8("Enter: Continue", 0, 7);
//
//        do {
//            button = hal_getButton();
//        } while (!(button & (HAL_BUTTON_ESC | HAL_BUTTON_ENTER)));
//        if (button & HAL_BUTTON_ESC) {
//            calibration.active = 0;
//            return;
//        }
//
//    } while (menu_getInputValue(&voltage1V, input, 500, 1500, "mV", "V", NULL));
//
//    // save calibration values
//    adc1V = cal_sampleADC(ADC_VOLTAGE_SENSE);
//
//    while (hal_getButton())
//        ;
//
//    do {
//        screen_Clear();
//        screen_FastString12x16("Cal", 0, 0);
//        screen_FastChar12x16(33, 0, 'i');
//        screen_FastString12x16("brat", 43, 0);
//        screen_FastChar12x16(89, 0, 'i');
//        screen_FastString12x16("on", 99, 0);
//        screen_FastString6x8("Apply exactly 12V.", 0, 2);
//        screen_FastString6x8("ESC: Abort", 0, 6);
//        screen_FastString6x8("Enter: Continue", 0, 7);
//
//        do {
//            button = hal_getButton();
//        } while (!(button & (HAL_BUTTON_ESC | HAL_BUTTON_ENTER)));
//        if (button & HAL_BUTTON_ESC) {
//            calibration.active = 0;
//            return;
//        }
//
//    } while (menu_getInputValue(&voltage12V, input, 11500, 12500, "mV", "V",
//            NULL));
//
//    // save calibration values
//    adc12V_lowRange = cal_sampleADC(ADC_VOLTAGE_SENSE);
//    hal_setVoltageGain(0);
//    timer_waitms(20);
//    adc12V_highRange = cal_sampleADC(ADC_VOLTAGE_SENSE);
//
//    while (hal_getButton())
//        ;
//
//    /*
//     * Step 4: calibrating high voltage range
//     */
//    do {
//        hal_setVoltageGain(0);
//        screen_Clear();
//        screen_FastString12x16("Cal", 0, 0);
//        screen_FastChar12x16(33, 0, 'i');
//        screen_FastString12x16("brat", 43, 0);
//        screen_FastChar12x16(89, 0, 'i');
//        screen_FastString12x16("on", 99, 0);
//        screen_FastString6x8("Apply exactly 30V.", 0, 2);
//        screen_FastString6x8("ESC: Abort", 0, 6);
//        screen_FastString6x8("Enter: Continue", 0, 7);
//
//        do {
//            button = hal_getButton();
//        } while (!(button & (HAL_BUTTON_ESC | HAL_BUTTON_ENTER)));
//        if (button & HAL_BUTTON_ESC) {
//            calibration.active = 0;
//            return;
//        }
//    } while (menu_getInputValue(&voltage30V, input, 29000, 31000, "mV", "V", NULL));
//
//    // save calibration values
//    adc30V = cal_sampleADC(ADC_VOLTAGE_SENSE);
//
//    while (hal_getButton())
//        ;
//
//    /*
//     * Step 5: all done!
//     */
//
//    screen_Clear();
//    screen_FastString12x16("Cal", 0, 0);
//    screen_FastChar12x16(33, 0, 'i');
//    screen_FastString12x16("brat", 43, 0);
//    screen_FastChar12x16(89, 0, 'i');
//    screen_FastString12x16("on", 99, 0);
//    screen_FastString12x16("completed", 0, 2);
//    screen_FastString6x8("ESC: Discard", 0, 6);
//    screen_FastString6x8("Enter: Save", 0, 7);
//    do {
//        button = hal_getButton();
//    } while (!(button & (HAL_BUTTON_ESC | HAL_BUTTON_ENTER)));
//    screen_Clear();
//    if (button & HAL_BUTTON_ESC) {
//        calibration.active = 0;
//        return;
//    }
//
//    // calculate calibration values
//    calibration.currentSetScaleLowRange = (float) (dac10A - dac100mA)
//            / (10000 - 100);
//    calibration.currentSetOffsetLowRange = dac100mA
//            - 100 * calibration.currentSetScaleLowRange;
//
//    calibration.currentSenseScaleLowRange = (float) (2000 - 100)
//            / (adc2A_lowRange - adc100mA);
//    calibration.currentSenseOffsetLowRange = adc100mA
//            - 100 / calibration.currentSenseScaleLowRange;
//
//    calibration.currentSenseScaleHighRange = (float) (10000 - 2000)
//            / (adc10A - adc2A_highRange);
//    calibration.currentSenseOffsetHighRange = adc2A_highRange
//            - 2000 / calibration.currentSenseScaleHighRange;
//
//    calibration.voltageSenseScaleLowRange = (float) (voltage12V - voltage1V)
//            / (adc12V_lowRange - adc1V);
//    calibration.voltageSenseOffsetLowRange = adc1V
//            - 1000 / calibration.voltageSenseScaleLowRange;
//
//    calibration.voltageSenseScaleHighRange = (float) (voltage30V - voltage12V)
//            / (adc30V - adc12V_highRange);
//    calibration.voltageSenseOffsetHighRange = adc12V_highRange
//            - 12000 / calibration.voltageSenseScaleHighRange;
//
//    // save calibration values in FLASH
//    cal_writeToFlash();
//
//    while (hal_getButton())
//        ;
//
//    calibration.active = 0;
}

void calibrationDisplayMultimeterInfo(void) {
    // loop while ESC is not pressed
    do {
        screen_Clear();
        screen_FastString6x8("\xCD\xCD\xCDMULTIMETER INFO\xCD\xCD\xCD", 0, 0);
        if (timer_TimeoutElapsed(meter.timeout)) {
            // no data received for at least one second
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
void cal_setCurrent(uint32_t mA) {
//    if ((mA <= 20000 && hal.setRange == RANGE_LOW) || mA <= 15000) {
//        // use low range (0-20A)
//        hal_setGain(0);
//        int16_t dacValue = mA * calibration.currentSetScaleLowRange
//                + calibration.currentSetOffsetLowRange;
//        if (dacValue < 0)
//            dacValue = 0;
//        else if (dacValue > 0x0fff)
//            dacValue = 0x0fff;
//        hal_setDAC(dacValue);
//    } else {
//        // use high range (0-200A)
//        hal_setGain(1);
//        // TODO add calibration
//        hal_setDAC(mA / 50);
//    }
}

/**
 * \brief Returns the current being drawn
 *
 * \return Current in mA
 */
int32_t cal_getCurrent(void) {
//    uint16_t biased = hal_getADC(ADC_CURRENT_SENSE, 1);
//    int32_t ret = 0;
//    if (hal.currentRange == RANGE_LOW) {
//        int16_t unbiased = biased - calibration.currentSenseOffsetLowRange;
//        ret = unbiased * calibration.currentSenseScaleLowRange;
//    } else {
//        int16_t unbiased = biased - calibration.currentSenseOffsetHighRange;
//        ret = unbiased * calibration.currentSenseScaleHighRange;
//    }
//    if (biased >= 3500 && hal.currentRange == RANGE_LOW) {
//        // low range is near limit -> switch to high range
//        hal_setCurrentGain(0);
//    } else if (biased <= 300 && hal.currentRange == RANGE_HIGH) {
//        // high range is near limit -> switch to low range
//        hal_setCurrentGain(1);
//    }
//    if (ret < 0)
//        ret = 0;
//    return ret;
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
}

int32_t cal_getUncalibVoltage(void) {
//    uint16_t biased = hal_getADC(ADC_VOLTAGE_SENSE, 1);
//    int32_t ret = 0;
//    if (hal.voltageRange == RANGE_LOW) {
//        int16_t unbiased = biased - CAL_DEF_VOLSENS_OFFSET_LOW;
//        ret = unbiased * CAL_DEF_VOLSENS_SCALE_LOW;
//    } else {
//        int16_t unbiased = biased - CAL_DEF_VOLSENS_OFFSET_HIGH;
//        ret = unbiased * CAL_DEF_VOLSENS_SCALE_HIGH;
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
}
int32_t cal_getUncalibCurrent(void) {
//    uint16_t biased = hal_getADC(ADC_CURRENT_SENSE, 1);
//    int32_t ret = 0;
//    if (hal.currentRange == RANGE_LOW) {
//        int16_t unbiased = biased - CAL_DEF_CURSENS_OFFSET_LOW;
//        ret = unbiased * CAL_DEF_CURSENS_SCALE_LOW;
//    } else {
//        int16_t unbiased = biased - CAL_DEF_CURSENS_OFFSET_HIGH;
//        ret = unbiased * CAL_DEF_CURSENS_SCALE_HIGH;
//    }
//    if (biased >= 3500 && hal.currentRange == RANGE_LOW) {
//        // low range is near limit -> switch to high range
//        hal_setCurrentGain(0);
//    } else if (biased <= 300 && hal.currentRange == RANGE_HIGH) {
//        // high range is near limit -> switch to low range
//        hal_setCurrentGain(1);
//    }
//    if (ret < 0)
//        ret = 0;
//    return ret;
}

/**
 * \brief Returns the temperature at heatsink1
 *
 * \return Temperature in 0.1°C
 */
uint16_t cal_getTemp1(void) {
//    return LM35_ADC_TO_TEMP(hal_getADC(ADC_TEMPERATURE1, 1));
}

/**
 * \brief Returns the temperature at heatsink2
 *
 * \return Temperature in 0.1°C
 */
uint16_t cal_getTemp2(void) {
//    return LM35_ADC_TO_TEMP(hal_getADC(ADC_TEMPERATURE2, 1));
}
