#include "timer.h"

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
