#include "arbitrary.h"

static const char arbParamNames[ARB_NUM_PARAMS][11] = { "CURRENT", "VOLTAGE",
        "RESISTANCE", "POWER" };
static const char *paramItems[5] = { arbParamNames[0], arbParamNames[1],
        arbParamNames[2], arbParamNames[3], NULL };

static const char arbModeNames[2][12] = { "SINGLE SHOT", "CONTINUOUS" };
static const char *modeItems[3] = { arbModeNames[0], arbModeNames[1], NULL };

static const char arbStateNames[3][9] = { "DISABLED", "ARMED", "RUNNING" };
static const char *stateItems[4] = { arbStateNames[0], arbStateNames[1],
        arbStateNames[2], NULL };

static uint32_t *arbSetParamPointers[4] = { &load.current, &load.voltage,
        &load.resistance, &load.power };

const char arbParamUnits0[ARB_NUM_PARAMS][6] = { "uA", "uV", "mOhm", "uW" };
const char arbParamUnits3[ARB_NUM_PARAMS][6] = { "mA", "mV", "Ohm", "mW" };
const char arbParamUnits6[ARB_NUM_PARAMS][6] = { "A", "V", "kOhm", "W" };

static const uint32_t minLength = 2, maxLength = 30000;

container_t c;
label_t lState, lLength, lParam, lMode;
entry_t eLength;
dropdown_t dParam, dMode, dState;
button_t bEdit;

void arb_Init(void) {
    arbitrary.state = ARB_DISABLED;
    arbitrary.mode = ARB_CONTINUOUS;
    arbitrary.numPoints = 1;
    arbitrary.param = &load.current;
    arbitrary.paramNum = 0;
    arbitrary.sequenceLength = 1000;
    arbitrary.points[0].time = 0;
    arbitrary.points[0].value = 0;

    /* create GUI elements */
    label_createWithText(&lState, "State:", FONT_MEDIUM);
    label_createWithText(&lMode, "Mode:", FONT_MEDIUM);
    label_createWithText(&lParam, "Param:", FONT_MEDIUM);
    label_createWithText(&lLength, "Length:", FONT_MEDIUM);

    dropdown_create(&dMode, modeItems, &arbitrary.mode, FONT_MEDIUM, 0, NULL);
    dropdown_create(&dParam, paramItems, &arbitrary.paramNum, FONT_MEDIUM, 0,
            arb_ParamChanged);
    dropdown_create(&dState, stateItems, &arbitrary.state, FONT_MEDIUM, 0,
            arb_StatusChanged);

    entry_create(&eLength, &arbitrary.sequenceLength, &maxLength, &minLength,
            FONT_MEDIUM, 4, UNIT_TIME, arb_AdjustPointsToLength);

    button_create(&bEdit, "Edit sequence", FONT_MEDIUM, 0, arb_editSequence);

    container_create(&c, 128, 55);

    container_attach(&c, &lState, 0, 2);
    container_attach(&c, &lMode, 0, 14);
    container_attach(&c, &lParam, 0, 26);
    container_attach(&c, &lLength, 0, 38);

    container_attach(&c, &dState, 48, 0);
    container_attach(&c, &dMode, 30, 12);
    container_attach(&c, &dParam, 36, 24);

    container_attach(&c, &eLength, 60, 36);

    container_attach(&c, &bEdit, 0, 48);
}

widget_t* arb_getWidget(void) {
    return (widget_t*) &c;
}

void arb_ParamChanged(void) {
    arbitrary.param = arbSetParamPointers[arbitrary.paramNum];
    if (arbitrary.state != ARB_DISABLED) {
        // set load in correct mode
        // This works correctly because load.mode and paramNum
        // are using the same coding for the 4 different
        // modes/parameters
        load.mode = arbitrary.paramNum;
    }
}

void arb_StatusChanged(void) {
    if (arbitrary.state == ARB_DISABLED || arbitrary.state == ARB_ARMED) {
        load.powerOn = 0;
    }
}

int32_t arb_getValue(uint32_t time) {
    int32_t value;
    uint8_t i;
    for (i = 0; i < arbitrary.numPoints; i++) {
        if (time < arbitrary.points[i].time)
            break;
    }
// i is next point after time, i-1 is the previous one
// handle exceptions
    if (i > 0 && i < arbitrary.numPoints) {
        // time is between two points
        switch (arbitrary.points[i - 1].hold) {
        case 0:
            // ZOH
            value = arbitrary.points[i - 1].value;
            break;
        case 1:
            // FOH
            value = common_Map(time, arbitrary.points[i - 1].time,
                    arbitrary.points[i].time, arbitrary.points[i - 1].value,
                    arbitrary.points[i].value);
            break;
        }
    } else if (i == 0) {
        // time is in front of first point
        switch (arbitrary.points[arbitrary.numPoints - 1].hold) {
        case 0:
            // ZOH
            value = arbitrary.points[arbitrary.numPoints - 1].value;
            break;
        case 1:
            // FOH
            value = common_Map(time,
                    arbitrary.points[arbitrary.numPoints - 1].time
                            - arbitrary.sequenceLength,
                    arbitrary.points[0].time,
                    arbitrary.points[arbitrary.numPoints - 1].value,
                    arbitrary.points[0].value);
            break;
        }
    } else {
        // time is passed last data point
        switch (arbitrary.points[arbitrary.numPoints - 1].hold) {
        case 0:
            // ZOH
            value = arbitrary.points[arbitrary.numPoints - 1].value;
            break;
        case 1:
            // FOH
            value = common_Map(time,
                    arbitrary.points[arbitrary.numPoints - 1].time,
                    arbitrary.sequenceLength + arbitrary.points[0].time,
                    arbitrary.points[arbitrary.numPoints - 1].value,
                    arbitrary.points[0].value);
            break;
        }
    }
    return value;
}

void arb_Update(void) {
    if (arbitrary.state == ARB_ARMED && load.powerOn) {
        // load turned on, trigger arbitrary sequence
        arbitrary.time = 0;
        arbitrary.state = ARB_RUNNING;
    }
    if (arbitrary.state == ARB_RUNNING) {
        arbitrary.time++;
        if (arbitrary.time > arbitrary.sequenceLength) {
            arbitrary.time = 0;
            if (arbitrary.mode == ARB_SINGLE_SHOT) {
                // only one sequence at a time
                arbitrary.state = ARB_ARMED;
                load.powerOn = 0;
            }
        }
    }
    if (arbitrary.state == ARB_RUNNING && arbitrary.param) {
        *(arbitrary.param) = arb_getValue(arbitrary.time);
    }
}

void arb_AdjustPointsToLength(void) {
    uint8_t i;
    for (i = 0; i < arbitrary.numPoints; i++) {
        if (arbitrary.points[i].time > arbitrary.sequenceLength) {
            // this point (and all following it) are out of sequence length
            // -> delete
            arbitrary.numPoints = i;
        }
    }
    if (arbitrary.numPoints == 0) {
        // can't have zero points
        // create point at 0ms with value of 0
        arbitrary.numPoints = 1;
        arbitrary.points[0].time = 0;
        arbitrary.points[0].value = 0;
    }
}

void arb_editSequence(void) {
    uint32_t button;
    int32_t encoder;
    uint8_t cursorX = 1;
    uint8_t cursorY;
    int32_t cursorValue;
    uint32_t cursorTime;
    uint8_t editActive = 0;
    uint8_t selectedPoint = 0;
#define ARB_GRAB_NONE       0
#define ARB_GRAB_TIME       1
#define ARB_GRAB_AMPLITUDE  2
    uint8_t grabPoint = ARB_GRAB_NONE;
// set encoder sensitivity high for cursor movement
    hal_setEncoderSensitivity(1);
    do {
        while (hal_getButton())
            ;
        // display current sequence
        // find maximum and minimum
        int32_t minValue = INT32_MAX, maxValue = INT32_MIN;
        uint8_t i;
        for (i = 0; i < arbitrary.numPoints; i++) {
            if (arbitrary.points[i].value > maxValue)
                maxValue = arbitrary.points[i].value;
            if (arbitrary.points[i].value < minValue)
                minValue = arbitrary.points[i].value;
        }
        // increase boundaries slightly -> min and max can't be the same
        int32_t rangeValue = maxValue - minValue;
        maxValue += rangeValue / 20 + 1;
        minValue -= rangeValue / 20 + 1;
#define ARB_VIEWWIN_HEIGHT      32
        screen_Clear();

        if (grabPoint == ARB_GRAB_NONE) {
            // calculate cursorY position
            // find points next to cursor
            cursorTime = common_Map(cursorX, 1, 126, 0,
                    arbitrary.sequenceLength);
            uint32_t maxDist = UINT32_MAX;
            for (i = 0; i < arbitrary.numPoints; i++) {
                int32_t dist = arbitrary.points[i].time - cursorTime;
                if (dist < 0)
                    dist = -dist;
                if (dist < maxDist) {
                    maxDist = dist;
                    selectedPoint = i;
                }
            }
            cursorValue = arb_getValue(cursorTime);
            cursorY = common_Map(cursorValue, maxValue, minValue, 1,
            ARB_VIEWWIN_HEIGHT - 1);
        } else {
            // cursor locked to selected point
            cursorValue = arbitrary.points[selectedPoint].value;
            cursorTime = arbitrary.points[selectedPoint].time;
            cursorX = common_Map(cursorTime, 0, arbitrary.sequenceLength, 1,
                    126);
            cursorY = common_Map(cursorValue, maxValue, minValue, 1,
            ARB_VIEWWIN_HEIGHT - 1);
        }
        // display cursor
        for (i = 0; i < ARB_VIEWWIN_HEIGHT; i += 2) {
            screen_SetPixel(cursorX, i, PIXEL_ON);
        }
        for (i = 0; i < 128; i += 2) {
            screen_SetPixel(i, cursorY, PIXEL_ON);
        }

        // display points and connecting lines
        screen_Rectangle(0, 0, 127, ARB_VIEWWIN_HEIGHT);
        for (i = 0; i < arbitrary.numPoints; i++) {
            uint8_t x = common_Map(arbitrary.points[i].time, 0,
                    arbitrary.sequenceLength, 1, 126);
            uint8_t y = common_Map(arbitrary.points[i].value, maxValue,
                    minValue, 1,
                    ARB_VIEWWIN_HEIGHT - 1);
            // draw datapoint
            screen_Rectangle(x - 1, y - 1, x + 1, y + 1);
        }
        // draw values between points
        uint8_t y = common_Map(arb_getValue(0), maxValue, minValue, 1,
        ARB_VIEWWIN_HEIGHT - 1);
        for (i = 2; i <= 126; i++) {
            uint8_t _y = y;
            // calculate time
            uint32_t time = common_Map(i, 1, 126, 0, arbitrary.sequenceLength);
            y = common_Map(arb_getValue(time), maxValue, minValue, 1,
            ARB_VIEWWIN_HEIGHT - 1);
            screen_Line(i - 1, _y, i, y);
        }

        // display coordinates
        screen_FastString6x8("X:", 0, 5);
        char value[11];
        string_fromUintUnits(cursorValue, value, 4,
                arbParamUnits0[arbitrary.paramNum],
                arbParamUnits3[arbitrary.paramNum],
                arbParamUnits6[arbitrary.paramNum]);
        screen_FastString6x8(value, 12, 5);
        screen_FastString6x8("t:", 74, 5);
        string_fromUintUnit(cursorTime, value, 3, 3, 's');
        screen_FastString6x8(value, 86, 5);

        // display buttons
        if (!editActive) {
            if (arbitrary.numPoints > 1)
                screen_SetSoftButton("Delete", 0);
            if (arbitrary.numPoints < ARB_MAX_POINTS)
                screen_SetSoftButton("Add", 1);
            screen_SetSoftButton("Edit", 2);
        } else {
            if (grabPoint != ARB_GRAB_TIME)
                screen_SetSoftButton("Grab t", 0);
            if (grabPoint != ARB_GRAB_AMPLITUDE)
                screen_SetSoftButton("Grab A", 1);
            switch (arbitrary.points[selectedPoint].hold) {
            case 0:
                screen_SetSoftButton("ZOH", 2);
                break;
            case 1:
                screen_SetSoftButton("FOH", 2);
                break;
            }
        }

        do {
            button = hal_getButton();
            encoder = hal_getEncoderMovement();
        } while (!button && !encoder);

        // move cursor and handle direct value input TODO
        switch (grabPoint) {
        case ARB_GRAB_NONE:
            if (encoder < 0 && cursorX > 1) {
                cursorX--;
            } else if (encoder > 0 && cursorX < 126) {
                cursorX++;
            }
            break;
        case ARB_GRAB_TIME: {
            int32_t inkrement = arbitrary.sequenceLength;
            if (inkrement < 256)
                inkrement = 256;
            arbitrary.points[selectedPoint].time += encoder * inkrement / 256;
            if (arbitrary.points[selectedPoint].time < 0)
                arbitrary.points[selectedPoint].time = 0;
            if (button & HAL_BUTTON_ENTER) {
                // enter new value
                menu_getInputValue(&arbitrary.points[selectedPoint].time,
                        "New time:", 0, arbitrary.sequenceLength, "ms", "s",
                        NULL);
                while ((button = hal_getButton()))
                    ;
            }
            if (arbitrary.points[selectedPoint].time < 0)
                arbitrary.points[selectedPoint].time = 0;
            else if (arbitrary.points[selectedPoint].time
                    > arbitrary.sequenceLength)
                arbitrary.points[selectedPoint].time = arbitrary.sequenceLength;
            // check whether two points have changed places
            while (selectedPoint > 0
                    && arbitrary.points[selectedPoint].time
                            < arbitrary.points[selectedPoint - 1].time) {
                struct arbDataPoint buf = arbitrary.points[selectedPoint];
                arbitrary.points[selectedPoint] = arbitrary.points[selectedPoint
                        - 1];
                arbitrary.points[selectedPoint - 1] = buf;
                selectedPoint--;
            }
            while (selectedPoint < arbitrary.numPoints - 1
                    && arbitrary.points[selectedPoint].time
                            > arbitrary.points[selectedPoint + 1].time) {
                struct arbDataPoint buf = arbitrary.points[selectedPoint];
                arbitrary.points[selectedPoint] = arbitrary.points[selectedPoint
                        + 1];
                arbitrary.points[selectedPoint + 1] = buf;
                selectedPoint++;
            }
        }
            break;
        case ARB_GRAB_AMPLITUDE: {
            int32_t inkrement = maxValue - minValue;
            if (inkrement < 64)
                inkrement = 64;
            arbitrary.points[selectedPoint].value += encoder * inkrement / 64;
            if (arbitrary.points[selectedPoint].value < 0)
                arbitrary.points[selectedPoint].value = 0;
            if (button & HAL_BUTTON_ENTER) {
                // enter new value
                menu_getInputValue(&arbitrary.points[selectedPoint].value,
                        "New value:", 0, 200000000,
                        arbParamUnits0[arbitrary.paramNum],
                        arbParamUnits3[arbitrary.paramNum],
                        arbParamUnits6[arbitrary.paramNum]);
                while ((button = hal_getButton()))
                    ;
            }
        }
            break;
        }

        if (!editActive) {
            // handle button input
            if ((button & HAL_BUTTON_SOFT1)
                    && arbitrary.numPoints < ARB_MAX_POINTS) {
                // calculate point after cursor
                uint8_t nextPoint;
                for (nextPoint = 0; nextPoint < arbitrary.numPoints;
                        nextPoint++) {
                    if (cursorTime < arbitrary.points[nextPoint].time)
                        break;
                }
                // move points after cursor one up
                for (i = arbitrary.numPoints; i > nextPoint; i--) {
                    arbitrary.points[i] = arbitrary.points[i - 1];
                }
                // add point at current cursorposition
                arbitrary.points[nextPoint].value = cursorValue;
                arbitrary.points[nextPoint].time = cursorTime;
                arbitrary.points[nextPoint].hold = 0;
                arbitrary.numPoints++;
            }
            if ((button & HAL_BUTTON_SOFT0) && arbitrary.numPoints > 1) {
                // move points after selected point one down
                for (i = selectedPoint; i < arbitrary.numPoints; i++) {
                    arbitrary.points[i] = arbitrary.points[i + 1];
                }
                arbitrary.numPoints--;
            }
            if (button & HAL_BUTTON_SOFT2) {
                editActive = 1;
            }
        } else {
            // currently in edit mode
            if (button & HAL_BUTTON_SOFT0) {
                grabPoint = ARB_GRAB_TIME;
            }
            if (button & HAL_BUTTON_SOFT1) {
                grabPoint = ARB_GRAB_AMPLITUDE;
            }
            if (button & HAL_BUTTON_SOFT2) {
                // change order hold
                arbitrary.points[selectedPoint].hold++;
                if (arbitrary.points[selectedPoint].hold >= 2) {
                    arbitrary.points[selectedPoint].hold = 0;
                }
            }
            if (button & HAL_BUTTON_ESC) {
                if (grabPoint == ARB_GRAB_NONE) {
                    // leave edit mode
                    editActive = 0;
                } else {
                    grabPoint = ARB_GRAB_NONE;
                }
                // delete ESC press, don't leave menu completely
                button &= ~HAL_BUTTON_ESC;
            }
        }

    } while (!(button & HAL_BUTTON_ESC));
    hal_setEncoderSensitivity(HAL_DEFAULT_ENCODER_SENSITIVITY);
    while (hal_getButton())
        ;
}
