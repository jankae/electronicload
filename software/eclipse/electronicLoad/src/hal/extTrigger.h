
#ifndef HAL_EXTTRIGGER_H_
#define HAL_EXTTRIGGER_H_

#include <stdlib.h>
#include "stm32f10x.h"

typedef enum {TRIG_RISING, TRIG_FALLING} trigEdge_t;

struct {
	void (*callback)(trigEdge_t edge);
} trigger;

void hal_triggerInit(void);

void hal_setTriggerInCallback(void (*callback)(trigEdge_t edge));

void hal_setTriggerOut(uint8_t state);

void EXTI4_IRQHandler(void);

#endif
