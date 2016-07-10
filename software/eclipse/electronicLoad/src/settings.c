#include "settings.h"

void settings_Menu(void) {
    char *entries[SETTINGS_NUM_ENTRIES];
    int8_t sel;
    do {
        char settingBaudrate[21] = "Baudrate: ";
        string_fromUint(settings.baudrate, &settingBaudrate[10], 6, 0);
        entries[0] = settingBaudrate;
        sel = menu_ItemChooseDialog(
                "\xCD\xCD\xCD\xCDSETTINGS MENU\xCD\xCD\xCD\xCD", entries,
                SETTINGS_NUM_ENTRIES);
        if (sel >= 0) {
            switch (sel) {
            case 0:
                settings_SelectBaudrate();
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
    switch(sel){
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
}
