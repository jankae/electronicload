#include "events.h"

static const char eventSrcNames[EV_NUM_SOURCETYPES][21] = { "TRIGGER RISE",
        "TRIGGER FALL", "PAR LOWER THAN", "PAR HIGHER THAN", "TIMER ZERO",
        "WAVEFORM PHASE" };
static const char *sourceList[] = { eventSrcNames[0], eventSrcNames[1],
        eventSrcNames[2], eventSrcNames[3], eventSrcNames[4], eventSrcNames[5],
        0 };

static const char eventDestNames[EV_NUM_DESTTYPES][21] = { "TRIGGER HIGH",
        "TRIGGER LOW", "SET PARAMETER", "SET TIMER", "SET LOAD MODE",
        "INPUT ON", "INPUT OFF" };

static const char eventSetParamNames[EV_NUM_SETPARAMS][21] = { "CURRENT",
        "VOLTAGE", "RESISTANCE", "POWER" };

static const char eventSetParamUnits0[EV_NUM_SETPARAMS][6] = { "uA", "uV",
        "mOhm", "uW" };

static const char eventSetParamUnits3[EV_NUM_SETPARAMS][6] = { "mA", "mV",
        "Ohm", "mW" };

static const char eventSetParamUnits6[EV_NUM_SETPARAMS][6] = { "A", "V", "kOhm",
        "W" };

static const uint32_t *eventSetParamPointers[EV_NUM_SETPARAMS] = {
        &load.current, &load.voltage, &load.resistance, &load.power };

static const char eventCompParamNames[EV_NUM_COMPPARAMS][21] = { "CURRENT",
        "VOLTAGE", "POWER", "SET CURRENT", "SET VOLTAGE", "SET RESIST.",
        "SET POWER" };
static const char* compParamList[EV_NUM_COMPPARAMS + 1] = {
        eventCompParamNames[0], eventCompParamNames[1], eventCompParamNames[2],
        eventCompParamNames[3], eventCompParamNames[4], eventCompParamNames[5],
        eventCompParamNames[6], 0 };

static const unit_t eventCompParamUnits[EV_NUM_COMPPARAMS] = { UNIT_CURRENT,
        UNIT_VOLTAGE, UNIT_POWER, UNIT_CURRENT, UNIT_VOLTAGE, UNIT_RESISTANCE,
        UNIT_POWER };

static uint32_t *eventCompParamPointers[EV_NUM_COMPPARAMS] = {
        &load.state.current, &load.state.voltage, &load.state.power,
        &load.current, &load.voltage, &load.resistance, &load.power };

static event_t eventList[EV_MAXEVENTS];
static eventEffect_t effectList[EV_MAXEFFECTS];

/* GUI elements */
static container_t c;
/* Buttons to add/delete events */
static button_t bAddEvent, bDeleteEvent;
/* selected event */
static dropdown_t dEvents;
/* source of the selected event */
static dropdown_t dSource;
/* source parameter of the selected event (only visible if EV_SRC_PARAM_HIGHER or EV_SRC_PARAM_LOWER) */
static dropdown_t dSrcParam;

static label_t lEvents, lSource, lSrcSetting1, lSrcSetting2;
static entry_t eSrcSetting1;
static entry_t eSrcSetting2;
/* pointers to the event descriptions */
static char *eventDescriptions[EV_MAXEVENTS + 1];

static uint8_t selectedEvent;

static uint32_t null = 0;
static uint32_t maxTimers = EV_MAXTIMERS - 1;
static uint32_t maxDegree = 359999;

void events_Init(void) {
    events.firstEvent = NULL;
    uint8_t i;
    for (i = 0; i < EV_MAXEVENTS; i++) {
        eventList[i].inUse = 0;
    }
    for (i = 0; i < EV_MAXEFFECTS; i++) {
        effectList[i].inUse = 0;
    }

    events_UpdateDescriptions();

    /* create GUI elements */
    /* these elements will always be displayed */
    label_createWithText(&lEvents, "Events:", FONT_MEDIUM);
    button_create(&bAddEvent, "Add", FONT_MEDIUM, 0, events_AddEvent);
    button_create(&bDeleteEvent, "Delete", FONT_MEDIUM, 0, events_DeleteEvent);

    /* these elements will only be displayed if there is an event */
    dropdown_create(&dEvents, eventDescriptions, &selectedEvent, FONT_MEDIUM,
            124, events_SetGUIToSelectedEvent);
    label_createWithText(&lSource, "Src:", FONT_MEDIUM);
    dropdown_create(&dSource, sourceList, NULL, FONT_MEDIUM, 100,
            events_SourceChanged);

    /* these elements will only be displayed if the event source is "Param higher/lower" */
    label_createWithLength(&lSrcSetting1, 6, FONT_MEDIUM);
    label_createWithLength(&lSrcSetting2, 10, FONT_MEDIUM);
    dropdown_create(&dSrcParam, compParamList, NULL, FONT_MEDIUM, 88,
            events_SetGUIToSelectedEvent);
    entry_create(&eSrcSetting1, NULL, NULL, NULL, FONT_MEDIUM, 4, UNIT_CURRENT,
            events_UpdateDescriptions);
    entry_create(&eSrcSetting2, NULL, NULL, NULL, FONT_MEDIUM, 4, UNIT_CURRENT,
            events_UpdateDescriptions);

    events_SetGUIToSelectedEvent();

    container_create(&c, 128, 55);

    container_attach(&c, &lEvents, 0, 2);
    container_attach(&c, &bAddEvent, 42, 0);
    container_attach(&c, &bDeleteEvent, 70, 0);
    container_attach(&c, &dEvents, 0, 13);
    container_attach(&c, &lSource, 0, 27);
    container_attach(&c, &dSource, 24, 25);

    container_attach(&c, &lSrcSetting1, 0, 39);
    container_attach(&c, &lSrcSetting2, 0, 51);
    container_attach(&c, &dSrcParam, 36, 37);
    container_attach(&c, &eSrcSetting1, 79, 37);
    container_attach(&c, &eSrcSetting2, 79, 49);
}

widget_t* events_getWidget(void) {
    return (widget_t*) &c;
}

void events_AddEvent(void) {
    /* try to allocate new event */
    event_t *new = event_allocateEvent();
    if (!new) {
        /* failed to allocate event */
        message_Box("Already at\nmaximum number\nof events", 3, 14, FONT_MEDIUM,
                MESSAGE_OK, &c);
        return;
    }
    event_t *last = events.firstEvent;
    selectedEvent = 0;
    if (!last) {
        /* this is the first event */
        events.firstEvent = new;
    } else {
        selectedEvent++;
        /* find end of event list */
        for (; last->next != NULL; last = last->next)
            selectedEvent++;
        /* add new event to end of list */
        last->next = new;
    }
    events_SetGUIToSelectedEvent();
}

void events_DeleteEvent(void) {
    if (!events.firstEvent) {
        /* nothing to delete */
        return;
    }
    if (selectedEvent == 0) {
        /* remove first item */
        event_t *nextBuffer = events.firstEvent->next;
        event_freeEvent(events.firstEvent);
        events.firstEvent = nextBuffer;
    } else {
        /* find selected event */
        event_t *deletor = events.firstEvent;
        uint8_t i = selectedEvent;
        event_t *previous = NULL;
        while (i && deletor) {
            if (i == 1)
                previous = deletor;
            i--;
            deletor = deletor->next;
        }
        if (!deletor) {
            /* couldn't find deletor */
            return;
        }
        /* remove deletor from event list */
        previous->next = deletor->next;
        event_freeEvent(deletor);
        selectedEvent--;
    }
    events_SetGUIToSelectedEvent();
}

void events_SourceChanged(void){
    if (!events.firstEvent) {
        /* error */
        return;
    }
    /* find event */
    event_t *ev = events.firstEvent;
    uint8_t i = selectedEvent;
    while (i && ev) {
        ev = ev->next;
        i--;
    }
    if (!ev) {
        /* some sort of error, shouldn't happen */
        return;
    }
    /* initialize values */
    switch(ev->srcType) {
    case EV_SRC_PARAM_HIGHER:
    case EV_SRC_PARAM_LOWER:
        ev->srcParamNum = 0;
        ev->srcLimit = 0;
        ev->srcParam = eventCompParamPointers[ev->srcParamNum];
        break;
    case EV_SRC_TIM_ZERO:
        ev->srcTimerNum = 0;
        break;
    case EV_SRC_WAVEFORM_PHASE:
        ev->phase = 0;
        break;
    }
    events_SetGUIToSelectedEvent();
}

void events_UpdateDescriptions(void) {
    event_t *ev;
    uint8_t i = 0;
    for (ev = events.firstEvent; ev != NULL; ev = ev->next) {
        eventDescriptions[i++] = ev->descr;
        /* update description for this event */
        switch (ev->srcType) {
        case EV_SRC_PARAM_LOWER:
        case EV_SRC_PARAM_HIGHER:
            strcpy(ev->descr, eventCompParamNames[ev->srcParamNum]);
            char arrow = '<';
            if (ev->srcType == EV_SRC_PARAM_HIGHER) {
                ev->descr[11] = '>';
            }
            uint8_t pos = strlen(ev->descr);
            ev->descr[pos] = arrow;
            string_fromUintUnits(ev->srcLimit, &ev->descr[pos + 1], 4,
                    unitNames[eventCompParamUnits[ev->srcParamNum]][0],
                    unitNames[eventCompParamUnits[ev->srcParamNum]][1],
                    unitNames[eventCompParamUnits[ev->srcParamNum]][2]);
            break;
        case EV_SRC_TIM_ZERO:
            strcpy(ev->descr, "Timer  =0");
            ev->descr[5] = (ev->srcTimerNum / 10) + '0';
            ev->descr[6] = (ev->srcTimerNum % 10) + '0';
            break;
        case EV_SRC_TRIG_FALL:
            strcpy(ev->descr, "Trigger Fall\x19");
            break;
        case EV_SRC_TRIG_RISE:
            strcpy(ev->descr, "Trigger Rise\x18");
            break;
        case EV_SRC_WAVEFORM_PHASE:
            strcpy(ev->descr, "WavePhase=");
            string_fromUintUnits(ev->phase, &ev->descr[10], 4,
                    unitNames[UNIT_DEGREE][0],
                    unitNames[UNIT_DEGREE][1],
                    unitNames[UNIT_DEGREE][2]);
            break;
        }
    }
    eventDescriptions[i] = 0;
}

void events_SetGUIToSelectedEvent(void) {
    events_UpdateDescriptions();
    /* set elements invisible */
    dEvents.base.flags.visible = 0;
    dSource.base.flags.visible = 0;
    lSource.base.flags.visible = 0;
    lSrcSetting1.base.flags.visible = 0;
    eSrcSetting1.base.flags.visible = 0;
    dSrcParam.base.flags.visible = 0;
    lSrcSetting2.base.flags.visible = 0;
    eSrcSetting2.base.flags.visible = 0;
    /* check if there is an event */
    if (events.firstEvent) {
        /* base elements visible */
        dEvents.base.flags.visible = 1;
        dSource.base.flags.visible = 1;
        lSource.base.flags.visible = 1;

        /* find event */
        event_t *ev = events.firstEvent;
        uint8_t i = selectedEvent;
        while (i && ev) {
            ev = ev->next;
            i--;
        }
        if (!ev) {
            /* some sort of error, shouldn't happen */
            return;
        }
        dSource.value = &ev->srcType;
        switch (ev->srcType) {
        case EV_SRC_PARAM_HIGHER:
        case EV_SRC_PARAM_LOWER:
            lSrcSetting1.base.flags.visible = 1;
            label_SetText(&lSrcSetting1, "Param:");
            dSrcParam.base.flags.visible = 1;
            dSrcParam.value = &ev->srcParamNum;
            lSrcSetting2.base.flags.visible = 1;
            label_SetText(&lSrcSetting2, "Threshold:");
            eSrcSetting2.base.flags.visible = 1;
            eSrcSetting2.value = &ev->srcLimit;
            eSrcSetting2.unit = eventCompParamUnits[ev->srcParamNum];
            break;
        case EV_SRC_TRIG_FALL:
        case EV_SRC_TRIG_RISE:
            break;
        case EV_SRC_TIM_ZERO:
            lSrcSetting1.base.flags.visible = 1;
            label_SetText(&lSrcSetting1, "Timer:");
            eSrcSetting1.base.flags.visible = 1;
            eSrcSetting1.value = &ev->srcTimerNum;
            eSrcSetting1.unit = UNIT_ITERATOR;
            eSrcSetting1.min = &null;
            eSrcSetting1.max = &maxTimers;
            break;
        case EV_SRC_WAVEFORM_PHASE:
            lSrcSetting1.base.flags.visible = 1;
            label_SetText(&lSrcSetting1, "Phase:");
            eSrcSetting1.base.flags.visible = 1;
            eSrcSetting1.value = &ev->phase;
            eSrcSetting1.unit = UNIT_DEGREE;
            eSrcSetting1.min = &null;
            eSrcSetting1.max = &maxDegree;
            break;
        }
    }
}

void events_HandleEvents(void) {
    event_t *ev;
    for (ev = events.firstEvent; ev != NULL; ev = ev->next) {
        events_UpdateTrigger(ev);
    }
    for (ev = events.firstEvent; ev != NULL; ev = ev->next) {
        if (ev->triggered) {
            events_triggerEventDestination(ev);
        }
    }
}

void events_UpdateTrigger(event_t *ev) {
    ev->triggered = 0;
    switch (ev->srcType) {
    case EV_SRC_TIM_ZERO:
        if (events.evTimers[ev->srcTimerNum] == 0)
            ev->triggered = 1;
        break;
    case EV_SRC_PARAM_HIGHER:
        if (*(ev->srcParam) > ev->srcLimit)
            ev->triggered = 1;
        break;
    case EV_SRC_PARAM_LOWER:
        if (*(ev->srcParam) < ev->srcLimit)
            ev->triggered = 1;
        break;
    case EV_SRC_TRIG_FALL:
        if (events.triggerInState == -1)
            ev->triggered = 1;
        break;
    case EV_SRC_TRIG_RISE:
        if (events.triggerInState == 1)
            ev->triggered = 1;
        break;
    case EV_SRC_WAVEFORM_PHASE:
        if (waveform.switchedOn) {
            // check whether phase limit has been passed since last cycle
            if (events.waveformPhase >= events.waveformOldPhase) {
                // phase hasn't passed 360°
                if (ev->srcLimit <= events.waveformPhase
                        && ev->srcLimit > events.waveformOldPhase) {
                    ev->triggered = 1;
                }
            } else {
                // phase has passed 360°
                if (ev->srcLimit > events.waveformOldPhase
                        || ev->srcLimit <= events.waveformPhase)
                    ev->triggered = 1;
            }
        }
        break;
    }
}

void events_triggerEventDestination(event_t *ev) {
    eventEffect_t *ef;
    for (ef = ev->firstEffect; ef != NULL; ef = ef->next) {
        switch (ef->destType) {
        case EV_DEST_SET_PARAM:
            *(ef->destParam) = ef->destSetValue;
            break;
        case EV_DEST_SET_TIMER:
            events.evTimers[ef->destTimerNum] = ef->destTimerValue;
            break;
        case EV_DEST_TRIG_HIGH:
            events.triggerOutState = 1;
            break;
        case EV_DEST_TRIG_LOW:
            events.triggerOutState = -1;
            break;
        case EV_DEST_LOAD_MODE:
            load_setMode(ef->destMode);
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

event_t* event_allocateEvent(void) {
    uint8_t i;
    for (i = 0; i < EV_MAXEVENTS; i++) {
        if (!eventList[i].inUse) {
            memset(&eventList[i], 0, sizeof(event_t));
            eventList[i].inUse = 1;
            return &eventList[i];
        }
    }
    return NULL;
}

eventEffect_t* event_allocateEffect(void) {
    uint8_t i;
    for (i = 0; i < EV_MAXEFFECTS; i++) {
        if (!effectList[i].inUse) {
            effectList[i].inUse = 1;
            return &effectList[i];
        }
    }
    return NULL;
}

//void events_getDescr(struct event ev, char* descr) {
//// construct source description
//    switch (ev.srcType) {
//    case EV_SRC_PARAM_HIGHER:
//        strcpy(descr, "Par>:      ");
//        strncpy(&descr[5], eventCompParamNames[ev.srcParamNum], 14);
//        break;
//    case EV_SRC_PARAM_LOWER:
//        strcpy(descr, "Par<:      ");
//        string_copyn(&descr[5], eventCompParamNames[ev.srcParamNum], 14);
//        break;
//    case EV_SRC_TIM_ZERO:
//        strcpy(descr, "Timer  =0");
//        descr[5] = (ev.srcTimerNum / 10) + '0';
//        descr[6] = (ev.srcTimerNum % 10) + '0';
//        break;
//    case EV_SRC_TRIG_FALL:
//        strcpy(descr, "Trigger Fall\x19");
//        break;
//    case EV_SRC_TRIG_RISE:
//        strcpy(descr, "Trigger Rise\x18");
//        break;
//    case EV_SRC_WAVEFORM_PHASE:
//        strcpy(descr, "WavePhase");
//        break;
//    }
//    descr[20] = 0;
//}
//
//void events_getEffectDescr(struct effect ef, char *descr) {
//    switch (ef.destType) {
//    case EV_DEST_NOTHING:
//        strcpy(descr, "No effect");
//        break;
//    case EV_DEST_LOAD_MODE:
//        switch (ef.destMode) {
//        case FUNCTION_CC:
//            strcpy(descr, "Set mode to CC");
//            break;
//        case FUNCTION_CV:
//            strcpy(descr, "Set mode to CV");
//            break;
//        case FUNCTION_CP:
//            strcpy(descr, "Set mode to CP");
//            break;
//        case FUNCTION_CR:
//            strcpy(descr, "Set mode to CR");
//            break;
//        default:
//            strcpy(descr, "ERROR");
//        }
//        break;
//    case EV_DEST_LOAD_OFF:
//        strcpy(descr, "Turn load off");
//        break;
//    case EV_DEST_LOAD_ON:
//        strcpy(descr, "Turn load on");
//        break;
//    case EV_DEST_SET_PARAM:
//        strcpy(descr, "Par=:      ");
//        strncpy(&descr[5], eventSetParamNames[ef.destParamNum], 14);
//        break;
//    case EV_DEST_SET_TIMER:
//        strcpy(descr, "Set timer   ");
//        descr[10] = (ef.destTimerNum / 10) + '0';
//        descr[11] = (ef.destTimerNum % 10) + '0';
//        break;
//    case EV_DEST_TRIG_HIGH:
//        strcpy(descr, "Trigger high");
//        break;
//    case EV_DEST_TRIG_LOW:
//        strcpy(descr, "Trigger low");
//        break;
//    }
//}
//
//void events_menu(void) {
//    char eventDescr[EV_MAXEVENTS][21];
//    char *descrList[EV_MAXEVENTS];
//    uint8_t i;
//    int8_t ev = 0;
//    do {
//        for (i = 0; i < EV_MAXEVENTS; i++) {
//            events_getSrcDescr(events.evlist[i], eventDescr[i]);
//            descrList[i] = eventDescr[i];
//        }
//        ev = menu_ItemChooseDialog(
//                "\xCD\xCD\xCD\xCD\xCD" "EVENT LIST\xCD\xCD\xCD\xCD\xCD\xCD",
//                descrList,
//                EV_MAXEVENTS, ev);
//        if (ev >= 0) {
//            events_editEventMenu(&events.evlist[ev]);
//        }
//    } while (ev >= 0);
//}
//
//void events_effectMenu(struct event *ev) {
//    char effectDescr[EV_MAXEFFECTS][21];
//    char *descrList[EV_MAXEFFECTS];
//    uint8_t i;
//    int8_t ef = 0;
//    do {
//        for (i = 0; i < EV_MAXEFFECTS; i++) {
//            events_getEffectDescr(ev->effects[i], effectDescr[i]);
//            descrList[i] = effectDescr[i];
//        }
//        ef = menu_ItemChooseDialog(
//                "\xCD\xCD\xCD\xCD\xCD" "EFFECT LIST\xCD\xCD\xCD\xCD\xCD",
//                descrList,
//                EV_MAXEFFECTS, ef);
//        if (ef >= 0) {
//            events_editEffectMenu(&ev->effects[ef]);
//        }
//    } while (ef >= 0);
//}
//
//void events_editEventMenu(struct event *ev) {
//    volatile uint8_t selectedRow = 1;
//    uint32_t button;
//    int32_t encoder;
//#define EV_ROW_SRC_SRC          1
//#define EV_ROW_SRC_TIMER        2
//#define EV_ROW_SRC_PARAM        3
//#define EV_ROW_SRC_VALUE        4
//#define EV_ROW_SRC_PHASE        5
//#define EV_ROW_SRC_EFFECTS      6
//    uint8_t rowContent[8];
//    do {
//        while (hal_getButton())
//            ;
//
//        int8_t i;
//        for (i = 0; i < 8; i++)
//            rowContent[i] = 0;
//        evSourceType_t src = ev->srcType;
//        // create menu display
//        screen_Clear();
//        screen_FastString6x8(
//                "\xCD\xCD\xCD\xCD\xCD" "EDIT EVENT\xCD\xCD\xCD\xCD\xCD\xCD", 0,
//                0);
//        // source display
//        screen_FastString6x8("Src:", 6, 1);
//        screen_FastString6x8(eventSrcNames[src], 30, 1);
//        rowContent[1] = EV_ROW_SRC_SRC;
//        char value[11];
//        switch (src) {
//        case EV_SRC_TIM_ZERO:
//            screen_FastString6x8("Timer:", 6, 2);
//            screen_FastChar6x8(ev->srcTimerNum + '0', 48, 2);
//            rowContent[2] = EV_ROW_SRC_TIMER;
//            break;
//        case EV_SRC_PARAM_HIGHER:
//        case EV_SRC_PARAM_LOWER:
//            screen_FastString6x8("Param:", 6, 2);
//            screen_FastString6x8(eventCompParamNames[ev->srcParamNum], 48, 2);
//            rowContent[2] = EV_ROW_SRC_PARAM;
//            screen_FastString6x8("Value:", 6, 3);
//            string_fromUintUnits(ev->srcLimit, value, 5,
//                    eventCompParamUnits0[ev->srcParamNum],
//                    eventCompParamUnits3[ev->srcParamNum],
//                    eventCompParamUnits6[ev->srcParamNum]);
//            screen_FastString6x8(value, 48, 3);
//            rowContent[3] = EV_ROW_SRC_VALUE;
//            break;
//        case EV_SRC_WAVEFORM_PHASE:
//            screen_FastString6x8("Phase:", 6, 2);
//            string_fromUintUnits(ev->srcLimit, value, 5, "m\xF8", "\xF8", NULL);
//            screen_FastString6x8(value, 48, 2);
//            rowContent[2] = EV_ROW_SRC_PHASE;
//            break;
//        }
//        if (src != EV_SRC_DISABLED) {
//            screen_FastString6x8("Event effects", 6, 7);
//            rowContent[7] = EV_ROW_SRC_EFFECTS;
//        }
//        screen_FastChar6x8(0x1A, 0, selectedRow);
//        // wait for user input
//        do {
//            button = hal_getButton();
//            encoder = hal_getEncoderMovement();
//        } while (!button && !encoder);
//
//        if ((button & HAL_BUTTON_DOWN) || encoder > 0) {
//            // move one entry down (if possible)
//            for (i = selectedRow + 1; i < 8; i++) {
//                if (rowContent[i] > 0) {
//                    selectedRow = i;
//                    break;
//                }
//            }
//        }
//
//        if ((button & HAL_BUTTON_UP) || encoder < 0) {
//            // move one entry up
//            for (i = selectedRow - 1; i > 0; i--) {
//                if (rowContent[i] > 0) {
//                    selectedRow = i;
//                    break;
//                }
//            }
//        }
//
//        if ((button & HAL_BUTTON_ENTER) || (button & HAL_BUTTON_ENCODER)) {
//            switch (rowContent[selectedRow]) {
//            case EV_ROW_SRC_SRC: {
//                // change source settings
//                char descr[EV_NUM_SOURCETYPES][21];
//                char *itemList[EV_NUM_SOURCETYPES];
//                uint8_t i;
//                for (i = 0; i < EV_NUM_SOURCETYPES; i++) {
//                    string_copy(descr[i], eventSrcNames[i]);
//                    itemList[i] = descr[i];
//                }
//                int8_t sel = menu_ItemChooseDialog("Select event source:",
//                        itemList,
//                        EV_NUM_SOURCETYPES, ev->srcType);
//                if (sel >= 0) {
//                    ev->srcType = sel;
//                }
//            }
//                break;
//            case EV_ROW_SRC_PARAM: {
//                // change compare parameter
//                char descr[EV_NUM_COMPPARAMS][21];
//                char *itemList[EV_NUM_COMPPARAMS];
//                uint8_t i;
//                for (i = 0; i < EV_NUM_COMPPARAMS; i++) {
//                    string_copy(descr[i], eventCompParamNames[i]);
//                    itemList[i] = descr[i];
//                }
//                int8_t sel = menu_ItemChooseDialog("Select parameter:",
//                        itemList,
//                        EV_NUM_COMPPARAMS, ev->srcParamNum);
//                if (sel >= 0) {
//                    ev->srcParamNum = sel;
//                    ev->srcParam = eventCompParamPointers[sel];
//                }
//            }
//                break;
//            case EV_ROW_SRC_VALUE: {
//                // change compare value
//                uint32_t val;
//                if (menu_getInputValue(&val, "param limit", 0, 200000000,
//                        eventCompParamUnits0[ev->srcParamNum],
//                        eventCompParamUnits3[ev->srcParamNum],
//                        eventCompParamUnits6[ev->srcParamNum])) {
//                    ev->srcLimit = val;
//                }
//            }
//                break;
//            case EV_ROW_SRC_TIMER: {
//                // select timer
//                uint32_t tim;
//                if (menu_getInputValue(&tim, "timer number", 0,
//                EV_MAXTIMERS - 1, "Timer", NULL, NULL)) {
//                    ev->srcTimerNum = tim;
//                }
//            }
//                break;
//            case EV_ROW_SRC_PHASE: {
//                // change phase value
//                uint32_t val;
//                if (menu_getInputValue(&val, "Phase:", 0, 360000, "mDeg",
//                        "Degree", NULL)) {
//                    ev->srcLimit = val;
//                }
//            }
//                break;
//            case EV_ROW_SRC_EFFECTS:
//                events_effectMenu(ev);
//                break;
//            }
//        }
//    } while (!(button & HAL_BUTTON_ESC));
//}
//
//void events_editEffectMenu(struct effect *ef) {
//    uint8_t selectedRow = 1;
//    uint32_t button;
//    int32_t encoder;
//#define EV_ROW_EFF_EFFECT       1
//#define EV_ROW_EFF_TIMER        2
//#define EV_ROW_EFF_TIMVAL       3
//#define EV_ROW_EFF_PARAM        4
//#define EV_ROW_EFF_PHASE        5
//#define EV_ROW_EFF_VALUE        6
//#define EV_ROW_EFF_MODE         7
//    uint8_t rowContent[8];
//    do {
//        while (hal_getButton())
//            ;
//
//        int8_t i;
//        for (i = 0; i < 8; i++)
//            rowContent[i] = 0;
//        // create menu display
//        screen_Clear();
//        screen_FastString6x8(
//                "\xCD\xCD\xCD\xCD\xCD" "EDIT EFFECT\xCD\xCD\xCD\xCD\xCD", 0, 0);
//        // destination display
//        screen_FastString6x8("Effect:", 6, 1);
//        screen_FastString6x8(eventDestNames[ef->destType], 48, 1);
//        rowContent[1] = EV_ROW_EFF_EFFECT;
//        char value[11];
//        switch (ef->destType) {
//        case EV_DEST_SET_TIMER:
//            screen_FastString6x8("Timer:", 6, 2);
//            screen_FastChar6x8(ef->destTimerNum + '0', 48, 2);
//            rowContent[2] = EV_ROW_EFF_TIMER;
//            screen_FastString6x8("Value:", 6, 3);
//            string_fromUintUnits(ef->destTimerValue, value, 5, "ms", "s", NULL);
//            screen_FastString6x8(value, 48, 3);
//            rowContent[3] = EV_ROW_EFF_TIMVAL;
//            break;
//        case EV_DEST_SET_PARAM:
//            screen_FastString6x8("Param:", 6, 2);
//            screen_FastString6x8(eventSetParamNames[ef->destParamNum], 48, 2);
//            rowContent[2] = EV_ROW_EFF_PARAM;
//            screen_FastString6x8("Value:", 6, 3);
//            string_fromUintUnits(ef->destSetValue, value, 5,
//                    eventCompParamUnits0[ef->destParamNum],
//                    eventCompParamUnits3[ef->destParamNum],
//                    eventCompParamUnits6[ef->destParamNum]);
//            screen_FastString6x8(value, 48, 3);
//            rowContent[3] = EV_ROW_EFF_VALUE;
//            break;
//        case EV_DEST_LOAD_MODE:
//            screen_FastString6x8("Mode: C", 6, 2);
//            switch (ef->destMode) {
//            case FUNCTION_CC:
//                screen_FastChar6x8('C', 48, 2);
//                break;
//            case FUNCTION_CV:
//                screen_FastChar6x8('V', 48, 2);
//                break;
//            case FUNCTION_CR:
//                screen_FastChar6x8('R', 48, 2);
//                break;
//            case FUNCTION_CP:
//                screen_FastChar6x8('P', 48, 2);
//                break;
//            }
//            rowContent[2] = EV_ROW_EFF_MODE;
//            break;
//        }
//
//        screen_FastChar6x8(0x1A, 0, selectedRow);
//        // wait for user input
//        do {
//            button = hal_getButton();
//            encoder = hal_getEncoderMovement();
//        } while (!button && !encoder);
//
//        if ((button & HAL_BUTTON_DOWN) || encoder > 0) {
//            // move one entry down (if possible)
//            for (i = selectedRow + 1; i < 8; i++) {
//                if (rowContent[i] > 0) {
//                    selectedRow = i;
//                    break;
//                }
//            }
//        }
//
//        if ((button & HAL_BUTTON_UP) || encoder < 0) {
//            // move one entry up
//            for (i = selectedRow - 1; i > 0; i--) {
//                if (rowContent[i] > 0) {
//                    selectedRow = i;
//                    break;
//                }
//            }
//        }
//
//        if ((button & HAL_BUTTON_ENTER) || (button & HAL_BUTTON_ENCODER)) {
//            switch (rowContent[selectedRow]) {
//            case EV_ROW_EFF_EFFECT: {
//                // change destination settings
//                char descr[EV_NUM_DESTTYPES][21];
//                char *itemList[EV_NUM_DESTTYPES];
//                uint8_t i;
//                for (i = 0; i < EV_NUM_DESTTYPES; i++) {
//                    string_copy(descr[i], eventDestNames[i]);
//                    itemList[i] = descr[i];
//                }
//                int8_t sel = menu_ItemChooseDialog("Select effect:", itemList,
//                EV_NUM_DESTTYPES, ef->destType);
//                if (sel >= 0) {
//                    ef->destType = sel;
//                }
//            }
//                break;
//            case EV_ROW_EFF_PARAM: {
//                // change set parameter
//                char descr[EV_NUM_SETPARAMS][21];
//                char *itemList[EV_NUM_SETPARAMS];
//                uint8_t i;
//                for (i = 0; i < EV_NUM_SETPARAMS; i++) {
//                    string_copy(descr[i], eventSetParamNames[i]);
//                    itemList[i] = descr[i];
//                }
//                int8_t sel = menu_ItemChooseDialog("Select parameter:",
//                        itemList,
//                        EV_NUM_SETPARAMS, ef->destParamNum);
//                if (sel >= 0) {
//                    ef->destParamNum = sel;
//                    ef->destParam = eventSetParamPointers[sel];
//                }
//            }
//                break;
//            case EV_ROW_EFF_VALUE: {
//                // change compare value
//                uint32_t val;
//                if (menu_getInputValue(&val, "param value", 0, 200000000,
//                        eventSetParamUnits0[ef->destParamNum],
//                        eventSetParamUnits3[ef->destParamNum],
//                        eventSetParamUnits6[ef->destParamNum])) {
//                    ef->destSetValue = val;
//                }
//            }
//                break;
//            case EV_ROW_EFF_TIMER: {
//                // select timer
//                uint32_t tim;
//                if (menu_getInputValue(&tim, "timer number", 0,
//                EV_MAXTIMERS - 1, "Timer", NULL, NULL)) {
//                    ef->destTimerNum = tim;
//                }
//            }
//                break;
//            case EV_ROW_EFF_TIMVAL: {
//                // change timer start value
//                uint32_t time;
//                if (menu_getInputValue(&time, "time", 0, 3600000, "ms", "s",
//                NULL)) {
//                    ef->destTimerValue = time;
//                }
//            }
//                break;
//            case EV_ROW_EFF_MODE: {
//                // change load mode
//                char *modeList[4] = { "CC", "CV", "CR", "CP" };
//                int8_t sel = menu_ItemChooseDialog("Select load mode:",
//                        modeList, 4, ef->destMode);
//                if (sel >= 0) {
//                    ef->destMode = sel;
//                }
//            }
//                break;
//            }
//        }
//    } while (!(button & HAL_BUTTON_ESC));
//}
