/**
 * \file
 * \brief   Timer source file.
 *
 *          This file contains all the timer stuff.
 */
#include "timer.h"

void timer_Init(void) {

    timer.ms = 0;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

    TIM_TimeBaseInitTypeDef timebase;
    timebase.TIM_ClockDivision = TIM_CKD_DIV1;
    timebase.TIM_CounterMode = TIM_CounterMode_Up;
    timebase.TIM_RepetitionCounter = 0;
    // set timer tick to 1us
    timebase.TIM_Prescaler = 71;
    // set timer period to 1ms
    timebase.TIM_Period = 999;

    // start timer
    TIM_TimeBaseInit(TIM1, &timebase);
    TIM_Cmd(TIM1, ENABLE);
    TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);

    // configure interrupt
    NVIC_InitTypeDef nvic;
    nvic.NVIC_IRQChannel = TIM1_UP_IRQn;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    nvic.NVIC_IRQChannelSubPriority = 0;
    nvic.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_Init(&nvic);

}

/**
 * \brief Waits for a specific amount of milliseconds
 *
 * Actual waittime could be up to one millisecond less.
 *
 * \param ms waittime in milliseconds
 */
void timer_waitms(uint16_t ms) {
    uint32_t time = timer.ms;
    while (timer.ms < time + ms)
        ;
}

/**
 * \brief Waits for a specific amount of microseconds
 *
 * Actual waittime might be slightly longer due to interfering
 * interrupts.
 *
 * \param us waittime in mikroseconds (maximum waittime is 999us)
 */
void timer_waitus(uint16_t us) {
    uint16_t timeStart = TIM1->CNT;
    uint32_t timems = timer.ms;
    uint16_t timeEnd = timeStart + us;
    // can't just simply wait for timer to match timeEnd
    // because it might be missed during an interrupt
    if (timeEnd < 1000) {
        // timer doesn't overflow before endtime
        // -> wait for timer to reach endtime.
        // Also abort if timer.ms changes (->missed endtime)
        while (TIM1->CNT < timeEnd && timems == timer.ms)
            ;
    } else {
        // timer overflows before endtime
        timeEnd -= 1000;
        // wait for timer to overflow, then wait for timer
        // to reach endtime.
        // Also abort if timer overflows a second time
        // (->missed endtime)
        while ((timems == timer.ms || TIM1->CNT < timeEnd)
                && timems + 1 >= timer.ms)
            ;
    }
}

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
        void (*callback)(), uint8_t priority) {
    if (timerNumber < 2 || timerNumber > 4)
        return 1;
    if (priority > 15)
        return 1;
    TIM_TypeDef *tim;
    uint8_t irq;
    uint32_t clock;
    switch (timerNumber) {
    case 2:
        tim = TIM2;
        irq = TIM2_IRQn;
        clock = RCC_APB1Periph_TIM2;
        break;
    case 3:
        tim = TIM3;
        irq = TIM3_IRQn;
        clock = RCC_APB1Periph_TIM3;
        break;
    case 4:
        tim = TIM4;
        irq = TIM4_IRQn;
        clock = RCC_APB1Periph_TIM4;
        break;
    }

    // set callback function
    timer.callbacks[timerNumber - 2] = callback;

    // enable timer clock
    RCC_APB1PeriphClockCmd(clock, ENABLE);
    TIM_TimeBaseInitTypeDef timebase;
    timebase.TIM_ClockDivision = TIM_CKD_DIV1;
    timebase.TIM_CounterMode = TIM_CounterMode_Up;
    timebase.TIM_RepetitionCounter = 0;
    // calculate timer period
    timebase.TIM_Prescaler = (period / 65536) + 1;
    timebase.TIM_Period = (period / timebase.TIM_Prescaler) - 1;

    // start timer
    TIM_TimeBaseInit(tim, &timebase);
    TIM_Cmd(tim, ENABLE);
    TIM_ITConfig(tim, TIM_IT_Update, ENABLE);

    // configure interrupt
    NVIC_InitTypeDef nvic;
    nvic.NVIC_IRQChannel = irq;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    nvic.NVIC_IRQChannelSubPriority = 0;
    nvic.NVIC_IRQChannelPreemptionPriority = priority;
    NVIC_Init(&nvic);

    return 0;
}

void TIM1_UP_IRQHandler(void) {
    if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET) {
        timer.ms++;
    }
}

void TIM2_IRQHandler(void) {
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET) {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        timer.callbacks[0]();
    }
}

void TIM3_IRQHandler(void) {
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET) {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        timer.callbacks[1]();
    }
}

void TIM4_IRQHandler(void) {
    if (TIM_GetITStatus(TIM4, TIM_IT_Update) == SET) {
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
        timer.callbacks[2]();
    }
}
