#include "settings.h"

static container_t c;
static dropdown_t dBaud;
static entry_t eMaxCurrent, eMaxPower, eMinVoltage, eMaxVoltage, eMinResist,
        eMaxResist;
static label_t lbaud, lMaxCurrent, lMaxPower, lMinVoltage, lMaxVoltage,
        lMinResist, lMaxResist, lOnError;
static button_t bDefLow, bDefHigh, bLoad, bSave;
static checkbox_t bOnError;
static char *entries[8];
static const char availableBaudrates[7][7] = { "1200", "4800", "9600", "19200",
        "38400", "57600", "115200" };

void settings_Init(void) {
    settings.baudrate = SETTINGS_DEF_BAUDRATE;
    settings_setDefaultLow();
    settings.turnOffOnError = 1;

    /* create GUI elements */
    /* create labels */
    label_createWithText(&lbaud, "Baudrate:", FONT_MEDIUM);
    label_createWithText(&lMaxCurrent, "Max. I:", FONT_MEDIUM);
    label_createWithText(&lMaxPower, "Max. P:", FONT_MEDIUM);
    label_createWithText(&lMinVoltage, "Min. V:", FONT_MEDIUM);
    label_createWithText(&lMaxVoltage, "Max. V:", FONT_MEDIUM);
    label_createWithText(&lMinResist, "Min. R:", FONT_MEDIUM);
    label_createWithText(&lMaxResist, "Max. R:", FONT_MEDIUM);
    label_createWithText(&lOnError, "Turn off on error:", FONT_MEDIUM);
    /* create buttons */
    button_create(&bDefLow, "DEFAULT LOW", FONT_SMALL, 0,
            settings_setDefaultLow);
    button_create(&bDefHigh, "DEFAULT HIGH", FONT_SMALL, 0,
            settings_setDefaultHigh);
    button_create(&bLoad, "Load", FONT_MEDIUM, 0, settings_LoadMenu);
    button_create(&bSave, "Save", FONT_MEDIUM, 0, settings_SaveMenu);
    checkbox_create(&bOnError, &settings.turnOffOnError, NULL);
    /* create baud chooser */
    int8_t sel;
    for (sel = 0; sel < 7; sel++) {
        entries[sel] = availableBaudrates[sel];
    }
    entries[7] = 0;
    dropdown_create(&dBaud, entries, (uint8_t*) &settings.baudrate, FONT_MEDIUM,
            0, settings_UpdateBaudrate);
    /* create entries */
    entry_create(&eMaxCurrent, &settings.maxCurrent, NULL, NULL, FONT_MEDIUM, 4,
            UNIT_CURRENT, settings_ChangedValue);
    entry_create(&eMaxPower, &settings.maxPower, NULL, NULL, FONT_MEDIUM, 4,
            UNIT_POWER, settings_ChangedValue);
    entry_create(&eMinVoltage, &settings.minVoltage, NULL, NULL, FONT_MEDIUM, 4,
            UNIT_VOLTAGE, settings_ChangedValue);
    entry_create(&eMaxVoltage, &settings.maxVoltage, NULL, NULL, FONT_MEDIUM, 4,
            UNIT_VOLTAGE, settings_ChangedValue);
    entry_create(&eMinResist, &settings.minResistance, NULL, NULL, FONT_MEDIUM,
            4, UNIT_RESISTANCE, settings_ChangedValue);
    entry_create(&eMaxResist, &settings.maxResistance, NULL, NULL, FONT_MEDIUM,
            4, UNIT_RESISTANCE, settings_ChangedValue);

    container_create(&c, 128, 55);

    container_attach(&c, &lbaud, 0, 2);
    container_attach(&c, &dBaud, 60, 0);
    container_attach(&c, &bDefLow, 0, 12);
    container_attach(&c, &bDefHigh, 60, 12);
    container_attach(&c, &lMaxCurrent, 0, 26);
    container_attach(&c, &lMaxPower, 0, 38);
    container_attach(&c, &lMinVoltage, 0, 50);
    container_attach(&c, &lMaxVoltage, 0, 62);
    container_attach(&c, &lMinResist, 0, 74);
    container_attach(&c, &lMaxResist, 0, 86);

    container_attach(&c, &eMaxCurrent, 60, 24);
    container_attach(&c, &eMaxPower, 60, 36);
    container_attach(&c, &eMinVoltage, 60, 48);
    container_attach(&c, &eMaxVoltage, 60, 60);
    container_attach(&c, &eMinResist, 60, 72);
    container_attach(&c, &eMaxResist, 60, 84);

    container_attach(&c, &lOnError, 0, 98);
    container_attach(&c, &bOnError, 108, 96);

    container_attach(&c, &bLoad, 60, 109);
    container_attach(&c, &bSave, 93, 109);
}

widget_t* settings_getWidget(void) {
    return (widget_t*) &c;
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
        settings_ChangedValue();
        settings_UpdateBaudrate();
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

void settings_UpdateBaudrate(void) {
    const uint32_t baud[7] = { 1200, 4800, 9600, 19200, 38400, 57600, 115200 };
    /* wait for ongoing transmission to finish */
    while (uart.busyFlag)
        ;
    uart_Init(baud[settings.baudrate]);
}

void settings_ChangedValue(void) {
    /* check if powerMode has to be switched */
    uint8_t highPower = 0;
    if (settings.maxCurrent > LOAD_MAXCURRENT_LOWP
            || settings.maxPower > LOAD_MAXPOWER_LOWP
            || settings.minResistance < LOAD_MINRESISTANCE_LOWP) {
        /* can't reach settings in low power mode -> switch to high power */
        highPower = 1;
    }
    settings.powerMode = highPower;
}

void settings_setDefaultLow(void) {
    settings.maxCurrent = LOAD_MAXCURRENT_LOWP;
    settings.maxPower = LOAD_MAXPOWER_LOWP;
    settings.minVoltage = LOAD_MINVOLTAGE_LOWP;
    settings.maxVoltage = LOAD_MAXVOLTAGE_LOWP;
    settings.minResistance = LOAD_MINRESISTANCE_LOWP;
    settings.maxResistance = LOAD_MAXRESISTANCE_LOWP;
    settings.powerMode = 0;
}

void settings_setDefaultHigh(void) {
    settings.maxCurrent = LOAD_MAXCURRENT_HIGHP;
    settings.maxPower = LOAD_MAXPOWER_HIGHP;
    settings.minVoltage = LOAD_MINVOLTAGE_HIGHP;
    settings.maxVoltage = LOAD_MAXVOLTAGE_HIGHP;
    settings.minResistance = LOAD_MINRESISTANCE_HIGHP;
    settings.maxResistance = LOAD_MAXRESISTANCE_HIGHP;
    settings.powerMode = 1;
}

void settings_LoadMenu(void) {
    GUIMessageResult_t res = message_Box("Load values\nfrom FLASH?", 2, 12,
            FONT_MEDIUM, MESSAGE_OK_ABORT, &bLoad);
    if (res == MESSAGE_RES_OK) {
        if (settings_readFromFlash()) {
            /* failed to read values */
            message_Box("No data in FLASH", 1, 16, FONT_MEDIUM, MESSAGE_OK,
                    &bLoad);
        }
    }
}

void settings_SaveMenu(void) {
    GUIMessageResult_t res = message_Box("Save current\nvalues into\nFLASH?", 3,
            12, FONT_MEDIUM, MESSAGE_OK_ABORT, &bSave);

    if (res == MESSAGE_RES_OK) {
        settings_writeToFlash();
    }
}
