#include "events.h"

static const char eventSrcNames[EV_NUM_SOURCETYPES][21] = { "TRIGGER RISE",
        "TRIGGER FALL", "PAR LOWER THAN", "PAR HIGHER THAN", "TIMER ZERO",
        "WAVEFORM PHASE" };
static const char * const sourceList[] = { eventSrcNames[0], eventSrcNames[1],
        eventSrcNames[2], eventSrcNames[3], eventSrcNames[4], eventSrcNames[5],
        0 };

static const char eventDestNames[EV_NUM_DESTTYPES][21] = { "TRIGGER HIGH",
        "TRIGGER LOW", "SET PARAMETER", "SET TIMER", "SET LOAD MODE",
        "INPUT ON", "INPUT OFF" };
static const char * const destList[] = { eventDestNames[0], eventDestNames[1],
        eventDestNames[2], eventDestNames[3], eventDestNames[4],
        eventDestNames[5], eventDestNames[6], 0 };

static const char * const loadModeList[] = { loadModeNames[0], loadModeNames[1],
        loadModeNames[2], loadModeNames[3], 0 };

static const char eventSetParamNames[EV_NUM_SETPARAMS][21] = { "SET CURRENT",
        "SET VOLTAGE", "SET RESIST.", "SET POWER" };
static const char* const setParamList[EV_NUM_SETPARAMS + 1] = {
        eventSetParamNames[0], eventSetParamNames[1], eventSetParamNames[2],
        eventSetParamNames[3], 0 };

static const unit_t eventSetParamUnits[EV_NUM_SETPARAMS] = { UNIT_CURRENT,
        UNIT_VOLTAGE, UNIT_RESISTANCE, UNIT_POWER };

static uint32_t * const eventSetParamPointers[EV_NUM_SETPARAMS] = {
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

static uint32_t * const eventCompParamPointers[EV_NUM_COMPPARAMS] = {
        &load.state.current, &load.state.voltage, &load.state.power,
        &load.current, &load.voltage, &load.resistance, &load.power };

static event_t eventList[EV_MAXEVENTS];
static eventEffect_t effectList[EV_MAXEFFECTS];

/* GUI elements */
static container_t c;

/* EVENT EDIT ELEMENTS */
static label_t lEvents;
/* Buttons to add/delete events */
static button_t bAddEvent, bDeleteEvent;
/* selected event */
static dropdown_t dEvents;
/* source of the selected event */
static label_t lSource;
static dropdown_t dSource;
/* source parameter of the selected event (only visible if EV_SRC_PARAM_HIGHER or EV_SRC_PARAM_LOWER) */
static dropdown_t dSrcParam;
/* generic source settings (will be used for different parameters depending on selected source) */
static label_t lSrcSetting1, lSrcSetting2;
static entry_t eSrcSetting1;
static entry_t eSrcSetting2;
/* pointers to the event descriptions */
static const char *eventDescriptions[EV_MAXEVENTS + 1];
static uint8_t selectedEventNum;
static event_t *selectedEvent;

/* EFFECT EDIT ELEMENTS */
static label_t lEffects;
/* Buttons to add/delete events */
static button_t bAddEffect, bDeleteEffect;
/* selected event */
static dropdown_t dEffects;
/* destination of the selected effect */
static label_t lDest;
static dropdown_t dDest;
/* generic destination settings (will be used for different parameters depending on selected effect) */
static label_t lDestSetting1, lDestSetting2;
static dropdown_t dDestSetting1;
static entry_t eDestSetting1, eDestSetting2;

static const char *effectDescriptions[EV_MAXEFFECTS + 1];
static uint8_t selectedEffectNum;
static eventEffect_t *selectedEffect;

static uint32_t null = 0;
static uint32_t maxTimers = EV_MAXTIMERS - 1;
static uint32_t maxDegree = 359999;
static uint32_t maxTimerValue = 9999000;

void events_Init(void) {
    events.firstEvent = NULL;
    uint8_t i;
    for (i = 0; i < EV_MAXEVENTS; i++) {
        eventList[i].inUse = 0;
    }
    for (i = 0; i < EV_MAXEFFECTS; i++) {
        effectList[i].inUse = 0;
    }

    selectedEventNum = 0;
    selectedEvent = NULL;
    selectedEffectNum = 0;
    selectedEffect = NULL;

    events_UpdateDescrPointers();

    /* create GUI elements */
    /* ELEMENTS TO ADD/DELETE EVENTS */
    /* these elements will always be displayed */
    label_createWithText(&lEvents, "Events:", FONT_MEDIUM);
    button_create(&bAddEvent, "Add", FONT_MEDIUM, 0, events_AddEvent);
    button_create(&bDeleteEvent, "Delete", FONT_MEDIUM, 0, events_DeleteEvent);

    /* ELEMENTS TO EDIT THE SELECTED EVENT */
    /* these elements will only be displayed if there is an event */
    dropdown_create(&dEvents, eventDescriptions, &selectedEventNum, FONT_MEDIUM,
            124, events_SelectedEventChanged);
    label_createWithText(&lSource, "Src:", FONT_MEDIUM);
    dropdown_create(&dSource, sourceList, NULL, FONT_MEDIUM, 100,
            events_SourceChanged);

    /* these elements will only be displayed if the event source is "Param higher/lower" */
    label_createWithLength(&lSrcSetting1, 6, FONT_MEDIUM);
    label_createWithLength(&lSrcSetting2, 10, FONT_MEDIUM);
    dropdown_create(&dSrcParam, compParamList, NULL, FONT_MEDIUM, 88,
            events_SettingChanged);
    entry_create(&eSrcSetting1, NULL, NULL, NULL, FONT_MEDIUM, 4, UNIT_CURRENT,
            events_SettingChanged);
    entry_create(&eSrcSetting2, NULL, NULL, NULL, FONT_MEDIUM, 4, UNIT_CURRENT,
            events_SettingChanged);

    /* ELEMENTS TO ADD/DELETE EFFECTS */
    label_createWithText(&lEffects, "Effects:", FONT_MEDIUM);
    button_create(&bAddEffect, "Add", FONT_MEDIUM, 0, events_AddEffect);
    button_create(&bDeleteEffect, "Delete", FONT_MEDIUM, 0,
            events_DeleteEffect);

    /* ELEMENTS TO EDIT THE SELECTED EFFECT */
    /* these elements will only be displayed if there is an effect */
    dropdown_create(&dEffects, effectDescriptions, &selectedEffectNum,
            FONT_MEDIUM, 124, events_SelectedEffectChanged);
    label_createWithText(&lDest, "Dest:", FONT_MEDIUM);
    dropdown_create(&dDest, destList, NULL, FONT_MEDIUM, 94,
            events_EffectDestChanged);

    label_createWithLength(&lDestSetting1, 6, FONT_MEDIUM);
    label_createWithLength(&lDestSetting2, 10, FONT_MEDIUM);
    dropdown_create(&dDestSetting1, setParamList, NULL, FONT_MEDIUM, 88,
            events_EffectSettingChanged);
    entry_create(&eDestSetting1, NULL, NULL, NULL, FONT_MEDIUM, 4, UNIT_CURRENT,
            events_EffectSettingChanged);
    entry_create(&eDestSetting2, NULL, NULL, NULL, FONT_MEDIUM, 4, UNIT_CURRENT,
            events_EffectSettingChanged);

    events_SetGUIToSelectedEvent();

    container_create(&c, 128, 55);

    /* Attach effect edit widgets */
    container_attach(&c, (widget_t*) &lEvents, 0, 2);
    container_attach(&c, (widget_t*) &bAddEvent, 42, 0);
    container_attach(&c, (widget_t*) &bDeleteEvent, 70, 0);
    container_attach(&c, (widget_t*) &dEvents, 0, 13);
    container_attach(&c, (widget_t*) &lSource, 0, 27);
    container_attach(&c, (widget_t*) &dSource, 24, 25);

    container_attach(&c, (widget_t*) &lSrcSetting1, 0, 39);
    container_attach(&c, (widget_t*) &lSrcSetting2, 0, 51);
    container_attach(&c, (widget_t*) &dSrcParam, 36, 37);
    container_attach(&c, (widget_t*) &eSrcSetting1, 79, 37);
    container_attach(&c, (widget_t*) &eSrcSetting2, 79, 49);

    /* Y coordinate of first effect edit widget */
#define EFFECT_Y_START      61
    container_attach(&c, (widget_t*) &lEffects, 0, EFFECT_Y_START + 2);
    container_attach(&c, (widget_t*) &bAddEffect, 48, EFFECT_Y_START);
    container_attach(&c, (widget_t*) &bDeleteEffect, 76, EFFECT_Y_START);
    container_attach(&c, (widget_t*) &dEffects, 0, EFFECT_Y_START + 13);
    container_attach(&c, (widget_t*) &lDest, 0, EFFECT_Y_START + 27);
    container_attach(&c, (widget_t*) &dDest, 30, EFFECT_Y_START + 25);

    container_attach(&c, (widget_t*) &lDestSetting1, 0, EFFECT_Y_START + 39);
    container_attach(&c, (widget_t*) &lDestSetting2, 0, EFFECT_Y_START + 51);
    container_attach(&c, (widget_t*) &dDestSetting1, 36, EFFECT_Y_START + 37);
    container_attach(&c, (widget_t*) &eDestSetting1, 79, EFFECT_Y_START + 37);
    container_attach(&c, (widget_t*) &eDestSetting2, 79, EFFECT_Y_START + 49);
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
                MESSAGE_OK, (widget_t*) &c);
        return;
    }
    event_t *last = events.firstEvent;
    selectedEventNum = 0;
    if (!last) {
        /* this is the first event */
        events.firstEvent = new;
    } else {
        selectedEventNum++;
        /* find end of event list */
        for (; last->next != NULL; last = last->next)
            selectedEventNum++;
        /* add new event to end of list */
        last->next = new;
    }
    events_UpdateDescription(new);
    events_UpdateDescrPointers();
    events_SelectedEventChanged();
}

void events_DeleteEvent(void) {
    if (!events.firstEvent) {
        /* nothing to delete */
        return;
    }
    if (selectedEventNum == 0) {
        /* remove first item */
        event_t *nextBuffer = events.firstEvent->next;
        events.firstEvent = nextBuffer;
    } else {
        /* find previous event */
        event_t *prev = events.firstEvent;
        event_t *previous = events.firstEvent;
        while (previous->next != selectedEvent) {
            previous = previous->next;
        }
        if (!previous) {
            /* couldn't find previous */
            return;
        }
        /* remove deletor from event list */
        previous->next = selectedEvent->next;
        selectedEventNum--;
    }
    /* free effects of event */
    eventEffect_t *ef = selectedEvent->firstEffect;
    while (ef) {
        eventEffect_t *next = ef->next;
        event_freeEffect(ef);
        ef = next;
    }
    /* free deleted event */
    event_freeEvent(selectedEvent);
    events_UpdateDescrPointers();
    events_SelectedEventChanged();
}

void events_SelectedEventChanged(void) {
    /* update the selected event pointer */
    selectedEvent = events.firstEvent;
    uint8_t i = selectedEventNum;
    while (i && selectedEvent) {
        selectedEvent = selectedEvent->next;
        i--;
    }
    /* selected the first effect of new event */
    selectedEffectNum = 0;
    events_SelectedEffectChanged();
    /* load effect descriptions for this event */
    events_UpdateEffectDescrPointers();
    events_SetGUIToSelectedEvent();
}

void events_SourceChanged(void) {
    if (!selectedEvent) {
        /* error */
        return;
    }
    /* initialize values */
    switch (selectedEvent->srcType) {
    case EV_SRC_PARAM_HIGHER:
    case EV_SRC_PARAM_LOWER:
        selectedEvent->srcParamNum = 0;
        selectedEvent->srcLimit = 0;
        selectedEvent->srcParam =
                eventCompParamPointers[selectedEvent->srcParamNum];
        break;
    case EV_SRC_TIM_ZERO:
        selectedEvent->srcTimerNum = 0;
        break;
    case EV_SRC_WAVEFORM_PHASE:
        selectedEvent->phase = 0;
        break;
    }
    events_UpdateDescription(selectedEvent);
    events_SetGUIToSelectedEvent();
}

void events_UpdateDescription(event_t *ev) {
    /* update description for this event */
    switch (ev->srcType) {
    case EV_SRC_PARAM_LOWER:
    case EV_SRC_PARAM_HIGHER:
        strcpy(ev->descr, eventCompParamNames[ev->srcParamNum]);
        char arrow = '<';
        if (ev->srcType == EV_SRC_PARAM_HIGHER) {
            arrow = '>';
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
                unitNames[UNIT_DEGREE][0], unitNames[UNIT_DEGREE][1],
                unitNames[UNIT_DEGREE][2]);
        break;
    }
}

void events_UpdateDescrPointers(void) {
    event_t *ev;
    uint8_t i = 0;
    for (ev = events.firstEvent; ev != NULL; ev = ev->next) {
        eventDescriptions[i++] = ev->descr;
    }
    eventDescriptions[i] = 0;
}

void events_SetGUIToSelectedEvent(void) {
    if (!selectedEvent) {
        /* set elements invisible */
        widget_SetInvisible((widget_t*) &dEvents);
        widget_SetInvisible((widget_t*) &dSource);
        widget_SetInvisible((widget_t*) &lSource);
        widget_SetInvisible((widget_t*) &lSrcSetting1);
        widget_SetInvisible((widget_t*) &eSrcSetting1);
        widget_SetInvisible((widget_t*) &dSrcParam);
        widget_SetInvisible((widget_t*) &lSrcSetting2);
        widget_SetInvisible((widget_t*) &eSrcSetting2);

        /* Effect elements */
        widget_SetInvisible((widget_t*) &lEffects);
        widget_SetInvisible((widget_t*) &bAddEffect);
        widget_SetInvisible((widget_t*) &bDeleteEffect);
    } else {
        /* base elements visible */
        widget_SetVisible((widget_t*) &dEvents);
        widget_SetVisible((widget_t*) &dSource);
        widget_SetVisible((widget_t*) &lSource);
        widget_SetVisible((widget_t*) &lEffects);
        widget_SetVisible((widget_t*) &bAddEffect);
        widget_SetVisible((widget_t*) &bDeleteEffect);
        dSource.value = &selectedEvent->srcType;
        events_SetGUIToSelectedSource(selectedEvent);
    }
    events_SetGUIToSelectedEffect();
}

void events_SetGUIToSelectedSource(event_t *ev) {
    widget_SetInvisible((widget_t*) &lSrcSetting1);
    widget_SetInvisible((widget_t*) &eSrcSetting1);
    widget_SetInvisible((widget_t*) &dSrcParam);
    widget_SetInvisible((widget_t*) &lSrcSetting2);
    widget_SetInvisible((widget_t*) &eSrcSetting2);
    switch (ev->srcType) {
    case EV_SRC_PARAM_HIGHER:
    case EV_SRC_PARAM_LOWER:
        widget_SetVisible((widget_t*) &lSrcSetting1);
        label_SetText(&lSrcSetting1, "Param:");
        widget_SetVisible((widget_t*) &dSrcParam);
        dSrcParam.value = &ev->srcParamNum;
        widget_SetVisible((widget_t*) &lSrcSetting2);
        label_SetText(&lSrcSetting2, "Threshold:");
        widget_SetVisible((widget_t*) &eSrcSetting2);
        eSrcSetting2.value = &ev->srcLimit;
        eSrcSetting2.unit = eventCompParamUnits[ev->srcParamNum];
        break;
    case EV_SRC_TRIG_FALL:
    case EV_SRC_TRIG_RISE:
        break;
    case EV_SRC_TIM_ZERO:
        widget_SetVisible((widget_t*) &lSrcSetting1);
        label_SetText(&lSrcSetting1, "Timer:");
        widget_SetVisible((widget_t*) &eSrcSetting1);
        eSrcSetting1.value = &ev->srcTimerNum;
        eSrcSetting1.unit = UNIT_ITERATOR;
        eSrcSetting1.min = &null;
        eSrcSetting1.max = &maxTimers;
        break;
    case EV_SRC_WAVEFORM_PHASE:
        widget_SetVisible((widget_t*) &lSrcSetting1);
        label_SetText(&lSrcSetting1, "Phase:");
        widget_SetVisible((widget_t*) &eSrcSetting1);
        eSrcSetting1.value = &ev->phase;
        eSrcSetting1.unit = UNIT_DEGREE;
        eSrcSetting1.min = &null;
        eSrcSetting1.max = &maxDegree;
        break;
    }
}

void events_SettingChanged(void) {
    if (selectedEvent->srcType == EV_SRC_PARAM_HIGHER
            || selectedEvent->srcType == EV_SRC_PARAM_LOWER) {
        /* update parameter pointer */
        selectedEvent->srcParam =
                eventCompParamPointers[selectedEvent->srcParamNum];
    }
    events_UpdateDescription(selectedEvent);
    events_SetGUIToSelectedSource(selectedEvent);
}

void events_AddEffect(void) {
    /* try to allocate new effect */
    eventEffect_t *new = event_allocateEffect();
    if (!new) {
        /* failed to allocate effect */
        message_Box("Already at\nmaximum number\nof effects", 3, 14,
                FONT_MEDIUM, MESSAGE_OK, (widget_t*) &c);
        return;
    }
    eventEffect_t *last = selectedEvent->firstEffect;
    selectedEffectNum = 0;
    if (!last) {
        /* this is the first effect */
        selectedEvent->firstEffect = new;
    } else {
        selectedEffectNum++;
        /* find end of effect list */
        for (; last->next != NULL; last = last->next)
            selectedEffectNum++;
        /* add new event to end of list */
        last->next = new;
    }
    events_UpdateEffectDescription(new);
    events_UpdateEffectDescrPointers();
    events_SelectedEffectChanged();
}

void events_DeleteEffect(void) {
    if (!selectedEvent->firstEffect) {
        /* nothing to delete */
        return;
    }
    if (selectedEffectNum == 0) {
        /* remove first item */
        eventEffect_t *nextBuffer = selectedEvent->firstEffect->next;
        event_freeEffect(selectedEvent->firstEffect);
        selectedEvent->firstEffect = nextBuffer;
    } else {
        /* find selected effect */
        eventEffect_t *deletor = selectedEvent->firstEffect;
        uint8_t i = selectedEffectNum;
        eventEffect_t *previous = NULL;
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
        /* remove deletor from effect list */
        previous->next = deletor->next;
        event_freeEffect(deletor);
        selectedEffectNum--;
    }
    events_UpdateEffectDescrPointers();
    events_SelectedEffectChanged();
}

void events_SelectedEffectChanged(void) {
    /* update selected event */
    if (!selectedEvent) {
        /* no event selected -> no effect available */
        selectedEffect = NULL;
    } else {
        /* update the selected effect pointer */
        selectedEffect = selectedEvent->firstEffect;
        uint8_t i = selectedEffectNum;
        while (i && selectedEffect) {
            selectedEffect = selectedEffect->next;
            i--;
        }
    }
    events_SetGUIToSelectedEffect();
}

void events_EffectDestChanged(void) {
    if (!selectedEffect) {
        /* error */
        return;
    }
    /* initialize values */
    switch (selectedEffect->destType) {
    case EV_DEST_LOAD_MODE:
        selectedEffect->destMode = FUNCTION_CC;
        break;
    case EV_DEST_LOAD_OFF:
    case EV_DEST_LOAD_ON:
    case EV_DEST_TRIG_HIGH:
    case EV_DEST_TRIG_LOW:
        /* nothing to do */
        break;
    case EV_DEST_SET_PARAM:
        selectedEffect->destParamNum = 0;
        selectedEffect->destParam =
                eventSetParamPointers[selectedEffect->destParamNum];
        break;
    case EV_DEST_SET_TIMER:
        selectedEffect->destTimerNum = 0;
        selectedEffect->destTimerValue = 0;
        break;
    }
    events_UpdateEffectDescription(selectedEffect);
    events_SetGUIToSelectedEffect();
}

void events_UpdateEffectDescription(eventEffect_t *ef) {
    switch (ef->destType) {
    case EV_DEST_LOAD_MODE:
        strcpy(ef->descr, "Set mode to ");
        strcpy(&ef->descr[12], loadModeNames[ef->destMode]);
        break;
    case EV_DEST_LOAD_OFF:
        strcpy(ef->descr, "Turn load off");
        break;
    case EV_DEST_LOAD_ON:
        strcpy(ef->descr, "Turn load on");
        break;
    case EV_DEST_SET_PARAM:
        strncpy(ef->descr, eventSetParamNames[ef->destParamNum], 14);
        uint8_t pos = strlen(ef->descr);
        ef->descr[pos] = '=';
        string_fromUintUnits(ef->destSetValue, &ef->descr[pos + 1], 4,
                unitNames[eventSetParamUnits[ef->destParamNum]][0],
                unitNames[eventSetParamUnits[ef->destParamNum]][1],
                unitNames[eventSetParamUnits[ef->destParamNum]][2]);
        break;
    case EV_DEST_SET_TIMER:
        strcpy(ef->descr, "Timer  ");
        if (ef->destTimerNum > 9) {
            ef->descr[6] = (ef->destTimerNum / 10) + '0';
        }
        ef->descr[7] = (ef->destTimerNum % 10) + '0';
        ef->descr[8] = '=';
        string_fromUintUnits(ef->destTimerValue, &ef->descr[9], 4,
                unitNames[UNIT_TIME][0], unitNames[UNIT_TIME][1],
                unitNames[UNIT_TIME][2]);
        break;
    case EV_DEST_TRIG_HIGH:
        strcpy(ef->descr, "Trigger high");
        break;
    case EV_DEST_TRIG_LOW:
        strcpy(ef->descr, "Trigger low");
        break;
    }
}

void events_UpdateEffectDescrPointers(void) {
    if(!selectedEvent) {
        /* no event available -> no effects available */
        return;
    }
    eventEffect_t *ef;
    uint8_t i = 0;
    for (ef = selectedEvent->firstEffect; ef != NULL; ef = ef->next) {
        effectDescriptions[i++] = ef->descr;
    }
    effectDescriptions[i] = 0;
}

void events_SetGUIToSelectedEffect(void) {
    if (!selectedEffect) {
        /* set elements invisible */
        widget_SetInvisible((widget_t*) &dEffects);
        widget_SetInvisible((widget_t*) &lDest);
        widget_SetInvisible((widget_t*) &dDest);
        widget_SetInvisible((widget_t*) &lDestSetting1);
        widget_SetInvisible((widget_t*) &dDestSetting1);
        widget_SetInvisible((widget_t*) &eDestSetting1);
        widget_SetInvisible((widget_t*) &lDestSetting2);
        widget_SetInvisible((widget_t*) &eDestSetting2);
    } else {
        /* base elements visible */
        widget_SetVisible((widget_t*) &dEffects);
        widget_SetVisible((widget_t*) &lDest);
        widget_SetVisible((widget_t*) &dDest);
        dDest.value = &selectedEffect->destType;
        events_SetGUIToSelectedDest(selectedEffect);
    }
}

void events_SetGUIToSelectedDest(eventEffect_t *ef) {
    widget_SetInvisible((widget_t*) &lDestSetting1);
    widget_SetInvisible((widget_t*) &dDestSetting1);
    widget_SetInvisible((widget_t*) &eDestSetting1);
    widget_SetInvisible((widget_t*) &lDestSetting2);
    widget_SetInvisible((widget_t*) &eDestSetting2);
    switch (ef->destType) {
    case EV_DEST_LOAD_MODE:
        widget_SetVisible((widget_t*) &lDestSetting1);
        label_SetText(&lDestSetting1, "Mode:");
        widget_SetVisible((widget_t*) &dDestSetting1);
        dDestSetting1.value = &ef->destMode;
        dDestSetting1.itemlist = loadModeList;
        break;
    case EV_DEST_LOAD_OFF:
    case EV_DEST_LOAD_ON:
        /* nothing to do */
        break;
    case EV_DEST_SET_PARAM:
        widget_SetVisible((widget_t*) &lDestSetting1);
        label_SetText(&lDestSetting1, "Param:");
        widget_SetVisible((widget_t*) &dDestSetting1);
        dDestSetting1.value = &ef->destParamNum;
        dDestSetting1.itemlist = setParamList;
        widget_SetVisible((widget_t*) &lDestSetting2);
        label_SetText(&lDestSetting2, "Value:");
        widget_SetVisible((widget_t*) &eDestSetting2);
        eDestSetting2.value = &ef->destSetValue;
        eDestSetting2.unit = eventSetParamUnits[ef->destParamNum];
        eDestSetting1.min = &null;
        eDestSetting1.max = NULL;
        break;
    case EV_DEST_SET_TIMER:
        widget_SetVisible((widget_t*) &lDestSetting1);
        label_SetText(&lDestSetting1, "Timer:");
        widget_SetVisible((widget_t*) &eDestSetting1);
        eDestSetting1.value = &ef->destTimerNum;
        eDestSetting1.unit = UNIT_ITERATOR;
        eDestSetting1.min = &null;
        eDestSetting1.max = &maxTimers;
        widget_SetVisible((widget_t*) &lDestSetting2);
        label_SetText(&lDestSetting2, "Value:");
        widget_SetVisible((widget_t*) &eDestSetting2);
        eDestSetting2.value = &ef->destTimerValue;
        eDestSetting2.unit = UNIT_TIME;
        eDestSetting2.min = &null;
        eDestSetting2.max = &maxTimerValue;
        break;
    case EV_DEST_TRIG_HIGH:
    case EV_DEST_TRIG_LOW:
        /* nothing to do */
        break;
    }
}

void events_EffectSettingChanged(void) {
    if (selectedEffect->destType == EV_DEST_SET_PARAM) {
        /* update parameter pointer */
        selectedEffect->destParam =
                eventSetParamPointers[selectedEffect->destParamNum];
    }
    events_UpdateEffectDescription(selectedEffect);
    events_SetGUIToSelectedDest(selectedEffect);
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
            memset(&effectList[i], 0, sizeof(eventEffect_t));
            effectList[i].inUse = 1;
            return &effectList[i];
        }
    }
    return NULL;
}
