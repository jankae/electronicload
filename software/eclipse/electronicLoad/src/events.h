#ifndef EVENTS_H_
#define EVENTS_H_

#include <stdint.h>
#include "loadFunctions.h"
#include "extTrigger.h"
#include "uart.h"

#include "GUI/gui.h"

#define EV_MAXEVENTS        10
#define EV_MAXEFFECTS       5
#define EV_MAXTIMERS        5

#define EV_MAXDESCRLENGTH   19

#define EV_NUM_SOURCETYPES  6
#define EV_NUM_DESTTYPES    7
#define EV_NUM_COMPPARAMS   7
#define EV_NUM_SETPARAMS    4

#define EV_TIMER_STOPPED    0xffffffff

typedef enum {
    EV_SRC_TRIG_RISE = 0,
    EV_SRC_TRIG_FALL = 1,
    EV_SRC_PARAM_LOWER = 2,
    EV_SRC_PARAM_HIGHER = 3,
    EV_SRC_TIM_ZERO = 4,
    EV_SRC_WAVEFORM_PHASE = 5
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

typedef struct eventEffect eventEffect_t;
typedef struct event event_t;

struct eventEffect {
    eventEffect_t *next;

    evDestType_t destType;
    union {
        /* change parameter settings */
        struct {
            uint32_t *destParam;
            uint8_t destParamNum;
            uint32_t destSetValue;
        };
        /* set timer settings */
        struct {
            uint8_t destTimerNum;
            uint32_t destTimerValue;
        };
        /* change load mode settings */
        loadMode_t destMode;
    };
    struct {
        uint8_t inUse :1;
    };
};

struct event {
    event_t *next;
    eventEffect_t *firstEffect;

    evSourceType_t srcType;
    union {
        /* Parameter higher/lower settings */
        struct {
            uint32_t *srcParam;
            uint8_t srcParamNum;
            uint32_t srcLimit;
        };
        /* Timer elapsed settings */
        uint32_t srcTimerNum;
        /* waveform phase */
        uint32_t phase;
    };
    struct {
        uint8_t inUse :1;
        uint8_t triggered :1;
    };
    char descr[EV_MAXDESCRLENGTH];
};

struct {
    event_t *firstEvent;

    uint32_t evTimers[EV_MAXTIMERS];
    // 0: no change, 1: rising edge, -1: falling edge
    int8_t triggerInState;
    int8_t triggerOutState;
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

widget_t* events_getWidget(void);

void events_AddEvent(void);

void events_DeleteEvent(void);

void events_SourceChanged(void);

void events_UpdateDescriptions(void);

void events_SetGUIToSelectedEvent(void);

/**
 * \brief Executes the destination action for every triggered event
 *
 * Should be called each millisecond
 */
void events_HandleEvents(void);

/**
 * \brief Checks whether an event has been triggered
 *
 * \param ev    Pointer to event
 */
void events_UpdateTrigger(event_t *ev);

/**
 * \brief Executes event destination action
 *
 * \param ev    Pointer to event
 */
void events_triggerEventDestination(event_t *ev);

/**
 * \brief Decrements running event timers by 1 millisecond
 *
 * Should be called each millisecond
 */
void events_decrementTimers(void);

void events_updateWaveformPhase(void);

event_t* event_allocateEvent(void);

eventEffect_t* event_allocateEffect(void);

inline void event_freeEvent(event_t *ev){
    ev->inUse = 0;
}

inline void event_freeEffect(eventEffect_t *ef){
    ef->inUse = 0;
}

/**
 * \brief Constructs a short description of an event
 *
 * \param ev        Event that will be described
 * \param *descr    Pointer to the char array (at least of size 21) that will contain the description
 */
void events_getSrcDescr(struct event ev, char *descr);

/**
 * \brief Constructs a short description of an effect
 *
 * \param ef        Effect that will be described
 * \param *descr    Pointer to the char array (at least of size 21) that will contain the description
 */
void events_getEffectDescr(struct effect ef, char *descr);

/**
 * \brief Display the 'event list' menu
 */
void events_menu(void);

/**
 * \brief Displays a menu which enables the user to edit an event entry
 *
 * \param ev    Event that will be edited
 */
void events_editEventMenu(struct event *ev);

void events_effectMenu(struct event *ev);

void events_editEffectMenu(struct effect *ef);

#endif
