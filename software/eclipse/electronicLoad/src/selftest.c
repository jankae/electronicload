#include "selftest.h"

uint8_t selftest_Run(void) {
    screen_Clear();
    screen_FastString6x8("Running selftest...", 0, 0);
    int32_t rail5V = hal_ReadVoltageRail(HAL_RAIL_P5V);
    int32_t rail15V = hal_ReadVoltageRail(HAL_RAIL_P15V);
    int32_t railn15V = hal_ReadVoltageRail(HAL_RAIL_N15V);
    char value[15];

    do {
        screen_FastString6x8("5V rail:", 0, 1);
        string_fromUintUnit(rail5V, value, 4, 3, 'V');
        screen_FastString6x8(value, 60, 1);
        if (rail5V < 4500 || rail5V > 5500) {
            uart_writeString("selftest failed: 5V rail\n");
            break;
        }

        screen_FastString6x8("15V rail:", 0, 2);
        string_fromUintUnit(rail15V, value, 5, 3, 'V');
        screen_FastString6x8(value, 60, 2);
        if (rail15V < 13500 || rail15V > 16500) {
            uart_writeString("selftest failed: 15V rail\n");
            break;
        }

        screen_FastString6x8("-15V rail:", 0, 3);
        if (railn15V < 0) {
            railn15V = -railn15V;
        }
        string_fromUintUnit(railn15V, value, 5, 3, 'V');
        // add '-'-sign
        char *ptr = value;
        while (*ptr == ' ')
            ptr++;
        ptr--;
        *ptr = '-';
        screen_FastString6x8(value, 66, 3);
        if (railn15V < 13500 || railn15V > 16500) {
            uart_writeString("selftest failed: -15V rail\n");
            break;
        }

        screen_FastString12x16("PASSED", 28, 6);
        uart_writeString("selftest passed.\n");
        // show 'passed' message for 5 seconds or until any button is pressed
        uint16_t wait;
        for (wait = 0; wait < 500; wait++) {
            timer_waitms(10);
            if (hal_getButton())
                break;
        }
        while (hal_getButton())
            ;
        return 0;
    } while (0);
    screen_FastString12x16("FAILED", 28, 6);
    // wait for user input
    while (!hal_getButton())
        ;
    while (hal_getButton())
        ;
    screen_Clear();
    screen_FastString12x16("WARNING", 22, 0);
    screen_FastString6x8("Selftest has failed.", 0, 2);
    screen_FastString6x8("Continue anyway?", 0, 3);
    screen_SetSoftButton("Yes", 2);
    while (!(hal_getButton() & HAL_BUTTON_SOFT2))
        ;
    while (hal_getButton())
        ;
    return 1;
}
