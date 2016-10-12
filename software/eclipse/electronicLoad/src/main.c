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
    multimeter_Init();
    uart_Init(115200);
    uart_writeString("electronicload\n");

    timer_waitms(500);

// External hardware inits
// (nothing so far)

// Software inits
    if (settings_readFromFlash()) {
        // no settings saved in flash -> use default values
        settings_Init();
    }
    events_Init();
    waveform_Init();
    load_Init();
    com_Init();
    stats_Reset();

    timer_SetupPeriodicFunction(3, MS_TO_TICKS(5), hal_updateDisplay, 12);

    selftest_Run();

    if (cal_readFromFlash()) {
        // no valid calibration data available
        cal_setDefaultCalibration();
        uart_writeString("WARNING: not calibrated\n");
        screen_Clear();
        screen_FastString12x16("WARNING", 22, 0);
        screen_FastString6x8("Not calibrated.", 0, 2);
        screen_FastString6x8("Continue anyway?", 0, 3);
        screen_SetSoftButton("Yes", 2);
        while (!(hal_getButton() & HAL_BUTTON_SOFT2))
            ;
        while (hal_getButton())
            ;
    } else {
        uart_writeString("calibration loaded\n");
    }

    // setup main menu
    menu_AddMainMenuEntry("Waveforms", waveform_Menu);
    menu_AddMainMenuEntry("Events", events_menu);
    menu_AddMainMenuEntry("U/I characteristic", characteristic_Menu);
    menu_AddMainMenuEntry("Calibration", calibrationMenu);
    menu_AddMainMenuEntry("Statistics", stats_Display);
    menu_AddMainMenuEntry("Settings", settings_Menu);
    menu_AddMainMenuEntry("Tests", test_Menu);
    menu_AddMainMenuEntry("Errors", error_Menu);

    menu_DefaultScreenHandler();
}

