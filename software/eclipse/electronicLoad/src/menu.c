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
    uint8_t encoderIndicator = 20;
    uint32_t encoderInkrement = 10;
    while (1) {
        // wait for all buttons to be released
        while (hal_getButton())
            ;
        // set default screen entries
        screen_Clear();
        char setValBuf[11];
        if (!load.powerOn) {
            screen_SetDefaultScreenString("!INPUT OFF!          ", 0, 0);
        } else {
            screen_SetDefaultScreenString("                     ", 0, 0);
        }
        screen_SetDefaultScreenString("\x19", encoderIndicator, 0);
        switch (load.mode) {
        case FUNCTION_CC:
            screen_SetDefaultScreenString("CC-Mode [A]:  ", 0, 1);
            string_fromUint(load.current/10, setValBuf, 5, 2);
            setvalue = &load.current;
            minValue = 0;
            maxValue = LOAD_MAXCURRENT;
            break;
        case FUNCTION_CV:
            screen_SetDefaultScreenString("CV-Mode [V]:  ", 0, 1);
            string_fromUint(load.voltage/10, setValBuf, 5, 2);
            setvalue = &load.voltage;
            minValue = LOAD_MINVOLTAGE;
            maxValue = LOAD_MAXVOLTAGE;
            break;
        case FUNCTION_CR:
            screen_SetDefaultScreenString("CR-Mode [Ohm]:", 0, 1);
            string_fromUint(load.resistance/10, setValBuf, 5, 2);
            setvalue = &load.resistance;
            minValue = LOAD_MINRESISTANCE;
            maxValue = LOAD_MAXRESISTANCE;
            break;
        case FUNCTION_CP:
            screen_SetDefaultScreenString("CP-Mode [W]:  ", 0, 1);
            string_fromUint(load.power/10, setValBuf, 5, 2);
            setvalue = &load.power;
            minValue = 0;
            maxValue = LOAD_MAXPOWER;
            break;
        }
        screen_SetDefaultScreenString(setValBuf, 15, 1);
        uint32_t button;
        int32_t encoder;
        do {
            screen_UpdateDefaultScreen();
            button = hal_getButton();
            encoder = hal_getEncoderMovement();
            timer_waitms(10);
        } while (!button && !encoder);
        // button has been pressed
        // -> evaluate
        /*********************************************************
         * four standard modes (CC, CV, CR, CP)
         ********************************************************/
        if (button & HAL_BUTTON_CC) {
            if (menu_getInputValue(&load.current, "'load current'", 0,
            LOAD_MAXCURRENT, 3)) {
                load.mode = FUNCTION_CC;
                load.powerOn = 0;
            }
        }
        if (button & HAL_BUTTON_CV) {
            if (menu_getInputValue(&load.voltage, "'load voltage'", 0,
            LOAD_MAXVOLTAGE, 3)) {
                load.mode = FUNCTION_CV;
                load.powerOn = 0;
            }
        }
        if (button & HAL_BUTTON_CR) {
            if (menu_getInputValue(&load.resistance,
                    "'load resistance'", LOAD_MINRESISTANCE,
                    LOAD_MAXRESISTANCE, 3)) {
                load.mode = FUNCTION_CR;
                load.powerOn = 0;
            }
        }
        if (button & HAL_BUTTON_CP) {
            if (menu_getInputValue(&load.power, "'load power'", 0,
            LOAD_MAXPOWER, 3)) {
                load.mode = FUNCTION_CP;
                load.powerOn = 0;
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
        if (button & HAL_BUTTON_LEFT) {
            // move inkrement one position to the left, skip dot
            if (encoderIndicator > 15) {
                // encoder inkrement is not all the way to the left
                // -> move it
                encoderIndicator--;
                if (encoderIndicator == 18) {
                    // set indicator to dot position
                    // -> move one position further
                    encoderIndicator--;
                }
                encoderInkrement *= 10;
            }
        }
        if (button & HAL_BUTTON_RIGHT) {
            // move inkrement one position to the right, skip dot
            if (encoderIndicator < 20) {
                // encoder inkrement is not all the way to the right
                // -> move it
                encoderIndicator++;
                if (encoderIndicator == 18) {
                    // set indicator to dot position
                    // -> move one position further
                    encoderIndicator++;
                }
                encoderInkrement /= 10;
            }
        }
        /*********************************************************
         * encoder inkrement handling
         ********************************************************/
        if (button & HAL_BUTTON_UP) {
            // move inkrement one position to the right, skip dot
            *setvalue += encoderInkrement;
        }
        if (button & HAL_BUTTON_DOWN) {
            // move inkrement one position to the right, skip dot
            *setvalue -= encoderInkrement;
        }
        *setvalue += encoder * encoderInkrement;
        if (*setvalue < minValue)
            *setvalue = minValue;
        if (*setvalue > maxValue)
            *setvalue = maxValue;
        /*********************************************************
         * enter main menu
         ********************************************************/
        if (button & HAL_BUTTON_ENTER) {
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
    char *entries[MENU_MAIN_MAX_ENRIES];
    uint8_t i;
    for (i = 0; i < menu.nentries; i++) {
        entries[i] = menu.entries[i].descr;
    }
    int8_t sel;
    do {
        sel = menu_ItemChooseDialog(
                "\xCD\xCD\xCD\xCD\xCD\xCDMAIN MENU\xCD\xCD\xCD\xCD\xCD\xCD",
                entries, menu.nentries);
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
 * press. Furthermore, it checks the inputvalue against
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
 * \param ndot      Number of digits behind decimal point (fixed integer input)
 * \return 0: input process aborted, 1: input parameter set
 */
uint8_t menu_getInputValue(uint32_t *value, char *descr, uint32_t min,
        uint32_t max, uint8_t ndot) {
    uint32_t inputValue;
    uint8_t valueValid = 0;
    do {
        while (hal_getButton())
            ;
        // display input mask
        screen_Clear();
        screen_FastString6x8("Input parameter:", 0, 0);
        screen_FastString6x8(descr, 0, 1);
        screen_Rectangle(1, 21, 126, 42);
        screen_Rectangle(2, 22, 125, 41);
        screen_FastString6x8("ESC: Abort", 0, 6);
        screen_FastString6x8("Enter: set parameter", 0, 7);
        uint32_t button;
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
            button = hal_getButton();
            if (inputPosition < 10) {
                // room left for input
                if ((dotPosition == 0xff
                        || inputPosition - dotPosition < ndot + 1)) {
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
                    } else if ((button & HAL_BUTTON_DOT) && dotPosition == 0xff
                            && ndot > 0) {
                        // dot button pressed and dot not set yet
                        dotPosition = inputPosition;
                        input[inputPosition++] = '.';
                        while (hal_getButton())
                            ;
                    }
                }
            }
            screen_FastString12x16(input, 4, 3);
        } while (!(button & (HAL_BUTTON_ESC | HAL_BUTTON_ENTER)));
        if (button & HAL_BUTTON_ESC)
            return 0;
        while (hal_getButton())
            ;
        // input buffer has been filled
        // -> transform to integer
        inputValue = 0;
        for (i = 0; i < 10; i++) {
            if (input[i] == 0) {
                // found end of input buffer
                if (ndot) {
                    // value is a fixed point integer
                    // calculate digits after dot in input
                    uint8_t afterDot = 0;
                    if (dotPosition != 0xff) {
                        afterDot = i - dotPosition - 1;
                    }
                    // shift value by remaining digits
                    uint8_t j;
                    for (j = 0; j < (ndot - afterDot); j++)
                        inputValue *= 10;
                }
                break;
            }
            if (input[i] >= '0' && input[i] <= '9') {
                // found a digit
                inputValue *= 10;
                inputValue += input[i] - '0';
            }
        }
        // check input value for boundaries
        if (inputValue > max || inputValue < min) {
            // value outside of boundaries
            // display error message
            screen_Clear();
            screen_FastString12x16("ERROR", 0, 0);
            screen_FastString6x8("Min. parameter:", 0, 2);
            char buf[11];
            string_fromUint(min, buf, 9, ndot);
            screen_FastString12x16(buf, 0, 3);
            screen_FastString6x8("Max. parameter:", 0, 5);
            string_fromUint(max, buf, 9, ndot);
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

int8_t menu_ItemChooseDialog(char *title, char **items, uint8_t nitems) {
    if (nitems == 0) {
        // no menu entries
        // -> no main menu
        return -1;
    }
    uint8_t selectedItem = 0;
    uint8_t firstDisplayedItem = 0;
    do {
        // wait for all buttons to be released
        while (hal_getButton())
            ;
        // display menu surroundings
        screen_Clear();
        screen_FastString6x8(title, 0, 0);
        screen_FastString6x8("Use \x18,\x19,ESC and Enter", 0, 7);
        // display menu entries
        uint8_t i;
        for (i = 0; i < 6 && i + firstDisplayedItem < nitems; i++) {
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
                if (selectedItem > firstDisplayedItem + 5)
                    firstDisplayedItem = selectedItem - 5;
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
