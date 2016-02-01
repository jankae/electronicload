#include "main.h"

#define MS_TO_TICKS(ms) (72000*ms)

void _exit(int a) {
	while (1) {
	};
}

int main(int argc, char* argv[]) {
	SystemInit();
	SysTick_Config(72000);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	// Peripheral inits
	hal_currentSinkInit();
	hal_displayInit();
	hal_frontPanelInit();
	hal_triggerInit();

	// External hardware inits
	// (nothing so far)

	// Software inits
	load_Init();
	// TODO read calibration

	timer_SetupPeriodicFunction(2, MS_TO_TICKS(1), load_update, 4);
	timer_SetupPeriodicFunction(3, MS_TO_TICKS(20), hal_updateDisplay, 12);

	while (1) {
	}
}

