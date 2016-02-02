#include "loadFunctions.h"

void load_Init(void) {
	loadFunctions.type = FUNCTION_CC;
	loadFunctions.current = 0;
	loadFunctions.voltage = LOAD_MAXVOLTAGE;
	loadFunctions.resistance =LOAD_MAXRESISTANCE;
	loadFunctions.power = 0;
    timer_SetupPeriodicFunction(2, MS_TO_TICKS(1), load_update, 4);
}

void load_set_CC(uint32_t c) {
	loadFunctions.type = FUNCTION_CC;
	loadFunctions.current = c;
}

void load_set_CV(uint32_t v) {
	loadFunctions.type = FUNCTION_CV;
	loadFunctions.voltage = v;
}

void load_set_CR(uint32_t r) {
	loadFunctions.type = FUNCTION_CR;
	loadFunctions.resistance = r;
}

void load_set_CP(uint32_t p) {
	loadFunctions.type = FUNCTION_CP;
	loadFunctions.power = p;
}

void load_update(void) {
	uint32_t voltage = cal_getVoltage();
	uint32_t current = 0;
	switch (loadFunctions.type) {
	case FUNCTION_CC:
		current = loadFunctions.current;
		break;
	case FUNCTION_CV:
		//TODO
		break;
	case FUNCTION_CR:
		current = (voltage * 1000) / loadFunctions.resistance;
		break;
	case FUNCTION_CP:
		if (voltage > 0)
			current = (loadFunctions.power * 1000) / voltage;
		else
			current = MAX_CURRENT;
		break;
	}
	cal_setCurrent(current);
}
