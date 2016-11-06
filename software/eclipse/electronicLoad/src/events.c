#include "events.h"

const char eventSrcNames[EV_NUM_SOURCETYPES][21] = { "DISABLED", "TRIGGER RISE",
        "TRIGGER FALL", "PAR LOWER THAN", "PAR HIGHER THAN", "TIMER ZERO",
        "WAVEFORM PHASE" };

const char eventDestNames[EV_NUM_DESTTYPES][21] = { "NONE", "TRIGGER HIGH",
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
        uint8_t j;
        for (j = 0; j < EV_MAXEFFECTS; j++) {
            events.evlist[i].effects[j].destMode = FUNCTION_CC;
            events.evlist[i].effects[j].destParam = eventSetParamPointers[0];
            events.evlist[i].effects[j].destParamNum = 0;
            events.evlist[i].effects[j].destSetValue = 0;
            events.evlist[i].effects[j].destTimerNum = 0;
            events.evlist[i].effects[j].destTimerValue = 0;
            events.evlist[i].effects[j].destType = EV_DEST_NOTHING;
        }
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
    switch (events.evlist[ev].srcType) {
    case EV_SRC_TIM_ZERO:
        if (events.evTimers[events.evlist[ev].srcTimerNum] == 0)
            triggered = 1;
        break;
    case EV_SRC_PARAM_HIGHER:
        if (*(events.evlist[ev].srcParam) > events.evlist[ev].srcLimit)
            triggered = 1;
        break;
    case EV_SRC_PARAM_LOWER:
        if (*(events.evlist[ev].srcParam) < events.evlist[ev].srcLimit)
            triggered = 1;
        break;
    case EV_SRC_TRIG_FALL:
        if (events.triggerInState == -1)
            triggered = 1;
        break;
    case EV_SRC_TRIG_RISE:
        if (events.triggerInState == 1)
            triggered = 1;
        break;
    case EV_SRC_WAVEFORM_PHASE:
        if (waveform.form != WAVE_NONE) {
            // check whether phase limit has been passed since last cycle
            if (events.waveformPhase >= events.waveformOldPhase) {
                // phase hasn't passed 360°
                if (events.evlist[ev].srcLimit <= events.waveformPhase
                        && events.evlist[ev].srcLimit
                                > events.waveformOldPhase) {
                    triggered = 1;
                }
            } else {
                // phase has passed 360°
                if (events.evlist[ev].srcLimit > events.waveformOldPhase
                        || events.evlist[ev].srcLimit <= events.waveformPhase)
                    triggered = 1;
            }
        }
        break;
    }
    return triggered;
}

void events_triggerEventDestination(uint8_t ev) {
    uint8_t i;
    for (i = 0; i < EV_MAXEFFECTS; i++) {
        switch (events.evlist[ev].effects[i].destType) {
        case EV_DEST_SET_PARAM:
            *(events.evlist[ev].effects[i].destParam) =
                    events.evlist[ev].effects[i].destSetValue;
            break;
        case EV_DEST_SET_TIMER:
            events.evTimers[events.evlist[ev].effects[i].destTimerNum] =
                    events.evlist[ev].effects[i].destTimerValue;
            break;
        case EV_DEST_TRIG_HIGH:
            events.triggerOutState = 1;
            break;
        case EV_DEST_TRIG_LOW:
            events.triggerOutState = -1;
            break;
        case EV_DEST_LOAD_MODE:
            load_setMode(events.evlist[ev].effects[i].destMode);
            break;
        case EV_DEST_LOAD_ON:
            load.powerOn = 1;
            break;
        case EV_DEST_LOAD_OFF:
            load.powerOn = 0;
            break;
        }
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

void events_getSrcDescr(struct event ev, char* descr) {
// construct source description
    switch (ev.srcType) {
    case EV_SRC_DISABLED:
        strcpy(descr, "DISABLED");
        break;
    case EV_SRC_PARAM_HIGHER:
        strcpy(descr, "Par>:      ");
        strncpy(&descr[5], eventCompParamNames[ev.srcParamNum], 14);
        break;
    case EV_SRC_PARAM_LOWER:
        strcpy(descr, "Par<:      ");
        string_copyn(&descr[5], eventCompParamNames[ev.srcParamNum], 14);
        break;
    case EV_SRC_TIM_ZERO:
        strcpy(descr, "Timer  =0");
        descr[5] = (ev.srcTimerNum / 10) + '0';
        descr[6] = (ev.srcTimerNum % 10) + '0';
        break;
    case EV_SRC_TRIG_FALL:
        strcpy(descr, "Trigger Fall\x19");
        break;
    case EV_SRC_TRIG_RISE:
        strcpy(descr, "Trigger Rise\x18");
        break;
    case EV_SRC_WAVEFORM_PHASE:
        strcpy(descr, "WavePhase");
        break;
    }
    descr[20] = 0;
}

void events_getEffectDescr(struct effect ef, char *descr) {
    switch (ef.destType) {
    case EV_DEST_NOTHING:
        strcpy(descr, "No effect");
        break;
    case EV_DEST_LOAD_MODE:
        switch (ef.destMode) {
        case FUNCTION_CC:
            strcpy(descr, "Set mode to CC");
            break;
        case FUNCTION_CV:
            strcpy(descr, "Set mode to CV");
            break;
        case FUNCTION_CP:
            strcpy(descr, "Set mode to CP");
            break;
        case FUNCTION_CR:
            strcpy(descr, "Set mode to CR");
            break;
        default:
            strcpy(descr, "ERROR");
        }
        break;
    case EV_DEST_LOAD_OFF:
        strcpy(descr, "Turn load off");
        break;
    case EV_DEST_LOAD_ON:
        strcpy(descr, "Turn load on");
        break;
    case EV_DEST_SET_PARAM:
        strcpy(descr, "Par=:      ");
        strncpy(&descr[5], eventSetParamNames[ef.destParamNum], 14);
        break;
    case EV_DEST_SET_TIMER:
        strcpy(descr, "Set timer   ");
        descr[10] = (ef.destTimerNum / 10) + '0';
        descr[11] = (ef.destTimerNum % 10) + '0';
        break;
    case EV_DEST_TRIG_HIGH:
        strcpy(descr, "Trigger high");
        break;
    case EV_DEST_TRIG_LOW:
        strcpy(descr, "Trigger low");
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
            events_getSrcDescr(events.evlist[i], eventDescr[i]);
            descrList[i] = eventDescr[i];
        }
        ev = menu_ItemChooseDialog(
                "\xCD\xCD\xCD\xCD\xCD" "EVENT LIST\xCD\xCD\xCD\xCD\xCD\xCD",
                descrList,
                EV_MAXEVENTS, ev);
        if (ev >= 0) {
            events_editEventMenu(&events.evlist[ev]);
        }
    } while (ev >= 0);
}

void events_effectMenu(struct event *ev) {
    char effectDescr[EV_MAXEFFECTS][21];
    char *descrList[EV_MAXEFFECTS];
    uint8_t i;
    int8_t ef = 0;
    do {
        for (i = 0; i < EV_MAXEFFECTS; i++) {
            events_getEffectDescr(ev->effects[i], effectDescr[i]);
            descrList[i] = effectDescr[i];
        }
        ef = menu_ItemChooseDialog(
                "\xCD\xCD\xCD\xCD\xCD" "EFFECT LIST\xCD\xCD\xCD\xCD\xCD",
                descrList,
                EV_MAXEFFECTS, ef);
        if (ef >= 0) {
            events_editEffectMenu(&ev->effects[ef]);
        }
    } while (ef >= 0);
}

void events_editEventMenu(struct event *ev) {
    volatile uint8_t selectedRow = 1;
    uint32_t button;
    int32_t encoder;
#define EV_ROW_SRC_SRC          1
#define EV_ROW_SRC_TIMER        2
#define EV_ROW_SRC_PARAM        3
#define EV_ROW_SRC_VALUE        4
#define EV_ROW_SRC_PHASE        5
#define EV_ROW_SRC_EFFECTS      6
    uint8_t rowContent[8];
    do {
        while (hal_getButton())
            ;

        int8_t i;
        for (i = 0; i < 8; i++)
            rowContent[i] = 0;
        evSourceType_t src = ev->srcType;
        // create menu display
        screen_Clear();
        screen_FastString6x8(
                "\xCD\xCD\xCD\xCD\xCD" "EDIT EVENT\xCD\xCD\xCD\xCD\xCD\xCD", 0,
                0);
        // source display
        screen_FastString6x8("Src:", 6, 1);
        screen_FastString6x8(eventSrcNames[src], 30, 1);
        rowContent[1] = EV_ROW_SRC_SRC;
        char value[11];
        switch (src) {
        case EV_SRC_TIM_ZERO:
            screen_FastString6x8("Timer:", 6, 2);
            screen_FastChar6x8(ev->srcTimerNum + '0', 48, 2);
            rowContent[2] = EV_ROW_SRC_TIMER;
            break;
        case EV_SRC_PARAM_HIGHER:
        case EV_SRC_PARAM_LOWER:
            screen_FastString6x8("Param:", 6, 2);
            screen_FastString6x8(eventCompParamNames[ev->srcParamNum], 48, 2);
            rowContent[2] = EV_ROW_SRC_PARAM;
            screen_FastString6x8("Value:", 6, 3);
            string_fromUintUnits(ev->srcLimit, value, 5,
                    eventCompParamUnits0[ev->srcParamNum],
                    eventCompParamUnits3[ev->srcParamNum],
                    eventCompParamUnits6[ev->srcParamNum]);
            screen_FastString6x8(value, 48, 3);
            rowContent[3] = EV_ROW_SRC_VALUE;
            break;
        case EV_SRC_WAVEFORM_PHASE:
            screen_FastString6x8("Phase:", 6, 2);
            string_fromUintUnits(ev->srcLimit, value, 5, "m\xF8", "\xF8", NULL);
            screen_FastString6x8(value, 48, 2);
            rowContent[2] = EV_ROW_SRC_PHASE;
            break;
        }
        if (src != EV_SRC_DISABLED) {
            screen_FastString6x8("Event effects", 6, 7);
            rowContent[7] = EV_ROW_SRC_EFFECTS;
        }
        screen_FastChar6x8(0x1A, 0, selectedRow);
        // wait for user input
        do {
            button = hal_getButton();
            encoder = hal_getEncoderMovement();
        } while (!button && !encoder);

        if ((button & HAL_BUTTON_DOWN) || encoder > 0) {
            // move one entry down (if possible)
            for (i = selectedRow + 1; i < 8; i++) {
                if (rowContent[i] > 0) {
                    selectedRow = i;
                    break;
                }
            }
        }

        if ((button & HAL_BUTTON_UP) || encoder < 0) {
            // move one entry up
            for (i = selectedRow - 1; i > 0; i--) {
                if (rowContent[i] > 0) {
                    selectedRow = i;
                    break;
                }
            }
        }

        if ((button & HAL_BUTTON_ENTER) || (button & HAL_BUTTON_ENCODER)) {
            switch (rowContent[selectedRow]) {
            case EV_ROW_SRC_SRC: {
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
                        EV_NUM_SOURCETYPES, ev->srcType);
                if (sel >= 0) {
                    ev->srcType = sel;
                }
            }
                break;
            case EV_ROW_SRC_PARAM: {
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
                        EV_NUM_COMPPARAMS, ev->srcParamNum);
                if (sel >= 0) {
                    ev->srcParamNum = sel;
                    ev->srcParam = eventCompParamPointers[sel];
                }
            }
                break;
            case EV_ROW_SRC_VALUE: {
                // change compare value
                uint32_t val;
                if (menu_getInputValue(&val, "param limit", 0, 200000000,
                        eventCompParamUnits0[ev->srcParamNum],
                        eventCompParamUnits3[ev->srcParamNum],
                        eventCompParamUnits6[ev->srcParamNum])) {
                    ev->srcLimit = val;
                }
            }
                break;
            case EV_ROW_SRC_TIMER: {
                // select timer
                uint32_t tim;
                if (menu_getInputValue(&tim, "timer number", 0,
                EV_MAXTIMERS - 1, "Timer", NULL, NULL)) {
                    ev->srcTimerNum = tim;
                }
            }
                break;
            case EV_ROW_SRC_PHASE: {
                // change phase value
                uint32_t val;
                if (menu_getInputValue(&val, "Phase:", 0, 360000, "mDeg",
                        "Degree", NULL)) {
                    ev->srcLimit = val;
                }
            }
                break;
            case EV_ROW_SRC_EFFECTS:
                events_effectMenu(ev);
                break;
            }
        }
    } while (!(button & HAL_BUTTON_ESC));
}

void events_editEffectMenu(struct effect *ef) {
    uint8_t selectedRow = 1;
    uint32_t button;
    int32_t encoder;
#define EV_ROW_EFF_EFFECT       1
#define EV_ROW_EFF_TIMER        2
#define EV_ROW_EFF_TIMVAL       3
#define EV_ROW_EFF_PARAM        4
#define EV_ROW_EFF_PHASE        5
#define EV_ROW_EFF_VALUE        6
#define EV_ROW_EFF_MODE         7
    uint8_t rowContent[8];
    do {
        while (hal_getButton())
            ;

        int8_t i;
        for (i = 0; i < 8; i++)
            rowContent[i] = 0;
        // create menu display
        screen_Clear();
        screen_FastString6x8(
                "\xCD\xCD\xCD\xCD\xCD" "EDIT EFFECT\xCD\xCD\xCD\xCD\xCD", 0, 0);
        // destination display
        screen_FastString6x8("Effect:", 6, 1);
        screen_FastString6x8(eventDestNames[ef->destType], 48, 1);
        rowContent[1] = EV_ROW_EFF_EFFECT;
        char value[11];
        switch (ef->destType) {
        case EV_DEST_SET_TIMER:
            screen_FastString6x8("Timer:", 6, 2);
            screen_FastChar6x8(ef->destTimerNum + '0', 48, 2);
            rowContent[2] = EV_ROW_EFF_TIMER;
            screen_FastString6x8("Value:", 6, 3);
            string_fromUintUnits(ef->destTimerValue, value, 5, "ms", "s", NULL);
            screen_FastString6x8(value, 48, 3);
            rowContent[3] = EV_ROW_EFF_TIMVAL;
            break;
        case EV_DEST_SET_PARAM:
            screen_FastString6x8("Param:", 6, 2);
            screen_FastString6x8(eventSetParamNames[ef->destParamNum], 48, 2);
            rowContent[2] = EV_ROW_EFF_PARAM;
            screen_FastString6x8("Value:", 6, 3);
            string_fromUintUnits(ef->destSetValue, value, 5,
                    eventCompParamUnits0[ef->destParamNum],
                    eventCompParamUnits3[ef->destParamNum],
                    eventCompParamUnits6[ef->destParamNum]);
            screen_FastString6x8(value, 48, 3);
            rowContent[3] = EV_ROW_EFF_VALUE;
            break;
        case EV_DEST_LOAD_MODE:
            screen_FastString6x8("Mode: C", 6, 2);
            switch (ef->destMode) {
            case FUNCTION_CC:
                screen_FastChar6x8('C', 48, 2);
                break;
            case FUNCTION_CV:
                screen_FastChar6x8('V', 48, 2);
                break;
            case FUNCTION_CR:
                screen_FastChar6x8('R', 48, 2);
                break;
            case FUNCTION_CP:
                screen_FastChar6x8('P', 48, 2);
                break;
            }
            rowContent[2] = EV_ROW_EFF_MODE;
            break;
        }

        screen_FastChar6x8(0x1A, 0, selectedRow);
        // wait for user input
        do {
            button = hal_getButton();
            encoder = hal_getEncoderMovement();
        } while (!button && !encoder);

        if ((button & HAL_BUTTON_DOWN) || encoder > 0) {
            // move one entry down (if possible)
            for (i = selectedRow + 1; i < 8; i++) {
                if (rowContent[i] > 0) {
                    selectedRow = i;
                    break;
                }
            }
        }

        if ((button & HAL_BUTTON_UP) || encoder < 0) {
            // move one entry up
            for (i = selectedRow - 1; i > 0; i--) {
                if (rowContent[i] > 0) {
                    selectedRow = i;
                    break;
                }
            }
        }

        if ((button & HAL_BUTTON_ENTER) || (button & HAL_BUTTON_ENCODER)) {
            switch (rowContent[selectedRow]) {
            case EV_ROW_EFF_EFFECT: {
                // change destination settings
                char descr[EV_NUM_DESTTYPES][21];
                char *itemList[EV_NUM_DESTTYPES];
                uint8_t i;
                for (i = 0; i < EV_NUM_DESTTYPES; i++) {
                    string_copy(descr[i], eventDestNames[i]);
                    itemList[i] = descr[i];
                }
                int8_t sel = menu_ItemChooseDialog("Select effect:", itemList,
                EV_NUM_DESTTYPES, ef->destType);
                if (sel >= 0) {
                    ef->destType = sel;
                }
            }
                break;
            case EV_ROW_EFF_PARAM: {
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
                        EV_NUM_SETPARAMS, ef->destParamNum);
                if (sel >= 0) {
                    ef->destParamNum = sel;
                    ef->destParam = eventSetParamPointers[sel];
                }
            }
                break;
            case EV_ROW_EFF_VALUE: {
                // change compare value
                uint32_t val;
                if (menu_getInputValue(&val, "param value", 0, 200000000,
                        eventSetParamUnits0[ef->destParamNum],
                        eventSetParamUnits3[ef->destParamNum],
                        eventSetParamUnits6[ef->destParamNum])) {
                    ef->destSetValue = val;
                }
            }
                break;
            case EV_ROW_EFF_TIMER: {
                // select timer
                uint32_t tim;
                if (menu_getInputValue(&tim, "timer number", 0,
                EV_MAXTIMERS - 1, "Timer", NULL, NULL)) {
                    ef->destTimerNum = tim;
                }
            }
                break;
            case EV_ROW_EFF_TIMVAL: {
                // change timer start value
                uint32_t time;
                if (menu_getInputValue(&time, "time", 0, 3600000, "ms", "s",
                NULL)) {
                    ef->destTimerValue = time;
                }
            }
                break;
            case EV_ROW_EFF_MODE: {
                // change load mode
                char *modeList[4] = { "CC", "CV", "CR", "CP" };
                int8_t sel = menu_ItemChooseDialog("Select load mode:",
                        modeList, 4, ef->destMode);
                if (sel >= 0) {
                    ef->destMode = sel;
                }
            }
                break;
            }
        }
    } while (!(button & HAL_BUTTON_ESC));
}
