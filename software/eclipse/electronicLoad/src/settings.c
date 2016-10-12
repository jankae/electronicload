#include "settings.h"

void settings_Init(void) {
    settings.baudrate = SETTINGS_DEF_BAUDRATE;
    settings.powerMode = 0;
    settings.maxCurrent[0] = LOAD_MAXCURRENT_LOWP;
    settings.maxPower[0] = LOAD_MAXPOWER_LOWP;
    settings.minVoltage[0] = LOAD_MINVOLTAGE_LOWP;
    settings.maxVoltage[0] = LOAD_MAXVOLTAGE_LOWP;
    settings.minResistance[0] = LOAD_MINRESISTANCE_LOWP;
    settings.maxResistance[0] = LOAD_MAXRESISTANCE_LOWP;
    settings.maxCurrent[1] = LOAD_MAXCURRENT_HIGHP;
    settings.maxPower[1] = LOAD_MAXPOWER_HIGHP;
    settings.minVoltage[1] = LOAD_MINVOLTAGE_HIGHP;
    settings.maxVoltage[1] = LOAD_MAXVOLTAGE_HIGHP;
    settings.minResistance[1] = LOAD_MINRESISTANCE_HIGHP;
    settings.maxResistance[1] = LOAD_MAXRESISTANCE_HIGHP;
    settings.powerControl = CONTROL_ANALOG;
    settings.resistanceControl = CONTROL_ANALOG;
    settings.turnOffOnError = 1;
}

uint8_t settings_readFromFlash(void) {
    // check whether there is any settings data in FLASH
    if (*(uint32_t*) FLASH_VALID_SETTINGS_INDICATOR == SETTINGS_INDICATOR) {
        // copy memory section from FLASH into RAM
        uint8_t i;
        uint32_t *from = (uint32_t*) FLASH_SETTINGS_DATA;
        uint32_t *to = (uint32_t*) &settings;
        uint8_t words = (sizeof(settings) + 3) / 4;
        for (i = 0; i < words; i++) {
            *to = *from;
            to++;
            from++;
        }
        return 0;
    }
    return 1;
}

void settings_writeToFlash(void) {
    FLASH_Unlock();
    FLASH_ClearFlag(
    FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
    FLASH_ErasePage(0x0807E000);
    if (sizeof(settings) >= 0x400)
        FLASH_ErasePage(0x0807E400);
    if (sizeof(settings) >= 0x800)
        FLASH_ErasePage(0x0807E800);
    if (sizeof(settings) >= 0xC00)
        FLASH_ErasePage(0x0807EC00);
    // FLASH is ready to be written at this point
    uint8_t i;
    uint32_t *from = (uint32_t*) &settings;
    uint32_t *to = (uint32_t*) FLASH_SETTINGS_DATA;
    uint8_t words = (sizeof(settings) + 3) / 4;
    for (i = 0; i < words; i++) {
        FLASH_ProgramWord((uint32_t) to, *from);
        to++;
        from++;
    }
    // set valid data indicator
    FLASH_ProgramWord((uint32_t) FLASH_VALID_SETTINGS_INDICATOR,
    SETTINGS_INDICATOR);
    FLASH_Lock();
}

void settings_Menu(void) {
    char *entries[SETTINGS_NUM_ENTRIES + 3];
    int8_t sel = 0;
    do {
        uint32_t minRes, maxRes, maxA, minV, maxV, maxW;

        char settingBaudrate[21] = "Baudrate: ";
        string_fromUintUnit(settings.baudrate, &settingBaudrate[10], 6, 0, 0);

        char maxCurrent[21] = "Max. Current:";
        char maxPower[21] = "Max. Power:  ";
        char minVoltage[21] = "Min. Voltage:";
        char maxVoltage[21] = "Max. Voltage:";
        char minResist[21] = "Min. Resist: ";
        char maxResist[21] = "Max. Resist: ";
        char settingHigh[21];
        char powerControl[21];
        char resistanceControl[21];
        char onError[21];

        if (settings.powerMode) {
            strcpy(settingHigh, "Mode: high power");
            minRes = LOAD_MINRESISTANCE_HIGHP;
            maxRes = LOAD_MAXRESISTANCE_HIGHP;
            maxA = LOAD_MAXCURRENT_HIGHP;
            minV = LOAD_MINVOLTAGE_HIGHP;
            maxV = LOAD_MAXVOLTAGE_HIGHP;
            maxW = LOAD_MAXPOWER_HIGHP;

        } else {
            strcpy(settingHigh, "Mode: low power");
            minRes = LOAD_MINRESISTANCE_LOWP;
            maxRes = LOAD_MAXRESISTANCE_LOWP;
            maxA = LOAD_MAXCURRENT_LOWP;
            minV = LOAD_MINVOLTAGE_LOWP;
            maxV = LOAD_MAXVOLTAGE_LOWP;
            maxW = LOAD_MAXPOWER_LOWP;
        }

        if (settings.powerControl == CONTROL_ANALOG) {
            strcpy(powerControl, "CP Ctrl: ANALOG");
        } else {
            strcpy(powerControl, "CP Ctrl: DIGITAL");
        }

        if (settings.resistanceControl == CONTROL_ANALOG) {
            strcpy(resistanceControl, "CR Ctrl: ANALOG");
        } else {
            strcpy(resistanceControl, "CR Ctrl: DIGITAL");
        }

        if (settings.turnOffOnError) {
            strcpy(onError, "On error: turn off");
        } else {
            strcpy(onError, "On error: keep on");
        }

        string_fromUintUnit(settings.maxCurrent[settings.powerMode],
                &maxCurrent[13], 4, 6, 'A');
        string_fromUintUnit(settings.maxPower[settings.powerMode],
                &maxPower[13], 4, 6, 'W');
        string_fromUintUnit(settings.minVoltage[settings.powerMode],
                &minVoltage[13], 4, 6, 'V');
        string_fromUintUnit(settings.maxVoltage[settings.powerMode],
                &maxVoltage[13], 4, 6, 'V');
        string_fromUintUnit(settings.minResistance[settings.powerMode],
                &minResist[13], 4, 3, 'R');
        string_fromUintUnit(settings.maxResistance[settings.powerMode],
                &maxResist[13], 4, 3, 'R');

        entries[0] = settingBaudrate;
        entries[1] = settingHigh;
        entries[2] = maxCurrent;
        entries[3] = maxPower;
        entries[4] = minVoltage;
        entries[5] = maxVoltage;
        entries[6] = minResist;
        entries[7] = maxResist;
        entries[8] = powerControl;
        entries[9] = resistanceControl;
        entries[10] = onError;

        char resetToDefault[21] = "Reset to default";
        entries[SETTINGS_NUM_ENTRIES] = resetToDefault;
        char readSaved[21] = "Load saved settings";
        entries[SETTINGS_NUM_ENTRIES + 1] = readSaved;
        char writeSaved[21] = "Save settings";
        entries[SETTINGS_NUM_ENTRIES + 2] = writeSaved;

        sel = menu_ItemChooseDialog(
                "\xCD\xCD\xCD\xCDSETTINGS MENU\xCD\xCD\xCD\xCD", entries,
                SETTINGS_NUM_ENTRIES + 3, sel);
        if (sel >= 0) {
            switch (sel) {
            case 0:
                settings_SelectBaudrate();
                break;
            case 1:
                settings.powerMode = !settings.powerMode;
                break;
            case 2:
                menu_getInputValue(&settings.maxCurrent[settings.powerMode],
                        maxCurrent, 0, maxA, NULL, "mA", "A");
                break;
            case 3:
                menu_getInputValue(&settings.maxPower[settings.powerMode],
                        maxPower, 0, maxW, NULL, "mW", "W");
                break;
            case 4:
                menu_getInputValue(&settings.minVoltage[settings.powerMode],
                        minVoltage, minV, maxV, NULL, "mV", "V");
                break;
            case 5:
                menu_getInputValue(&settings.maxVoltage[settings.powerMode],
                        maxVoltage, minV, maxV, NULL, "mV", "V");
                break;
            case 6:
                menu_getInputValue(&settings.minResistance[settings.powerMode],
                        minResist, minRes, maxRes, "mOhm", "Ohm", "kOhm");
                break;
            case 7:
                menu_getInputValue(&settings.maxResistance[settings.powerMode],
                        maxResist, minRes, maxRes, "mOhm", "Ohm", "kOhm");
                break;
            case 8:
                // toggle power control
                if (settings.powerControl == CONTROL_ANALOG)
                    settings.powerControl = CONTROL_DIGITAL;
                else
                    settings.powerControl = CONTROL_ANALOG;
                break;
            case 9:
                // toggle resistance control
                if (settings.resistanceControl == CONTROL_ANALOG)
                    settings.resistanceControl = CONTROL_DIGITAL;
                else
                    settings.resistanceControl = CONTROL_ANALOG;
                break;
            case 10:
                settings.turnOffOnError = !settings.turnOffOnError;
                break;
            case SETTINGS_NUM_ENTRIES:
                settings_ResetToDefaultMenu();
                break;
            case SETTINGS_NUM_ENTRIES + 1:
                settings_LoadMenu();
                break;
            case SETTINGS_NUM_ENTRIES + 2:
                settings_SaveMenu();
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
            entries, 7, 0);
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

void settings_ResetToDefaultMenu(void) {
    // wait for all buttons to be released
    while (hal_getButton())
        ;
    screen_Clear();
    screen_FastString6x8("Reset all settings to", 0, 0);
    screen_FastString6x8("default values?", 0, 1);
    screen_SetSoftButton("No", 0);
    screen_SetSoftButton("Yes", 2);
    uint32_t button;
    do {
        button = hal_getButton();
    } while (!(button & (HAL_BUTTON_ESC | HAL_BUTTON_SOFT0 | HAL_BUTTON_SOFT2)));

    if (button & HAL_BUTTON_SOFT2) {
        // reset settings if 'yes' has been selected
        settings_Init();
    }
}

void settings_LoadMenu(void) {
    // wait for all buttons to be released
    while (hal_getButton())
        ;
    screen_Clear();
    screen_FastString6x8("Reset all settings to", 0, 0);
    screen_FastString6x8("saved values?", 0, 1);
    screen_SetSoftButton("No", 0);
    screen_SetSoftButton("Yes", 2);
    uint32_t button;
    do {
        button = hal_getButton();
    } while (!(button & (HAL_BUTTON_ESC | HAL_BUTTON_SOFT0 | HAL_BUTTON_SOFT2)));

    if (button & HAL_BUTTON_SOFT2) {
        // read settings if 'yes' has been selected
        if (settings_readFromFlash()) {
            // couldn't read data
            while (hal_getButton())
                ;
            screen_Clear();
            screen_FastString6x8("No saved values", 0, 0);
            screen_FastString6x8("available", 0, 1);
            screen_SetSoftButton("OK", 2);
            while (!(hal_getButton() & (HAL_BUTTON_ESC | HAL_BUTTON_SOFT2)))
                ;
        }
    }
}

void settings_SaveMenu(void) {
    // wait for all buttons to be released
    while (hal_getButton())
        ;
    screen_Clear();
    screen_FastString6x8("Use current settings", 0, 0);
    screen_FastString6x8("as saved values?", 0, 1);
    screen_SetSoftButton("No", 0);
    screen_SetSoftButton("Yes", 2);
    uint32_t button;
    do {
        button = hal_getButton();
    } while (!(button & (HAL_BUTTON_ESC | HAL_BUTTON_SOFT0 | HAL_BUTTON_SOFT2)));

    if (button & HAL_BUTTON_SOFT2) {
        // write settings if 'yes' has been selected
        settings_writeToFlash();
    }
}
