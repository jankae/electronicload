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
    stats_Init();

    timer_SetupPeriodicFunction(3, MS_TO_TICKS(5), hal_updateDisplay, 12);

    selftest_Run();

    cal_Init();

    notebook_t n;

    notebook_create(&n, FONT_SMALL, 128, 64);
    const char waveform[] = "WAVEFORM";
    notebook_addPage(&n, waveform_getWidget(), waveform);
    const char characteristics[] = "U/I-CURVE";
    notebook_addPage(&n, characteristic_getWidget(), characteristics);
    const char arbitrary[] = "ARBITRARY";
    notebook_addPage(&n, arb_getWidget(), arbitrary);
    const char stats[] = "STATISTIC";
    notebook_addPage(&n, stats_getWidget(), stats);
    const char settings[] = "SETTINGS";
    notebook_addPage(&n, settings_getWidget(), settings);
    const char calib[] = "CALIBRATION";
    notebook_addPage(&n, cal_getWidget(), calib);

    GUISignal_t signal;
    memset(&signal, 0, sizeof(signal));
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
        widget_Redraw(&n);
    }

//    // setup main menu
//    menu_AddMainMenuEntry("Waveforms", waveform_Menu);
    menu_AddMainMenuEntry("Events", events_menu);
//    menu_AddMainMenuEntry("Arbitrary Sequence", arb_Menu);
//    menu_AddMainMenuEntry("U/I characteristic", characteristic_Menu);
//    menu_AddMainMenuEntry("Statistics", stats_Display);
//    menu_AddMainMenuEntry("Settings", settings_Menu);
//    menu_AddMainMenuEntry("Calibration", calibrationMenu);
    menu_AddMainMenuEntry("Tests", test_Menu);
    menu_AddMainMenuEntry("Errors", error_Menu);
//
//    menu_DefaultScreenHandler();
}

