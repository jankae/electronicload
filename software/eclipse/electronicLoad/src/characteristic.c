#include "characteristic.h"

static container_t c;
static button_t bStart, bView, bTransmit;
static entry_t eStartI, eStopI, eAbortV, eDeltaT;
static label_t lStartI, lStopI, lAbortV, lDeltaT;
static int32_t minDelta = 1, maxDelta = 1000;

void characteristic_Init(void) {
    /* set default values */
    characteristic.currentStart = 0;
    characteristic.currentStop = settings.maxCurrent;
    characteristic.abortVoltage = 0;
    characteristic.deltaT = 10;

    /* create GUI elements */
    label_createWithText(&lStartI, "I start:", FONT_MEDIUM);
    label_createWithText(&lStopI, "I stop:", FONT_MEDIUM);
    label_createWithText(&lAbortV, "U abort:", FONT_MEDIUM);
    label_createWithText(&lDeltaT, "T delta:", FONT_MEDIUM);

    entry_create(&eStartI, &characteristic.currentStart, &settings.maxCurrent,
    NULL, FONT_MEDIUM, 4, UNIT_CURRENT, NULL);
    entry_create(&eStopI, &characteristic.currentStop, &settings.maxCurrent,
    NULL, FONT_MEDIUM, 4, UNIT_CURRENT, NULL);
    entry_create(&eAbortV, &characteristic.abortVoltage, &settings.maxVoltage,
    NULL, FONT_MEDIUM, 4, UNIT_VOLTAGE, NULL);
    entry_create(&eDeltaT, &characteristic.deltaT, &maxDelta, &minDelta,
            FONT_MEDIUM, 4, UNIT_TIME, NULL);

    button_create(&bStart, "Start", FONT_MEDIUM, 0, characteristic_Run);
    button_create(&bView, "View", FONT_MEDIUM, 0,
            characteristic_ViewResult);
    button_create(&bTransmit, "Transmit", FONT_MEDIUM, 0,
            characteristic_TransmitResult);

    container_create(&c, 128, 55);

    container_attach(&c, &lStartI, 0, 2);
    container_attach(&c, &lStopI, 0, 14);
    container_attach(&c, &lAbortV, 0, 26);
    container_attach(&c, &lDeltaT, 0, 38);

    container_attach(&c, &eStartI, 60, 0);
    container_attach(&c, &eStopI, 60, 12);
    container_attach(&c, &eAbortV, 60, 24);
    container_attach(&c, &eDeltaT, 60, 36);

    container_attach(&c, &bStart, 0, 48);
    container_attach(&c, &bView, 38, 48);
    container_attach(&c, &bTransmit, 70, 48);
}

widget_t* characteristic_getWidget(void) {
    return (widget_t*) &c;
}

void characteristic_Run(void) {
    // Set static display content during sampling
    screen_Clear();
    screen_FastString12x16("Sampling..", 0, 0);
    screen_Rectangle(2, 21, 125, 42);
    screen_Rectangle(3, 22, 124, 41);
    // disable waveform generation
    // (it mustn't mess with the characteristic tracing)
    waveform.switchedOn = 0;
    // reset characteristic state variables
    characteristic.timeCount = 0;
    characteristic.pointCount = 0;
    // set CC mode with current for the first datapoint
    load_set_CC(characteristic_DatapointToCurrent(0));
    // enable input
    load.powerOn = 1;
    characteristic.active = 1;
    // wait for measurements to finish
    // or until the user aborts the process
    uint32_t button;
    do {
        button = hal_getButton();
        uint8_t i;
        for (i = 0; i < characteristic.pointCount; i++) {
            screen_VerticalLine(3 + i, 23, 18);
        }
        timer_waitms(20);
    } while (!(button & HAL_BUTTON_ESC) && characteristic.active);
    // switch off load
    load.powerOn = 0;
    if (characteristic.active) {
        // user aborted measurement
        characteristic.active = 0;
        return;
    } else {
        characteristic.resultValid = 1;
        // display result
        characteristic_ViewResult();
    }
}

void characteristic_TransmitResult(void) {
    // transmit characteristic settings
    uart_writeString("U/I-characteristic result:\n");
    uart_writeString("Starting at ");
    char value[11];
    string_fromUint(characteristic.currentStart, value, 8, 6);
    uart_writeString(value);
    uart_writeString("A\nStopping at ");
    string_fromUint(characteristic.currentStop, value, 8, 6);
    uart_writeString(value);
    uart_writeString("A or ");
    string_fromUint(characteristic.abortVoltage, value, 9, 6);
    uart_writeString(value);
    uart_writeString("V\nTime between datapoints: ");
    string_fromUint(characteristic.deltaT, value, 6, 3);
    uart_writeString(value);
    uart_writeString("s\n\nCaptured ");
    string_fromUint(characteristic.pointCount, value, 3, 0);
    uart_writeString(value);
    uart_writeString(" datapoints\n\ntime[s];current[A];voltage[V]\n");

    // transmit datapoints as ascii-csv
    uint8_t i;
    for (i = 0; i < characteristic.pointCount; i++) {
        string_fromUint(i * characteristic.deltaT, value, 6, 3);
        uart_writeString(value);
        uart_writeByte(';');
        string_fromUint(characteristic_DatapointToCurrent(i), value, 8, 6);
        uart_writeString(value);
        uart_writeByte(';');
        string_fromUint(characteristic.voltageResponse[i], value, 9, 6);
        uart_writeString(value);
        uart_writeByte('\n');
    }
}

void characteristic_ViewResult(void) {
    uint32_t button;
    int32_t encoder;
    uint32_t minVoltage = UINT32_MAX, maxVoltage = 0;
    uint8_t i;
    for (i = 0; i < characteristic.pointCount; i++) {
        if (characteristic.voltageResponse[i] < minVoltage)
            minVoltage = characteristic.voltageResponse[i];
        if (characteristic.voltageResponse[i] > maxVoltage)
            maxVoltage = characteristic.voltageResponse[i];
    }
    maxVoltage += (maxVoltage - minVoltage) / 10;
    uint8_t cursorX = 0;
    // set encoder sensitivity high for cursor movement
    hal_setEncoderSensitivity(1);
    do {
        while (hal_getButton())
            ;
        screen_Clear();
        // display graph axis
        screen_VerticalLine(2, 0, 55);
        screen_HorizontalLine(2, 54, 126);
        // draw small arrows
        // y-axis
        screen_SetPixel(1, 1, PIXEL_ON);
        screen_SetPixel(0, 2, PIXEL_ON);
        screen_SetPixel(3, 1, PIXEL_ON);
        screen_SetPixel(4, 2, PIXEL_ON);
        // x-axis
        screen_SetPixel(125, 52, PIXEL_ON);
        screen_SetPixel(126, 53, PIXEL_ON);
        screen_SetPixel(126, 55, PIXEL_ON);
        screen_SetPixel(125, 56, PIXEL_ON);
        // display data points
        uint8_t lastx = 3;
        uint8_t lasty = common_Map(characteristic.voltageResponse[0],
                minVoltage, maxVoltage, 53, 0);
        for (i = 1; i < characteristic.pointCount; i++) {
            uint8_t x = i + 3;
            uint8_t y = common_Map(characteristic.voltageResponse[i],
                    minVoltage, maxVoltage, 53, 0);
            screen_Line(lastx, lasty, x, y);
            lastx = x;
            lasty = y;
        }
        // display cursor lines
        for (i = 0; i < 54; i += 2) {
            screen_SetPixel(cursorX + 3, i, PIXEL_ON);
        }
        uint8_t cursorY = common_Map(characteristic.voltageResponse[cursorX],
                minVoltage, maxVoltage, 53, 0);
        for (i = 0; i < 128; i += 2) {
            screen_SetPixel(i, cursorY, PIXEL_ON);
        }
        // display cursor values
        screen_FastString6x8("I:", 0, 7);
        screen_FastString6x8("U:", 64, 7);
        char value[11];
        string_fromUintUnit(characteristic_DatapointToCurrent(cursorX), value,
                4, 6, 'A');
        screen_FastString6x8(value, 12, 7);
        string_fromUintUnit(characteristic.voltageResponse[cursorX], value, 4,
                6, 'V');
        screen_FastString6x8(value, 76, 7);
        // wait for user input
        do {
            button = hal_getButton();
            encoder = hal_getEncoderMovement();
        } while (!button && !encoder);
        // move cursor
        int32_t newCursorX = cursorX;
        newCursorX += encoder;
        if (button & HAL_BUTTON_LEFT)
            newCursorX--;
        if (button & HAL_BUTTON_RIGHT)
            newCursorX++;
        // constrain cursor
        if (newCursorX < 0) {
            cursorX = 0;
        } else if (newCursorX >= characteristic.pointCount) {
            cursorX = characteristic.pointCount - 1;
        } else {
            cursorX = newCursorX;
        }
    } while (!(button & HAL_BUTTON_ESC));
    hal_setEncoderSensitivity(HAL_DEFAULT_ENCODER_SENSITIVITY);
    while (hal_getButton())
        ;
}

void characteristic_Update(void) {
    if (!characteristic.active)
        return;
    characteristic.timeCount++;
    if (characteristic.timeCount >= characteristic.deltaT) {
        characteristic.timeCount = 0;
        // reached a new data point
        if (characteristic.pointCount >= 119) {
            // finished U/I-characteristic
            characteristic.voltageResponse[119] = load.state.voltage;
            characteristic.active = 0;
        } else {
            // save measured voltage
            characteristic.voltageResponse[characteristic.pointCount] =
                    load.state.voltage;
            // check whether we have crossed the abort voltage
            if (characteristic.pointCount > 0) {
                uint32_t lastVol, curVol;
                lastVol =
                        characteristic.voltageResponse[characteristic.pointCount
                                - 1];
                curVol =
                        characteristic.voltageResponse[characteristic.pointCount];
                if ((lastVol <= characteristic.abortVoltage
                        && curVol >= characteristic.abortVoltage)
                        || (lastVol >= characteristic.abortVoltage
                                && curVol <= characteristic.abortVoltage)) {
                    // crossed the abor voltage -> stop measurement
                    characteristic.active = 0;
                }
            }
            // set the next current
            load.current = characteristic_DatapointToCurrent(
                    characteristic.pointCount);
        }
        characteristic.pointCount++;
    }
}

uint32_t characteristic_DatapointToCurrent(uint8_t point) {
    uint32_t current = characteristic.currentStart;
    current += ((int64_t) (characteristic.currentStop
            - characteristic.currentStart) * point) / 119;
    return current;
}
