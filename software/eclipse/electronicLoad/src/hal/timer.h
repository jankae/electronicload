#ifndef TIMER_H_
#define TIMER_H_

#include "stm32f10x.h"
#include "stm32f10x_conf.h"

#define MS_TO_TICKS(ms) (72000*ms)

struct {
	void ((*callbacks[3])());
} timer;

uint8_t timer_SetupPeriodicFunction(uint8_t timerNumber, uint32_t period,
		void (*callback)(), uint8_t priority);

void TIM2_IRQHandler(void);

#endif
