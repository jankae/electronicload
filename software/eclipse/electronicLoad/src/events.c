#include "events.h"

const char eventSrcNames[EV_NUM_SOURCETYPES][21] = { "DISABLED", "TRIGGER RISE",
        "TRIGGER FALL", "PAR LOWER THAN", "PAR HIGHER THAN", "TIMER ZERO",
        "WAVEFORM PHASE" };

const char eventDestNames[EV_NUM_DESTTYPES][21] = { "TRIGGER HIGH",
        "TRIGGER LOW", "SET PARAMETER", "SET TIMER", "SET LOAD MODE",
        "INPUT ON", "INPUT OFF" };

const char eventSetParamNames[EV_NUM_SETPARAMS][21] = { "CURRENT", "VOLTAGE",
        "RESISTANCE", "POWER" };

const char eventSetParamUnits0[EV_NUM_SETPARAMS][6] =
        { "uA", "uV", "mOhm", "uW" };

const char eventSetParamUnits3[EV_NUM_SETPARAMS][6] =
        { "mA", "mV", "Ohm", "mW" };

const char eventSetParamUnits6[EV_NUM_SETPARAMS][6] = { "A", "V", "kOhm", "W" };

uint32_t *eventSetParamPointers[EV_NUM_SETPARAMS] = { &load.current,
        &load.voltage, &load.resistance, &load.power };

const char eventCompParamNames[EV_NUM_COMPPARAMS][21] = { "CURRENT", "VOLTAGE",
        "POWER", "SET CURRENT", "SET VOLTAGE", "SET RESIST.", "SET POWER" };

const char eventCompParamUnits0[EV_NUM_COMPPARAMS][6] = { "uA", "uV", "uW",
        "uA", "uV", "mOhm", "uW" };

const char eventCompParamUnits3[EV_NUM_COMPPARAMS][6] = { "mA", "mV", "mW",
        "mA", "mV", "Ohm", "mW" };

const char eventCompParamUnits6[EV_NUM_COMPPARAMS][6] = { "A", "V", "W", "A",
        "V", "kOhm", "W" };

uint32_t *eventCompParamPointers[EV_NUM_COMPPARAMS] = { &load.state.current,
        &load.state.voltage, &load.state.power, &load.current, &load.voltage,
        &load.resistance, &load.power };

void events_Init(void) {
    uint8_t i;
    for (i = 0; i < EV_MAXEVENTS; i++) {
        events.evlist[i].srcType = EV_SRC_DISABLED;
        events.evlist[i].srcParam = eventCompParamPointers[0];
        events.evlist[i].srcParamNum = 0;
        events.evlist[i].srcLimit = 0;
        events.evlist[i].srcTimerNum = 0;
        events.evlist[i].destMode = FUNCTION_CC;
        events.evlist[i].destParam = eventSetParamPointers[0];
        events.evlist[i].destParamNum = 0;
        events.evlist[i].destSetValue = 0;
        events.evlist[i].destTimerNum = 0;
        events.evlist[i].destTimerValue = 0;
        events.evlist[i].destType = EV_DEST_LOAD_OFF;
    }
}

void events_HandleEvents(void) {
    uint8_t i;
    uint8_t triggered[EV_MAXEVENTS];
    for (i = 0; i < EV_MAXEVENTS; i++) {
        if (events_isEventSourceTriggered(i)) {
            triggered[i] = 1;
        } else {
            triggered[i] = 0;
        }
    }
    for (i = 0; i < EV_MAXEVENTS; i++) {
        if (triggered[i]) {
            events_triggerEventDestination(i);
        }
    }
}

uint8_t events_isEventSourceTriggered(uint8_t ev) {
    uint8_t triggered = 0;
    if (events.evlist[ev].srcType == EV_SRC_TIM_ZERO) {
        if (events.evTimers[events.evlist[ev].srcTimerNum] == 0)
            triggered = 1;
    }
    if (events.evlist[ev].srcType == EV_SRC_PARAM_HIGHER) {
        if (*(events.evlist[ev].srcParam) > events.evlist[ev].srcLimit)
            triggered = 1;
    }
    if (events.evlist[ev].srcType == EV_SRC_PARAM_LOWER) {
        if (*(events.evlist[ev].srcParam) < events.evlist[ev].srcLimit)
            triggered = 1;
    }
    if (events.evlist[ev].srcType == EV_SRC_TRIG_FALL) {
        if (events.triggerInState == -1)
            triggered = 1;
    }
    if (events.evlist[ev].srcType == EV_SRC_TRIG_RISE) {
        if (events.triggerInState == 1)
            triggered = 1;
    }
    if (events.evlist[ev].srcType == EV_SRC_WAVEFORM_PHASE
            && waveform.form != WAVE_NONE) {
        // check whether phase limit has been passed since last cycle
        if (events.waveformPhase >= events.waveformOldPhase) {
            // phase hasn't passed 360°
            if (events.evlist[ev].srcLimit <= events.waveformPhase
                    && events.evlist[ev].srcLimit > events.waveformOldPhase) {
                triggered = 1;
            }
        } else {
            // phase has passed 360°
            if (events.evlist[ev].srcLimit > events.waveformOldPhase
                    || events.evlist[ev].srcLimit <= events.waveformPhase)
                triggered = 1;
        }
    }
    return triggered;
}

void events_triggerEventDestination(uint8_t ev) {
    if (events.evlist[ev].destType == EV_DEST_SET_PARAM) {
        *(events.evlist[ev].destParam) = events.evlist[ev].destSetValue;
    }
    if (events.evlist[ev].destType == EV_DEST_SET_TIMER) {
        events.evTimers[events.evlist[ev].destTimerNum] =
                events.evlist[ev].destTimerValue;
    }
    if (events.evlist[ev].destType == EV_DEST_TRIG_HIGH) {
        hal_setTriggerOut(1);
    }
    if (events.evlist[ev].destType == EV_DEST_TRIG_LOW) {
        hal_setTriggerOut(0);
    }
    if (events.evlist[ev].destType == EV_DEST_LOAD_MODE) {
        load_setMode(events.evlist[ev].destMode);
    }
    if (events.evlist[ev].destType == EV_DEST_LOAD_ON) {
        load.powerOn = 1;
    }
    if (events.evlist[ev].destType == EV_DEST_LOAD_OFF) {
        load.powerOn = 0;
    }
}

void events_decrementTimers(void) {
    uint8_t i;
    for (i = 0; i < EV_MAXTIMERS; i++) {
        if (events.evTimers[i] == 0) {
            // timer elapsed -> stop timer
            events.evTimers[i] = EV_TIMER_STOPPED;
        } else if (events.evTimers[i] != EV_TIMER_STOPPED) {
            // timer is running -> decrement
            events.evTimers[i]--;
        }
    }
}

void events_updateWaveformPhase(void) {
    events.waveformOldPhase = events.waveformPhase;
    events.waveformPhase = ((uint64_t) (waveform.phase) * 360000UL) / 65536;
}

void events_getDescr(uint8_t ev, char* descr) {
    if (events.evlist[ev].srcType == EV_SRC_DISABLED) {
        descr[0] = ev + '0';
        descr[1] = ':';
        string_copy(&descr[2], "DISABLED");
        return;
    }
// construct source description
    switch (events.evlist[ev].srcType) {
    case EV_SRC_PARAM_HIGHER:
        string_copy(descr, "P>:      ");
        string_copyn(&descr[3],
                eventCompParamNames[events.evlist[ev].srcParamNum], 6);
        break;
    case EV_SRC_PARAM_LOWER:
        string_copy(descr, "P<:      ");
        string_copyn(&descr[3],
                eventCompParamNames[events.evlist[ev].srcParamNum], 6);
        break;
    case EV_SRC_TIM_ZERO:
        string_copy(descr, "Timer  =0");
        descr[5] = (events.evlist[ev].srcTimerNum / 10) + '0';
        descr[6] = (events.evlist[ev].srcTimerNum % 10) + '0';
        break;
    case EV_SRC_TRIG_FALL:
        string_copy(descr, "TrigFall\x19");
        break;
    case EV_SRC_TRIG_RISE:
        string_copy(descr, "TrigRise\x18");
        break;
    case EV_SRC_WAVEFORM_PHASE:
        string_copy(descr, "WavePhase");
        break;
    }

    descr[9] = 0x1A; // right arrow

// construct destination description
    switch (events.evlist[ev].destType) {
    case EV_DEST_LOAD_MODE:
        string_copy(&descr[10], "Mode C ");
        switch (events.evlist[ev].destMode) {
        case FUNCTION_CC:
            descr[16] = 'C';
            break;
        case FUNCTION_CV:
            descr[16] = 'V';
            break;
        case FUNCTION_CR:
            descr[16] = 'R';
            break;
        case FUNCTION_CP:
            descr[16] = 'P';
            break;
        }
        break;
    case EV_DEST_LOAD_OFF:
        string_copy(&descr[10], "InputOff");
        break;
    case EV_DEST_LOAD_ON:
        string_copy(&descr[10], "InputOn");
        break;
    case EV_DEST_SET_PARAM:
        string_copy(&descr[10], "P=:       ");
        string_copyn(&descr[13],
                eventCompParamNames[events.evlist[ev].destParamNum], 7);
        break;
    case EV_DEST_SET_TIMER:
        string_copy(&descr[10], "SetTim   ");
        descr[17] = (events.evlist[ev].destTimerNum / 10) + '0';
        descr[18] = (events.evlist[ev].destTimerNum % 10) + '0';
        break;
    case EV_DEST_TRIG_HIGH:
        string_copy(&descr[10], "TrigHigh\x18");
        break;
    case EV_DEST_TRIG_LOW:
        string_copy(&descr[10], "TrigLow\x19");
        break;
    }
}

void events_menu(void) {
    char eventDescr[EV_MAXEVENTS][21];
    char *descrList[EV_MAXEVENTS];
    uint8_t i;
    int8_t ev = 0;
    do {
        for (i = 0; i < EV_MAXEVENTS; i++) {
            events_getDescr(i, eventDescr[i]);
            descrList[i] = eventDescr[i];
        }
        ev = menu_ItemChooseDialog(
                "\xCD\xCD\xCD\xCD\xCD" "EVENT LIST\xCD\xCD\xCD\xCD\xCD\xCD",
                descrList,
                EV_MAXEVENTS, ev);
        if (ev >= 0) {
            events_editEventMenu(ev);
        }
    } while (ev >= 0);
}

void events_editEventMenu(uint8_t ev) {

    uint8_t selectedRow = 1;
    uint32_t button;
    int32_t encoder;
    do {
        while (hal_getButton())
            ;

        evSourceType_t src = events.evlist[ev].srcType;
        evDestType_t dest = events.evlist[ev].destType;
        // create menu display
        screen_Clear();
        screen_FastString6x8(
                "\xCD\xCD\xCD\xCD\xCD" "EDIT EVENT\xCD\xCD\xCD\xCD\xCD\xCD", 0,
                0);
        // source display
        screen_FastString6x8("Src:", 6, 1);
        screen_FastString6x8(eventSrcNames[src], 30, 1);
        if (src == EV_SRC_TIM_ZERO) {
            screen_FastString6x8("Timer:", 6, 2);
            screen_FastChar6x8(events.evlist[ev].srcTimerNum + '0', 48, 2);
        } else if (src == EV_SRC_PARAM_HIGHER || src == EV_SRC_PARAM_LOWER) {
            screen_FastString6x8("Param:", 6, 2);
            screen_FastString6x8(
                    eventCompParamNames[events.evlist[ev].srcParamNum], 48, 2);
            screen_FastString6x8("Value:", 6, 3);
            char value[11];
            string_fromUint(events.evlist[ev].srcLimit, value, 9, 3);
            screen_FastString6x8(value, 48, 3);
        } else if (src == EV_SRC_WAVEFORM_PHASE) {
            screen_FastString6x8("Phase:", 6, 2);
            char value[11];
            string_fromUint(events.evlist[ev].srcLimit, value, 9, 3);
            screen_FastString6x8(value, 48, 2);
        }
        if (src != EV_SRC_DISABLED) {
            // source is enabled -> we must have a destination
            // destination display
            screen_FastString6x8("Dest:", 6, 5);
            screen_FastString6x8(eventDestNames[dest], 36, 5);
            if (dest == EV_DEST_SET_TIMER) {
                screen_FastString6x8("Timer:", 6, 6);
                screen_FastChar6x8(events.evlist[ev].destTimerNum + '0', 48, 6);
                screen_FastString6x8("Value:", 6, 7);
                char value[11];
                string_fromUint(events.evlist[ev].destTimerValue, value, 9, 3);
                screen_FastString6x8(value, 48, 7);
            } else if (dest == EV_DEST_SET_PARAM) {
                screen_FastString6x8("Param:", 6, 6);
                screen_FastString6x8(
                        eventSetParamNames[events.evlist[ev].destParamNum], 48,
                        6);
                screen_FastString6x8("Value:", 6, 7);
                char value[11];
                string_fromUint(events.evlist[ev].destSetValue, value, 9, 3);
                screen_FastString6x8(value, 48, 7);
            } else if (dest == EV_DEST_LOAD_MODE) {
                screen_FastString6x8("Mode: C", 6, 6);
                switch (events.evlist[ev].destMode) {
                case FUNCTION_CC:
                    screen_FastChar6x8('C', 48, 6);
                    break;
                case FUNCTION_CV:
                    screen_FastChar6x8('V', 48, 6);
                    break;
                case FUNCTION_CR:
                    screen_FastChar6x8('R', 48, 6);
                    break;
                case FUNCTION_CP:
                    screen_FastChar6x8('P', 48, 6);
                    break;
                }
            }
        }
        screen_FastChar6x8(0x1A, 0, selectedRow);
        // wait for user input
        do {
            button = hal_getButton();
            encoder = hal_getEncoderMovement();
        } while (!button && !encoder);

        if ((button & HAL_BUTTON_DOWN) || encoder > 0) {
            // move one entry down (if possible)
            switch (selectedRow) {
            case 1:
                if (src == EV_SRC_PARAM_HIGHER || src == EV_SRC_PARAM_LOWER
                        || src == EV_SRC_TIM_ZERO
                        || src == EV_SRC_WAVEFORM_PHASE)
                    // selected source has settings -> move to first setting
                    selectedRow = 2;
                else if (src != EV_SRC_DISABLED)
                    // selected source has no settings -> move directly to destination
                    selectedRow = 5;
                break;
            case 2:
                if (src == EV_SRC_PARAM_HIGHER || src == EV_SRC_PARAM_LOWER)
                    // selected source has setting value
                    selectedRow = 3;
                else
                    // no additional setting value -> directly to destination
                    selectedRow = 5;
                break;
            case 3:
                selectedRow = 5;
                break;
            case 5:
                if (dest == EV_DEST_SET_PARAM || dest == EV_DEST_SET_TIMER
                        || dest == EV_DEST_LOAD_MODE)
                    // selected destination has settings -> move to first setting
                    selectedRow = 6;
                break;
            case 6:
                if (dest == EV_DEST_SET_PARAM || dest == EV_DEST_SET_TIMER)
                    // move to second setting
                    selectedRow = 7;
                break;
            }
        }

        if ((button & HAL_BUTTON_UP) || encoder < 0) {
            // move one entry up
            if (selectedRow != 1 && selectedRow != 5) {
                // not at the top of source or destination -> can move one row up
                selectedRow--;
            } else if (selectedRow == 5) {
                // at top of destination -> can move up but next position depends
                // on source setting
                if (src == EV_SRC_PARAM_LOWER || src == EV_SRC_PARAM_HIGHER)
                    selectedRow = 3;
                else if (src == EV_SRC_TIM_ZERO || EV_SRC_WAVEFORM_PHASE)
                    selectedRow = 2;
                else
                    selectedRow = 1;
            }
        }

        if ((button & HAL_BUTTON_ENTER) || (button & HAL_BUTTON_ENCODER)) {
            // change selected setting
            if (selectedRow == 1) {
                // change source settings
                char descr[EV_NUM_SOURCETYPES][21];
                char *itemList[EV_NUM_SOURCETYPES];
                uint8_t i;
                for (i = 0; i < EV_NUM_SOURCETYPES; i++) {
                    string_copy(descr[i], eventSrcNames[i]);
                    itemList[i] = descr[i];
                }
                int8_t sel = menu_ItemChooseDialog("Select event source:",
                        itemList,
                        EV_NUM_SOURCETYPES, events.evlist[ev].srcType);
                if (sel >= 0) {
                    events.evlist[ev].srcType = sel;
                }
            } else if (selectedRow == 2
                    && (src == EV_SRC_PARAM_HIGHER || src == EV_SRC_PARAM_LOWER)) {
                // change compare parameter
                char descr[EV_NUM_COMPPARAMS][21];
                char *itemList[EV_NUM_COMPPARAMS];
                uint8_t i;
                for (i = 0; i < EV_NUM_COMPPARAMS; i++) {
                    string_copy(descr[i], eventCompParamNames[i]);
                    itemList[i] = descr[i];
                }
                int8_t sel = menu_ItemChooseDialog("Select parameter:",
                        itemList,
                        EV_NUM_COMPPARAMS, events.evlist[ev].srcParamNum);
                if (sel >= 0) {
                    events.evlist[ev].srcParamNum = sel;
                    events.evlist[ev].srcParam = eventCompParamPointers[sel];
                }
            } else if (selectedRow == 3
                    && (src == EV_SRC_PARAM_HIGHER || src == EV_SRC_PARAM_LOWER)) {
                // change compare value
                uint32_t val;
                if (menu_getInputValue(&val, "param limit", 0, 1000000,
                        eventCompParamUnits0[events.evlist[ev].srcParamNum],
                        eventCompParamUnits3[events.evlist[ev].srcParamNum],
                        eventCompParamUnits6[events.evlist[ev].srcParamNum])) {
                    events.evlist[ev].srcLimit = val;
                }
            } else if (selectedRow == 2 && src == EV_SRC_TIM_ZERO) {
                // select timer
                uint32_t tim;
                if (menu_getInputValue(&tim, "timer number", 0,
                EV_MAXTIMERS - 1, "Timer", NULL, NULL)) {
                    events.evlist[ev].srcTimerNum = tim;
                }
            } else if (selectedRow == 2 && src == EV_SRC_WAVEFORM_PHASE) {
                // change phase value
                uint32_t val;
                if (menu_getInputValue(&val, "Phase:", 0, 360000, "mDeg",
                        "Degree", NULL)) {
                    events.evlist[ev].srcLimit = val;
                }
            } else if (selectedRow == 5) {
                // change destination settings
                char descr[EV_NUM_DESTTYPES][21];
                char *itemList[EV_NUM_DESTTYPES];
                uint8_t i;
                for (i = 0; i < EV_NUM_DESTTYPES; i++) {
                    string_copy(descr[i], eventDestNames[i]);
                    itemList[i] = descr[i];
                }
                int8_t sel = menu_ItemChooseDialog("Select event dest.:",
                        itemList,
                        EV_NUM_DESTTYPES, events.evlist[ev].destType);
                if (sel >= 0) {
                    events.evlist[ev].destType = sel;
                }
            } else if (selectedRow == 6 && dest == EV_DEST_SET_PARAM) {
                // change set parameter
                char descr[EV_NUM_SETPARAMS][21];
                char *itemList[EV_NUM_SETPARAMS];
                uint8_t i;
                for (i = 0; i < EV_NUM_SETPARAMS; i++) {
                    string_copy(descr[i], eventSetParamNames[i]);
                    itemList[i] = descr[i];
                }
                int8_t sel = menu_ItemChooseDialog("Select parameter:",
                        itemList,
                        EV_NUM_SETPARAMS, events.evlist[ev].destParamNum);
                if (sel >= 0) {
                    events.evlist[ev].destParamNum = sel;
                    events.evlist[ev].destParam = eventSetParamPointers[sel];
                }
            } else if (selectedRow == 7 && dest == EV_DEST_SET_PARAM) {
                // change compare value
                uint32_t val;
                if (menu_getInputValue(&val, "param value", 0, 1000000,
                        eventSetParamUnits0[events.evlist[ev].destParamNum],
                        eventSetParamUnits3[events.evlist[ev].destParamNum],
                        eventSetParamUnits6[events.evlist[ev].destParamNum])) {
                    events.evlist[ev].destSetValue = val;
                }
            } else if (selectedRow == 6 && dest == EV_DEST_SET_TIMER) {
                // select timer
                uint32_t tim;
                if (menu_getInputValue(&tim, "timer number", 0,
                EV_MAXTIMERS - 1, "Timer", NULL, NULL)) {
                    events.evlist[ev].destTimerNum = tim;
                }
            } else if (selectedRow == 7 && dest == EV_DEST_SET_TIMER) {
                // change timer start value
                uint32_t time;
                if (menu_getInputValue(&time, "time", 0, 3600000, "ms", "s",
                NULL)) {
                    events.evlist[ev].destTimerValue = time;
                }
            } else if (selectedRow == 6 && dest == EV_DEST_LOAD_MODE) {
                // change load mode
                char *modeList[4] = { "CC", "CV", "CR", "CP" };
                int8_t sel = menu_ItemChooseDialog("Select load mode:",
                        modeList, 4, events.evlist[ev].destMode);
                if (sel >= 0) {
                    events.evlist[ev].destMode = sel;
                }
            }
        }

    } while (!(button & HAL_BUTTON_ESC));
}
