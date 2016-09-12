#include "settings.h"

void settings_Init(void) {
    settings.baudrate = SETTINGS_DEF_BAUDRATE;
    settings.maxCurrent = LOAD_MAXCURRENT;
    settings.maxPower = LOAD_MAXPOWER;
    settings.maxVoltage = LOAD_MAXVOLTAGE;
    settings.minResistance = LOAD_MINRESISTANCE;
}

void settings_Menu(void) {
    char *entries[SETTINGS_NUM_ENTRIES];
    int8_t sel;
    do {
        char settingBaudrate[21] = "Baudrate: ";
        string_fromUint(settings.baudrate, &settingBaudrate[10], 6, 0);
        entries[0] = settingBaudrate;

        char maxCurrent[21] = "Max. Current:";
        string_fromUint(settings.maxCurrent, &maxCurrent[13], 6, 3);
        entries[1] = maxCurrent;

        char maxPower[21] = "Max. Power:  ";
        string_fromUint(settings.maxPower, &maxPower[13], 6, 3);
        entries[2] = maxPower;

        char maxVoltage[21] = "Max. Voltage:";
        string_fromUint(settings.maxVoltage, &maxVoltage[13], 6, 3);
        entries[3] = maxVoltage;

        char minResist[21] = "Min. Resist: ";
        string_fromUint(settings.minResistance, &minResist[13], 6, 3);
        entries[4] = minResist;

        sel = menu_ItemChooseDialog(
                "\xCD\xCD\xCD\xCDSETTINGS MENU\xCD\xCD\xCD\xCD", entries,
                SETTINGS_NUM_ENTRIES);
        if (sel >= 0) {
            switch (sel) {
            case 0:
                settings_SelectBaudrate();
                break;
            case 1:
                menu_getInputValue(&settings.maxCurrent, maxCurrent, 0,
                LOAD_MAXCURRENT, "mA", "A", NULL);
                break;
            case 2:
                menu_getInputValue(&settings.maxPower, maxPower, 0,
                LOAD_MAXPOWER, "mW", "W", NULL);
                break;
            case 3:
                menu_getInputValue(&settings.maxVoltage, maxVoltage, LOAD_MINVOLTAGE,
                LOAD_MAXVOLTAGE, "mV", "V", NULL);
                break;
            case 4:
                menu_getInputValue(&settings.minResistance, minResist,
                        LOAD_MINRESISTANCE,
                        LOAD_MAXRESISTANCE, "mOhm", "Ohm", NULL);
                break;
            }
        }
    } while (sel >= 0);
}

void settings_SelectBaudrate(void) {
    char *entries[7];
    const char availableBaudrates[7][7] = { "1200", "4800", "9600", "19200",
            "38400", "57600", "115200" };
    int8_t sel;
    for (sel = 0; sel < 7; sel++) {
        entries[sel] = availableBaudrates[sel];
    }
    sel = menu_ItemChooseDialog("\xCD\xCD\xCDSELECT BAUDRATE\xCD\xCD\xCD",
            entries, 7);
    uint32_t baudratebuffer = settings.baudrate;
    switch (sel) {
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
