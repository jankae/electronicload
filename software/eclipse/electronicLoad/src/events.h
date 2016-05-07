#ifndef EVENTS_H_
#define EVENTS_H_

#include <stdint.h>
#include "loadFunctions.h"
#include "extTrigger.h"
#include "menu.h"
#include "uart.h"

#define EV_MAXEVENTS        10
#define EV_MAXTIMERS        5

#define EV_NUM_SOURCETYPES  7
#define EV_NUM_DESTTYPES    7
#define EV_NUM_COMPPARAMS   7
#define EV_NUM_SETPARAMS    4

#define EV_TIMER_STOPPED    0xffffffff

typedef enum {
    EV_SRC_DISABLED = 0,
    EV_SRC_TRIG_RISE = 1,
    EV_SRC_TRIG_FALL = 2,
    EV_SRC_PARAM_LOWER = 3,
    EV_SRC_PARAM_HIGHER = 4,
    EV_SRC_TIM_ZERO = 5,
    EV_SRC_WAVEFORM_PHASE = 6
} evSourceType_t;
typedef enum {
    EV_DEST_TRIG_HIGH = 0,
    EV_DEST_TRIG_LOW = 1,
    EV_DEST_SET_PARAM = 2,
    EV_DEST_SET_TIMER = 3,
    EV_DEST_LOAD_MODE = 4,
    EV_DEST_LOAD_ON = 5,
    EV_DEST_LOAD_OFF = 6
} evDestType_t;

struct event {
    /******************************
     * event source parameters
     *****************************/
    evSourceType_t srcType;
    // variables for param lower/higher
    uint32_t *srcParam;
    uint8_t srcParamNum;
    uint32_t srcLimit;
    uint8_t srcTimerNum;
    /******************************
     * event destination parameters
     *****************************/
    evDestType_t destType;
    // variables for set param
    uint32_t *destParam;
    uint8_t destParamNum;
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
    // 0: no change, 1: rising edge, -1: falling edge
    int8_t triggerInState;
    /******************************
     * waveform phase paramters
     *****************************/
    uint32_t waveformOldPhase;
    uint32_t waveformPhase;
} events;

/**
 * \brief Initializes event structs (disables all events)
 */
void events_Init(void);

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

void events_updateWaveformPhase(void);

/**
 * \brief Constructs a short description of an event
 *
 * \param ev        Number of the event that will be described
 * \param *descr    Pointer to the char array (at least of size 21) that will contain the description
 */
void events_getDescr(uint8_t ev, char *descr);

/**
 * \brief Display the 'event list' menu
 */
void events_menu(void);

/**
 * \brief Displays a menu which enables the user to edit an event entry
 *
 * \param ev    Number of the event that will be edited
 */
void events_editEventMenu(uint8_t ev);

#endif
