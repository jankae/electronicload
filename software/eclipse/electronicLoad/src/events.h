#ifndef EVENTS_H_
#define EVENTS_H_

#include <stdint.h>
#include "loadFunctions.h"
#include "extTrigger.h"

#define EV_MAXEVENTS        5
#define EV_MAXTIMERS        5

#define EV_TIMER_STOPPED    0xffffffff

typedef enum {
    EV_SRC_TRIG_RISE,
    EV_SRC_TRIG_FALL,
    EV_SRC_PARAM_LOWER,
    EV_SRC_PARAM_HIGHER,
    EV_SRC_TIM_ZERO,
    EV_SRC_DISABLED
} evSourceType_t;
typedef enum {
    EV_DEST_TRIG_HIGH,
    EV_DEST_TRIG_LOW,
    EV_DEST_SET_PARAM,
    EV_DEST_SET_TIMER,
    EV_DEST_LOAD_MODE
} evDestType_t;

struct event {
    /******************************
     * event source parameters
     *****************************/
    evSourceType_t sourceType;
    // variables for param lower/higher
    uint32_t *srcParam;
    uint32_t srcLimit;
    /******************************
     * event destination parameters
     *****************************/
    evDestType_t destType;
    // variables for set param
    uint32_t *destParam;
    uint32_t destSetValue;
    // variables for set timer
    uint8_t destTimerNum;
    uint32_t destTimerValue;
    // variable for load mode
    loadMode_t destMode;
};

struct {
    uint32_t evTimers[EV_MAXTIMERS];
    struct event evlist[EV_MAXEVENTS];
} events;

/**
 * \brief Executes the destination action for every triggered event
 *
 * Should be called each millisecond
 */
void events_HandleEvents(void);

/**
 * \brief Checks whether an event has been triggered
 *
 * \param ev    Number of event
 * \return      0: event not triggered, 1: event triggered
 */
uint8_t events_isEventSourceTriggered(uint8_t ev);

/**
 * \brief Executes event destination action
 *
 * \param ev    Number of event
 */
void events_triggerEventDestination(uint8_t ev);

/**
 * \brief Decrements running event timers by 1 millisecond
 *
 * Should be called each millisecond
 */
void events_decrementTimers(void);

#endif