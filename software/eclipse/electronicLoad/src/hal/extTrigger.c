#include "extTrigger.h"

void hal_triggerInit(void) {
	trigger.callback = NULL;

	GPIO_InitTypeDef gpio;

	// initialize display GPIO
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	// external trigger out
	gpio.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOA, &gpio);
	hal_setTriggerOut(0);
	// external trigger in
	gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	gpio.GPIO_Pin = GPIO_Pin_4;
	GPIO_Init(GPIOA, &gpio);

	// setup external interrupt
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource4);
	EXTI_InitTypeDef exti;
	exti.EXTI_Line = EXTI_Line4;
	exti.EXTI_LineCmd = ENABLE;
	exti.EXTI_Mode = EXTI_Mode_Interrupt;
	exti.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_Init(&exti);

	// setup nvic controller
	NVIC_InitTypeDef nvic;
	nvic.NVIC_IRQChannel = EXTI4_IRQn;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	nvic.NVIC_IRQChannelPreemptionPriority = 1;
	nvic.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&nvic);
}

void hal_setTriggerInCallback(void (*callback)(trigEdge_t edge)) {
	trigger.callback = callback;
}

void hal_setTriggerOut(uint8_t state) {
	if (state) {
		GPIO_WriteBit(GPIOA, GPIO_Pin_6, Bit_SET);
	} else {
		GPIO_WriteBit(GPIOA, GPIO_Pin_6, Bit_RESET);
	}
}

void EXTI4_IRQHandler(void) {
	if (EXTI_GetITStatus(EXTI_Line4) == SET) {
		EXTI_ClearITPendingBit(EXTI_Line4);
		if (trigger.callback) {
			if (GPIOA->IDR & GPIO_Pin_4)
				trigger.callback(TRIG_RISING);
			else
				trigger.callback(TRIG_FALLING);
		}
	}
}
