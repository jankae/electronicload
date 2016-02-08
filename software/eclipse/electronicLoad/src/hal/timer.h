/**
 * \file
 * \brief   Timer header file.
 *
 *          This file contains all the timer stuff.
 */
#ifndef TIMER_H_
#define TIMER_H_

#include "stm32f10x.h"
#include "stm32f10x_conf.h"

#define MS_TO_TICKS(ms) (72000*ms)

struct {
    void ((*callbacks[3])());
    volatile uint32_t ms;
} timer;

/**
 * \brief Initializes the system clock
 *
 * Configures timer 1 to overflow every ms
 */
void timer_Init(void);

/**
 * \brief Waits for a specific amount of milliseconds
 *
 * Actual waittime could be up to one millisecond less.
 *
 * \param ms waittime in milliseconds
 */
void timer_waitms(uint16_t ms);

/**
 * \brief Waits for a specific amount of microseconds
 *
 * Actual waittime might be slightly longer due to interfering
 * interrupts.
 *
 * \param us waittime in mikroseconds (maximum waittime is 999us)
 */
void timer_waitus(uint16_t us);

/**
 * \brief Returns a timeout time
 *
 * \param ms Time in milliseconds until timeout
 */
uint32_t timer_SetTimeout(uint32_t ms);

/**
 * \brief Checks whether a timeout has elapsed
 *
 * \param timeout The systemtime at which the timeout elapses
 * \return 1: timeout elapsed, 0: timeout still running
 */
uint8_t timer_TimeoutElapsed(uint32_t timeout);

/**
 * \brief Sets up a regularly called function
 *
 * Registers a callback function which will be called periodically
 * in an interrupt after a certain amount of systicks
 *
 * \param timerNumber   Number of the used timer (2-4). Don't use the
 *                      same timer twice!
 * \param period        Number of systicks between function calls
 * \param callback      Pointer to function that will be called
 * \param priority      Priority of interrupt from which the function
 *                      will be called
 */
uint8_t timer_SetupPeriodicFunction(uint8_t timerNumber, uint32_t period,
        void (*callback)(), uint8_t priority);

void TIM1_UP_IRQHandler(void);

void TIM2_IRQHandler(void);

void TIM3_IRQHandler(void);

void TIM4_IRQHandler(void);

#endif
