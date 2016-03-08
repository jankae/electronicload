#include "main.h"

void _exit(int a) {
    while (1) {
    };
}

int main(int argc, char* argv[]) {
    /* TODO move this section into HAL */
    SystemInit();
    SysTick_Config(72000);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    // Peripheral inits
    timer_Init();
    hal_currentSinkInit();
    hal_displayInit();
    hal_frontPanelInit();
    hal_triggerInit();
    uart_Init();
    uart_writeString("electronicload\n");

    timer_waitms(100);

// External hardware inits
// (nothing so far)

// Software inits
    events_Init();
    load_Init();
    stats_Reset();
    if (cal_readFromFlash()) {
        // no valid calibration data available
        cal_setDefaultCalibration();
        uart_writeString("WARNING: not calibrated\n");
    } else {
        uart_writeString("calibration loaded\n");
    }

    timer_SetupPeriodicFunction(3, MS_TO_TICKS(20), hal_updateDisplay, 12);

    // setup main menu
    menu_AddMainMenuEntry("Events", events_menu);
    menu_AddMainMenuEntry("Calibration", calibrationProcess);
    menu_AddMainMenuEntry("Statistics", stats_Display);

    menu_DefaultScreenHandler();
}

