#include "events.h"

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
        if (0/* TODO ext trigger fall detection */)
            triggered = 1;
    }
    if (events.evlist[ev].sourceType == EV_SRC_TRIG_RISE) {
        if (0/* TODO ext trigger rise detection */)
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
