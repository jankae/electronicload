#include "arbitrary.h"

const char arbParamNames[ARB_NUM_PARAMS][21] = { "CURRENT", "VOLTAGE",
        "RESISTANCE", "POWER" };

const char arbParamUnits0[ARB_NUM_PARAMS][6] = { "uA", "uV", "mOhm", "uW" };
const char arbParamUnits3[ARB_NUM_PARAMS][6] = { "mA", "mV", "Ohm", "mW" };
const char arbParamUnits6[ARB_NUM_PARAMS][6] = { "A", "V", "kOhm", "W" };

void arb_Init(void) {
    arbitrary.active = 0;
    arbitrary.numPoints = 4;
    arbitrary.param = &load.current;
    arbitrary.paramNum = 0;
    arbitrary.points[0].time = 1000;
    arbitrary.points[0].value = 0;
    arbitrary.points[1].time = 2000;
    arbitrary.points[1].value = 1000;
    arbitrary.points[2].time = 3000;
    arbitrary.points[2].value = 0;
    arbitrary.points[3].time = 4000;
    arbitrary.points[3].value = 1000;
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
            value = common_Map(time, 0, arbitrary.points[0].time,
                    arbitrary.points[arbitrary.numPoints - 1].value,
                    arbitrary.points[0].value);
            break;
        }
    } else {
        // time is passed last data point
        // shouldn't happen but just in case use its value
        value = arbitrary.points[arbitrary.numPoints - 1].value;
    }
    return value;
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
        int32_t minValue = INT32_MAX, maxValue = INT32_MIN, maxTime = INT32_MIN;
        uint8_t i;
        for (i = 0; i < arbitrary.numPoints; i++) {
            if (arbitrary.points[i].time > maxTime)
                maxTime = arbitrary.points[i].time;
            if (arbitrary.points[i].value > maxValue)
                maxValue = arbitrary.points[i].value;
            if (arbitrary.points[i].value < minValue)
                minValue = arbitrary.points[i].value;
        }
        // increase boundaries slightly -> min and max can't be the same
        int32_t rangeValue = maxValue - minValue;
        maxValue += rangeValue / 20 + 1;
        minValue -= rangeValue / 20 + 1;
        maxTime++;
#define ARB_VIEWWIN_HEIGHT      32
        screen_Clear();

        if (grabPoint == ARB_GRAB_NONE) {
            // calculate cursorY position
            // find points next to cursor
            cursorTime = common_Map(cursorX, 1, 126, 0, maxTime);
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
            cursorX = common_Map(cursorTime, 0, maxTime, 1, 126);
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
            uint8_t x = common_Map(arbitrary.points[i].time, 0, maxTime, 1,
                    126);
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
            uint32_t time = common_Map(i, 1, 126, 0, maxTime);
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
            int32_t inkrement = maxTime;
            if (inkrement < 256)
                inkrement = 256;
            arbitrary.points[selectedPoint].time += encoder * inkrement / 256;
            if (arbitrary.points[selectedPoint].time < 0)
                arbitrary.points[selectedPoint].time = 0;
            // check whether two points have changed places
            if (selectedPoint > 0) {
                if (arbitrary.points[selectedPoint].time
                        < arbitrary.points[selectedPoint - 1].time) {
                    int32_t timeBuf = arbitrary.points[selectedPoint].time;
                    int32_t valBuf = arbitrary.points[selectedPoint].value;
                    arbitrary.points[selectedPoint].time =
                            arbitrary.points[selectedPoint - 1].time;
                    arbitrary.points[selectedPoint].value =
                            arbitrary.points[selectedPoint - 1].value;
                    arbitrary.points[selectedPoint - 1].time = timeBuf;
                    arbitrary.points[selectedPoint - 1].value = valBuf;
                    selectedPoint--;
                }
            }
            if (selectedPoint < arbitrary.numPoints - 1) {
                if (arbitrary.points[selectedPoint].time
                        > arbitrary.points[selectedPoint + 1].time) {
                    int32_t timeBuf = arbitrary.points[selectedPoint].time;
                    int32_t valBuf = arbitrary.points[selectedPoint].value;
                    arbitrary.points[selectedPoint].time =
                            arbitrary.points[selectedPoint + 1].time;
                    arbitrary.points[selectedPoint].value =
                            arbitrary.points[selectedPoint + 1].value;
                    arbitrary.points[selectedPoint + 1].time = timeBuf;
                    arbitrary.points[selectedPoint + 1].value = valBuf;
                    selectedPoint++;
                }
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
                    arbitrary.points[i].value = arbitrary.points[i - 1].value;
                    arbitrary.points[i].time = arbitrary.points[i - 1].time;
                }
                // add point at current cursorposition
                arbitrary.points[nextPoint].value = cursorValue;
                arbitrary.points[nextPoint].time = cursorTime;
                arbitrary.numPoints++;
            }
            if ((button & HAL_BUTTON_SOFT0) && arbitrary.numPoints > 1) {
                // move points after selected point one down
                for (i = selectedPoint; i < arbitrary.numPoints; i++) {
                    arbitrary.points[i].value = arbitrary.points[i + 1].value;
                    arbitrary.points[i].time = arbitrary.points[i + 1].time;
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
}
