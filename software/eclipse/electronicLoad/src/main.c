#include "main.h"

void _exit(int a) {
    while (1) {
    };
}

uint32_t testVar = 0;
uint8_t baudrate = 2;
void baudChange(void) {
    uint32_t baudratebuffer = settings.baudrate;
    switch (baudrate) {
    case 0:
        settings.baudrate = 1200;
        break;
    case 1:
        settings.baudrate = 4800;
        break;
    case 2:
        settings.baudrate = 9600;
        break;
    case 3:
        settings.baudrate = 19200;
        break;
    case 4:
        settings.baudrate = 38400;
        break;
    case 5:
        settings.baudrate = 57600;
        break;
    case 6:
        settings.baudrate = 115200;
        break;
    }
    if (settings.baudrate != baudratebuffer) {
        // baudrate has changed -> re-init UART
        // wait for transmission to finish
        while (uart.busyFlag)
            ;
        uart_Init(settings.baudrate);
    }
}

void buttonCallback1(void) {
    uart_writeString("Button clicked\n");
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
    settings_Init();
    settings_readFromFlash();

    events_Init();
    waveform_Init();
    characteristic_Init();
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

    notebook_t n;

    notebook_create(&n, FONT_SMALL, 128, 64);
    const char waveform[] = "WAVEFORM";
    notebook_addPage(&n, waveform_getWidget(), waveform);
    const char characteristics[] = "U/I-CURVE";
    notebook_addPage(&n, characteristic_getWidget(), characteristics);
    const char settings[] = "SETTINGS";
    notebook_addPage(&n, settings_getWidget(), settings);

    GUISignal_t signal;
    memset(&signal, 0, sizeof(signal));
    coords_t origin = { .x = 0, .y = 0 };
    screen_Clear();
    n.flags.focussed = 1;
    n.base.flags.selected = 1;
    while (1) {
        do {
            signal.clicked = hal_getButton();
            signal.encoder = hal_getEncoderMovement();
        } while (!signal.clicked && signal.encoder == 0);
        while (hal_getButton())
            ;
        widget_input(&n, signal);
        screen_Clear();
        n.base.func.draw(&n, origin);
    }

//    // setup main menu
//    menu_AddMainMenuEntry("Waveforms", waveform_Menu);
//    menu_AddMainMenuEntry("Events", events_menu);
//    menu_AddMainMenuEntry("Arbitrary Sequence", arb_Menu);
//    menu_AddMainMenuEntry("U/I characteristic", characteristic_Menu);
//    menu_AddMainMenuEntry("Statistics", stats_Display);
//    menu_AddMainMenuEntry("Settings", settings_Menu);
//    menu_AddMainMenuEntry("Calibration", calibrationMenu);
//    menu_AddMainMenuEntry("Tests", test_Menu);
//    menu_AddMainMenuEntry("Errors", error_Menu);
//
//    menu_DefaultScreenHandler();
}

