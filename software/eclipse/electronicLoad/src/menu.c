/**
 * \file
 * \brief Menu routines source file
 *
 * Contains menu and user interface functions
 */
#include "menu.h"

/**
 * \brief Handles user inputs while the default screen is active
 *
 * Functionality includes: displaying selected mode and value,
 * switching load on/off, switching between standard modes (CC,
 * CV, CR, CP) and handling encoder inputs (changes the value
 * correspondent to the selected mode)
 */
void menu_DefaultScreenHandler(void) {
    int32_t *setvalue;
    int32_t minValue;
    int32_t maxValue;
    uint8_t encoderPosition = 0;
    uint32_t baseInkrement;
    uint8_t dotPosition;
    uint8_t maxEncoderPosition;
    while (1) {
        // wait for all buttons to be released
        while (hal_getButton())
            ;
        // set default screen entries
        screen_Clear();
        char buf[11];
        if (load.state.temp1 > LOAD_MAX_TEMP || load.state.temp2 > LOAD_MAX_TEMP) {
            screen_FastString12x16("HIGH TEMP", 10, 4);
        } else if (!load.powerOn) {
            screen_FastString12x16("INPUT OFF", 10, 4);
        } else if (waveform.form != WAVE_NONE) {
            screen_FastString12x16("WAVE ON", 22, 4);
        }

        char bigUnit[8];
        char smallUnit1[8];
        char smallUnit2[8];
        switch (load.mode) {
        case FUNCTION_CC:
            string_fromUintUnit(load.state.current, bigUnit, 4, 6, 'A');
            string_fromUintUnit(load.state.voltage, smallUnit1, 4, 6, 'V');
            string_fromUintUnit(load.state.power, smallUnit2, 4, 6, 'W');

            screen_FastString6x8("CC-Mode:", 0, 3);
            if (settings.powerMode) {
                string_fromUint(load.current / 1000, buf, 5, 3);
                screen_FastChar6x8(114, 3, 'A');
                baseInkrement = 1000;
                dotPosition = 3;
                maxEncoderPosition = 5;
            } else {
                string_fromUint(load.current / 10, buf, 5, 2);
                screen_FastString6x8("mA", 114, 3);
                baseInkrement = 10;
                dotPosition = 2;
                maxEncoderPosition = 5;
            }
            screen_FastString6x8(buf, 78, 3);
            setvalue = &load.current;
            minValue = 0;
            maxValue = settings.maxCurrent[settings.powerMode];
            break;
        case FUNCTION_CV:
            string_fromUintUnit(load.state.current, smallUnit1, 4, 6, 'A');
            string_fromUintUnit(load.state.voltage, bigUnit, 4, 6, 'V');
            string_fromUintUnit(load.state.power, smallUnit2, 4, 6, 'W');

            screen_FastString6x8("CV-Mode:", 0, 3);
            string_fromUint(load.voltage / 10000, buf, 5, 2);
            screen_FastChar6x8(114, 3, 'V');
            baseInkrement = 10000;
            dotPosition = 2;
            maxEncoderPosition = 5;

            screen_FastString6x8(buf, 78, 3);
            setvalue = &load.voltage;
            minValue = settings.minVoltage[settings.powerMode];
            maxValue = settings.maxVoltage[settings.powerMode];
            break;
        case FUNCTION_CR:
            string_fromUintUnit(load.state.current, bigUnit, 4, 6, 'A');
            string_fromUintUnit(load.state.voltage, smallUnit1, 4, 6, 'V');
            string_fromUintUnit(load.state.power, smallUnit2, 4, 6, 'W');

            screen_FastString6x8("CR-Mode:", 0, 3);
            if (settings.powerMode) {
                string_fromUint(load.resistance / 10, buf, 5, 2);
                screen_FastChar6x8(114, 3, 'R');
                baseInkrement = 10;
                dotPosition = 2;
                maxEncoderPosition = 5;
            } else {
                string_fromUint(load.resistance / 1000, buf, 5, 3);
                screen_FastString6x8("kR", 114, 3);
                baseInkrement = 1000;
                dotPosition = 3;
                maxEncoderPosition = 5;
            }
            screen_FastString6x8(buf, 78, 3);

            setvalue = &load.resistance;
            minValue = settings.minResistance[settings.powerMode];
            maxValue = settings.maxResistance[settings.powerMode];
            break;
        case FUNCTION_CP:
            string_fromUintUnit(load.state.current, smallUnit1, 4, 6, 'A');
            string_fromUintUnit(load.state.voltage, smallUnit2, 4, 6, 'V');
            string_fromUintUnit(load.state.power, bigUnit, 4, 6, 'W');

            screen_FastString6x8("CP-Mode:", 0, 3);
            if (settings.powerMode) {
                string_fromUint(load.power / 10000, buf, 5, 2);
                screen_FastChar6x8(114, 3, 'W');
                baseInkrement = 10000;
                dotPosition = 2;
                maxEncoderPosition = 5;
            } else {
                string_fromUint(load.power / 100, buf, 5, 1);
                screen_FastString6x8("mW", 114, 3);
                baseInkrement = 100;
                dotPosition = 1;
                maxEncoderPosition = 5;
            }
            screen_FastString6x8(buf, 78, 3);

            setvalue = &load.power;
            minValue = 0;
            maxValue = settings.maxPower[settings.powerMode];
            break;
        }
        screen_FastString12x16(bigUnit, 0, 0);
        screen_FastString6x8(smallUnit1, 86, 0);
        screen_FastString6x8(smallUnit2, 86, 1);

        screen_InvertChar6x8(108 - encoderPosition * 6, 3);

        string_fromUintUnit(load.state.temp1, buf, 3, 0, 0);
        buf[3] = '\xf8';
        buf[4] = 'C';
        buf[5] = '/';
        string_fromUintUnit(load.state.temp2, &buf[6], 3, 0, 0);
        buf[9] = '\xf8';
        buf[10] = 'C';
        screen_FastString6x8(buf, 62, 2);

        screen_SetSoftButton("\x1b", 0);
        screen_SetSoftButton("\x1a", 1);
        screen_SetSoftButton("Menu", 2);

        uint32_t button;
        int32_t encoder;
        uint32_t wait = timer_SetTimeout(300);
        do {
            button = hal_getButton();
            encoder = hal_getEncoderMovement();
            timer_waitms(10);
        } while (!button && !encoder && !timer_TimeoutElapsed(wait));
        // button has been pressed
        // -> evaluate
        /*********************************************************
         * four standard modes (CC, CV, CR, CP)
         ********************************************************/
        if (button & HAL_BUTTON_CC) {
            if (menu_getInputValue(&load.current, "'load current'", 0,
                    settings.maxCurrent[settings.powerMode], NULL, "mA", "A")) {
                load.mode = FUNCTION_CC;
                load.powerOn = 0;
                waveform.form = WAVE_NONE;
                encoderPosition = 0;
            }
        }
        if (button & HAL_BUTTON_CV) {
            if (menu_getInputValue(&load.voltage, "'load voltage'",
                    settings.minVoltage[settings.powerMode],
                    settings.maxVoltage[settings.powerMode], NULL, "mV", "V")) {
                load.mode = FUNCTION_CV;
                load.powerOn = 0;
                waveform.form = WAVE_NONE;
                encoderPosition = 0;
            }
        }
        if (button & HAL_BUTTON_CR) {
            if (menu_getInputValue(&load.resistance, "'load resistance'",
                    settings.minResistance[settings.powerMode],
                    settings.maxResistance[settings.powerMode], "mOhm", "Ohm",
                    NULL)) {
                load.mode = FUNCTION_CR;
                load.powerOn = 0;
                waveform.form = WAVE_NONE;
                encoderPosition = 0;
            }
        }
        if (button & HAL_BUTTON_CP) {
            if (menu_getInputValue(&load.power, "'load power'", 0,
                    settings.maxPower[settings.powerMode], NULL, "mW", "W")) {
                load.mode = FUNCTION_CP;
                load.powerOn = 0;
                waveform.form = WAVE_NONE;
                encoderPosition = 0;
            }
        }
        /*********************************************************
         * load on/off
         ********************************************************/
        if (button & HAL_BUTTON_ONOFF) {
            // toggle on/off
            load.powerOn ^= 1;
        }
        /*********************************************************
         * encoder inkrement setting
         ********************************************************/
        if (button & HAL_BUTTON_SOFT0) {
            // move inkrement one position to the left, skip dot
            encoderPosition++;
            if (encoderPosition == dotPosition) {
                encoderPosition++;
            }
            if (encoderPosition > maxEncoderPosition)
                encoderPosition = maxEncoderPosition;
        }
        if (button & HAL_BUTTON_SOFT1) {
            // move inkrement one position to the right, skip dot
            if (encoderPosition > 0) {
                encoderPosition--;
                if (encoderPosition == dotPosition) {
                    encoderPosition--;
                }
            }
        }
        /*********************************************************
         * encoder inkrement handling
         ********************************************************/
        if (encoder) {
            uint32_t inkrement = 1;
            uint8_t j;
            for (j = 0; j < encoderPosition; j++) {
                if (j != dotPosition)
                    inkrement *= 10;
            }
            *setvalue += encoder * inkrement * baseInkrement;
        }
        if (*setvalue < minValue)
            *setvalue = minValue;
        if (*setvalue > maxValue)
            *setvalue = maxValue;
        /*********************************************************
         * enter main menu
         ********************************************************/
        if (button & HAL_BUTTON_SOFT2) {
            menu_MainMenu();
        }
    }
}

/**
 * \brief Displays the main menu and handles user inputs
 *
 * Displays the main menu with all entries registered using
 * menu_AddMainMenuEntry(). The user can move through the menu
 * by using the UP and DOWN buttons or turning the encoder.
 * A submenu item is entered by pressing ENTER or the encoder.
 */
void menu_MainMenu(void) {
    const char *entries[MENU_MAIN_MAX_ENRIES];
    uint8_t i;
    for (i = 0; i < menu.nentries; i++) {
        entries[i] = menu.entries[i].descr;
    }
    int8_t sel = 0;
    do {
        sel = menu_ItemChooseDialog(
                "\xCD\xCD\xCD\xCD\xCD\xCDMAIN MENU\xCD\xCD\xCD\xCD\xCD\xCD",
                entries, menu.nentries, sel);
        if (sel >= 0) {
            menu.entries[sel].menuFunction();
        }
    } while (sel >= 0);
}

/**
 * \brief Adds an entry to the main menu
 *
 * \param *descr        Name of the menu entry (up to 20 chararcters)
 * \param *menuFunction Pointer to function that will be called upon
 *                      selecting the menu entry
 */
void menu_AddMainMenuEntry(char *descr, void (*menuFunction)()) {
    if (menu.nentries < MENU_MAIN_MAX_ENRIES) {
        // still some space left -> add entry
        menu.entries[menu.nentries].menuFunction = menuFunction;
        uint8_t i = 0;
        while (*descr && i < 20) {
            menu.entries[menu.nentries].descr[i++] = *descr++;
        }
        // fill empty characters with space
        for (; i < 20; i++)
            menu.entries[menu.nentries].descr[i] = ' ';
        menu.entries[menu.nentries].descr[20] = 0;
        menu.nentries++;
    }
}

/**
 * \brief Retrieves a input parameter from the user
 *
 * This function handles the parameter input by the user.
 * It displays the input dialog and handles every button
 * press. Furthermore, it checks the input value against
 * boundaries. If it does not meet these boundaries an
 * error message is displayed and the input process is
 * repeated until a valid value has been set or the user
 * aborts the input process.
 *
 * \param *value    Pointer to the input parameter value
 *                  (will only be written to if input was successful)
 * \param *descr    Pointer to a char array describing the input parameter
 *                  (up to 21 characters)
 * \param min       Lower boundary for input value
 * \param max       Upper boundary for input value
 * \param unit1e0   Base Unit number (e.g. uA), can also be NULL
 * \param unit1e3   1000*base unit (e.g. mA), can also be NULL
 * \param unit1e6   1000000*base unit (e.g. A), can also be NULL
 * \return 0: input process aborted, 1: input parameter set
 */
uint8_t menu_getInputValue(uint32_t *value, char *descr, uint32_t min,
        uint32_t max, const char *unit1e0, const char *unit1e3,
        const char *unit1e6) {
    uint64_t inputValue;
    uint8_t valueValid = 0;
    do {
        while (hal_getButton())
            ;
        // display input mask
        screen_Clear();
        screen_FastString6x8(descr, 0, 0);
        screen_Rectangle(1, 13, 126, 34);
        screen_Rectangle(2, 14, 125, 33);
        // set unit buttons
        if (unit1e0) {
            screen_SetSoftButton(unit1e0, 0);
        }
        if (unit1e3) {
            screen_SetSoftButton(unit1e3, 1);
        }
        if (unit1e6) {
            screen_SetSoftButton(unit1e6, 2);
        }
        uint32_t button;
        uint32_t mult;
        // current input position (in input array)
        uint8_t inputPosition = 0;
        // position of dot (0xff = no dot set)
        uint8_t dotPosition = 0xff;
        // input buffer string
        char input[11];
        uint8_t i;
        // clear input array
        for (i = 0; i < 11; i++)
            input[i] = 0;
        // get input
        do {
            while (!(button = hal_getButton()))
                ;
            if (inputPosition < 10) {
                // digits after dot not completely filled
                // -> can get new input
                if (button & HAL_BUTTON_ISDIGIT) {
                    // a digit button has been pressed
                    if (button & HAL_BUTTON_0)
                        input[inputPosition++] = '0';
                    if (button & HAL_BUTTON_1)
                        input[inputPosition++] = '1';
                    if (button & HAL_BUTTON_2)
                        input[inputPosition++] = '2';
                    if (button & HAL_BUTTON_3)
                        input[inputPosition++] = '3';
                    if (button & HAL_BUTTON_4)
                        input[inputPosition++] = '4';
                    if (button & HAL_BUTTON_5)
                        input[inputPosition++] = '5';
                    if (button & HAL_BUTTON_6)
                        input[inputPosition++] = '6';
                    if (button & HAL_BUTTON_7)
                        input[inputPosition++] = '7';
                    if (button & HAL_BUTTON_8)
                        input[inputPosition++] = '8';
                    if (button & HAL_BUTTON_9)
                        input[inputPosition++] = '9';
                    while (hal_getButton())
                        ;
                } else if ((button & HAL_BUTTON_DOT) && dotPosition == 0xff) {
                    // dot button pressed and dot not set yet
                    dotPosition = inputPosition;
                    input[inputPosition++] = '.';
                    while (hal_getButton())
                        ;
                }
            }
            screen_FastString12x16(input, 4, 2);
            if (unit1e0 && (button & HAL_BUTTON_SOFT0)) {
                mult = 1;
                break;
            }
            if (unit1e3 && (button & HAL_BUTTON_SOFT1)) {
                mult = 1000;
                break;
            }
            if (unit1e6 && (button & HAL_BUTTON_SOFT2)) {
                mult = 1000000;
                break;
            }
        } while (!(button & HAL_BUTTON_ESC));
        if (button & HAL_BUTTON_ESC)
            return 0;
        while (hal_getButton())
            ;
        // input buffer has been filled
        // -> transform to integer
        inputValue = 0;
        for (i = 0; i < 10 && input[i] != 0; i++) {
            if (input[i] >= '0' && input[i] <= '9') {
                // found a digit
                inputValue *= 10;
                inputValue += input[i] - '0';
            }
        }
        inputValue = inputValue * mult;
        if (dotPosition != 0xff) {
            // dot set
            for (; i - dotPosition > 1; i--) {
                inputValue /= 10;
            }
        }

        // check input value for boundaries
        if (inputValue > max || inputValue < min) {
            uint8_t dot = 0;
            if (mult == 1000)
                dot = 3;
            else if (mult == 1000000)
                dot = 6;
            // value outside of boundaries
            // display error message
            screen_Clear();
            screen_FastString12x16("ERROR", 0, 0);
            screen_FastString6x8("Min. parameter:", 0, 2);
            char buf[11];
            string_fromUint(min, buf, 9, dot);
            screen_FastString12x16(buf, 0, 3);
            screen_FastString6x8("Max. parameter:", 0, 5);
            string_fromUint(max, buf, 9, dot);
            screen_FastString12x16(buf, 0, 6);
            uint32_t timeout = timer_SetTimeout(6000);
            // display error message for 3 seconds
            // (can be aborted by pressing escape button)
            while (!(hal_getButton() & HAL_BUTTON_ESC)
                    && !timer_TimeoutElapsed(timeout))
                ;
        } else {
            valueValid = 1;
        }
    } while (!valueValid);

    *value = inputValue;
    return 1;
}

int8_t menu_ItemChooseDialog(const char *title, const char **items,
        uint8_t nitems, uint8_t startitem) {
    if (nitems == 0) {
        // no menu entries
        // -> no main menu
        return -1;
    }
    uint8_t selectedItem = 0;
    if (startitem < nitems)
        selectedItem = startitem;
    uint8_t firstDisplayedItem = 0;
    do {
        // wait for all buttons to be released
        while (hal_getButton())
            ;
        // display menu surroundings
        screen_Clear();
        screen_FastString6x8(title, 0, 0);
//        screen_FastString6x8("Use \x18,\x19,ESC and Enter", 0, 7);
        // display menu entries
        uint8_t i;
        for (i = 0; i < 7 && i + firstDisplayedItem < nitems; i++) {
            screen_FastString6x8(items[i + firstDisplayedItem], 6, i + 1);
        }
        // display arrow at selected menu entry
        screen_FastChar6x8(0, 1 + selectedItem - firstDisplayedItem, 0x1A);

        uint32_t button;
        int32_t encoder;
        // wait for user input
        do {
            button = hal_getButton();
            encoder = hal_getEncoderMovement();
        } while (!button && !encoder);
        // button has been pressed
        // -> evaluate
        /*********************************************************
         * moving in menu
         ********************************************************/
        if ((button & HAL_BUTTON_UP) || encoder < 0) {
            // move entry selection one up
            if (selectedItem > 0) {
                selectedItem--;
                // scroll if necessary
                if (selectedItem < firstDisplayedItem)
                    firstDisplayedItem = selectedItem;
            }
        }
        if ((button & HAL_BUTTON_DOWN) || encoder > 0) {
            // move entry selection one down
            if (selectedItem < nitems - 1) {
                selectedItem++;
                // scroll if necessary
                if (selectedItem > firstDisplayedItem + 6)
                    firstDisplayedItem = selectedItem - 6;
            }
        }
        /*********************************************************
         * leaving menu
         ********************************************************/
        if (button & HAL_BUTTON_ESC) {
            return -1;
        }
        /*********************************************************
         * entering submenu
         ********************************************************/
        if (button & (HAL_BUTTON_ENTER | HAL_BUTTON_ENCODER)) {
            return selectedItem;
        }
    } while (1); // not really an endless loop

}
