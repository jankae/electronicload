#include "main.h"

void _exit(int a) {
    while (1) {
    };
}

int main(int argc, char* argv[]) {
    SystemInit();
    SysTick_Config(72000);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    // Peripheral inits
    timer_Init();
    hal_currentSinkInit();
    hal_displayInit();
    hal_frontPanelInit();
    hal_triggerInit();

    // External hardware inits
    // (nothing so far)

    // Software inits
    load_Init();
    if (cal_readFromFlash()) {
        // no valid calibration data available
        cal_setDefaultCalibration();
    }

    timer_SetupPeriodicFunction(3, MS_TO_TICKS(20), hal_updateDisplay, 12);

    // setup main menu
    menu_AddMainMenuEntry("Calibration", calibrationProcess);

    menu_DefaultScreenHandler();
}

