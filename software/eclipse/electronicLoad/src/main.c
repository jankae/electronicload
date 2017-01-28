#include "main.h"

void _exit(int a) {
    while (1) {
    };
}

uint32_t testVar = 0;

void buttonCallback1(void) {
    testVar++;
    uart_writeString("Button1 clicked\n");
}
void buttonCallback2(void) {
    testVar += 10;
    uart_writeString("Button2 clicked\n");
}
void buttonCallback3(void) {
    testVar += 100;
    uart_writeString("Button3 clicked\n");
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
    arb_Init();
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

//    screen_Clear();
//    screen_InvertChar12x16(0, 0);
//    screen_InvertChar12x16(12, 0);
//    screen_InvertChar12x16(24, 0);
//    screen_Char4x6('A', 0, 0);
//    screen_Char4x6('A', 4, 1);
//    screen_Char4x6('A', 8, 2);
//    screen_Char4x6('A', 12, 3);
//    screen_Char4x6('A', 16, 4);
//    screen_Char4x6('A', 20, 5);
//    screen_Char4x6('A', 24, 6);
//    screen_Char4x6('A', 28, 7);

//    while(1);
    // TODO remove widget test

    container_t c;
    container_create(&c, 128, 64);
    button_t testButton1;
    char uart[] = "+1";
    button_create(&testButton1, 20, 10, uart, buttonCallback1);
    button_t testButton2;
    char uart2[] = "+10";
    button_create(&testButton2, 20, 10, uart2, buttonCallback2);
    button_t testButton3;
    char uart3[] = "+100";
    button_create(&testButton3, 20, 10, uart3, buttonCallback3);

    entry_t display;
    uint32_t min = 0;
    uint32_t max = 20000000;
    entry_create(&display, &testVar, &max, &min, FONT_MEDIUM, 4, UNIT_CURRENT, NULL);

    container_attach(&c, &testButton1, 0, 0);
    container_attach(&c, &testButton2, 30, 0);
    container_attach(&c, &testButton3, 60, 0);
    container_attach(&c, &display, 0, 30);
    GUISignal_t signal;
    coords_t origin = { .x = 0, .y = 0 };
    screen_Clear();
    while (1) {
        do {
            signal.clicked = hal_getButton();
            signal.encoder = hal_getEncoderMovement();
        } while (!signal.clicked && signal.encoder == 0);
        while (hal_getButton())
            ;
        widget_input(&c, signal);
        screen_Clear();
        widget_draw(&c, origin);
    }

    // setup main menu
    menu_AddMainMenuEntry("Waveforms", waveform_Menu);
    menu_AddMainMenuEntry("Events", events_menu);
    menu_AddMainMenuEntry("Arbitrary Sequence", arb_Menu);
    menu_AddMainMenuEntry("U/I characteristic", characteristic_Menu);
    menu_AddMainMenuEntry("Statistics", stats_Display);
    menu_AddMainMenuEntry("Settings", settings_Menu);
    menu_AddMainMenuEntry("Calibration", calibrationMenu);
    menu_AddMainMenuEntry("Tests", test_Menu);
    menu_AddMainMenuEntry("Errors", error_Menu);

    menu_DefaultScreenHandler();
}

