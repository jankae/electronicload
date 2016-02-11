#include "events.h"

void events_Init(void) {
    uint8_t i;
    for (i = 0; i < EV_MAXEVENTS; i++) {
        events.evlist[i].sourceType = EV_SRC_DISABLED;
        events.evlist[i].srcParam = NULL;
        events.evlist[i].srcLimit = 0;
        events.evlist[i].destMode = FUNCTION_CC;
        events.evlist[i].destParam = NULL;
        events.evlist[i].destSetValue = 0;
        events.evlist[i].destTimerNum = 0;
        events.evlist[i].destTimerValue = 0;
        events.evlist[i].destType = EV_DEST_LOAD_OFF;
    }
}

void events_HandleEvents(void) {
    uint8_t i;
    for (i = 0; i < EV_MAXEVENTS; i++) {
        if (events_isEventSourceTriggered(i)) {
            events_triggerEventDestination(i);
        }
    }
}

uint8_t events_isEventSourceTriggered(uint8_t ev) {
    uint8_t triggered = 0;
    if (events.evlist[ev].sourceType == EV_SRC_TIM_ZERO) {
        if (events.evTimers[events.evlist[ev].destTimerNum] == 0)
            triggered = 1;
    }
    if (events.evlist[ev].sourceType == EV_SRC_PARAM_HIGHER) {
        if (*(events.evlist[ev].srcParam) > events.evlist[ev].srcLimit)
            triggered = 1;
    }
    if (events.evlist[ev].sourceType == EV_SRC_PARAM_LOWER) {
        if (*(events.evlist[ev].srcParam) < events.evlist[ev].srcLimit)
            triggered = 1;
    }
    if (events.evlist[ev].sourceType == EV_SRC_TRIG_FALL) {
        if (events.triggerInState == -1)
            triggered = 1;
    }
    if (events.evlist[ev].sourceType == EV_SRC_TRIG_RISE) {
        if (events.triggerInState == 1)
            triggered = 1;
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

void events_getDescr(uint8_t ev, char* descr) {
    if (events.evlist[ev].sourceType == EV_SRC_DISABLED) {
        string_copy(descr, "DISABLED");
        return;
    }
    // construct source description
    switch (events.evlist[ev].sourceType) {
    case EV_SRC_PARAM_HIGHER:
        // TODO add partial parameter name
        string_copy(descr, "P>:      ");
        break;
    case EV_SRC_PARAM_LOWER:
        // TODO add partial parameter name
        string_copy(descr, "P<:      ");
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
        string_copy(&descr[10], "OutputOff");
        break;
    case EV_DEST_LOAD_ON:
        string_copy(&descr[10], "OutputOn");
        break;
    case EV_DEST_SET_PARAM:
        // TODO add partial parameter name
        string_copy(&descr[10], "P=:       ");
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
    for (i = 0; i < EV_MAXEVENTS; i++) {
        events_getDescr(i, eventDescr[i]);
        descrList[i] = eventDescr[i];
    }
    int8_t ev;
    do {
        ev = menu_ItemChooseDialog(
                "\xCD\xCD\xCD\xCD\xCD" "EVENT LIST\xCD\xCD\xCD\xCD\xCD\xCD",
                descrList,
                EV_MAXEVENTS);
    } while (ev >= 0);
}
