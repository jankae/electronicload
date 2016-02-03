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
    uint32_t *setvalue;
    uint32_t minValue;
    uint32_t maxValue;
    uint8_t encoderIndicator = 20;
    uint8_t encoderInkrement = 1;
    while (1) {
        // wait for all buttons to be released
        while (hal_getButton())
            ;
        // set default screen entries
        char setValBuf[11];
        if (!loadFunctions.powerOn) {
            screen_SetDefaultScreenString("!INPUT OFF!          ", 0, 0);
        } else {
            screen_SetDefaultScreenString("                     ", 0, 0);
        }
        screen_SetDefaultScreenString("\x19", encoderIndicator, 0);
        switch (loadFunctions.mode) {
        case FUNCTION_CC:
            screen_SetDefaultScreenString("CC-Mode [A]:  ", 0, 1);
            string_fromUint(loadFunctions.current, setValBuf, 6, 3);
            setvalue = &loadFunctions.current;
            minValue = 0;
            maxValue = LOAD_MAXCURRENT;
            break;
        case FUNCTION_CV:
            screen_SetDefaultScreenString("CC-Mode [V]:  ", 0, 1);
            string_fromUint(loadFunctions.voltage, setValBuf, 6, 3);
            setvalue = &loadFunctions.voltage;
            minValue = LOAD_MINVOLTAGE;
            maxValue = LOAD_MAXVOLTAGE;
            break;
        case FUNCTION_CR:
            screen_SetDefaultScreenString("CC-Mode [Ohm]:", 0, 1);
            string_fromUint(loadFunctions.resistance, setValBuf, 6, 3);
            setvalue = &loadFunctions.resistance;
            minValue = LOAD_MINRESISTANCE;
            maxValue = LOAD_MAXRESISTANCE;
            break;
        case FUNCTION_CP:
            screen_SetDefaultScreenString("CC-Mode [W]:  ", 0, 1);
            string_fromUint(loadFunctions.power, setValBuf, 6, 3);
            setvalue = &loadFunctions.power;
            minValue = 0;
            maxValue = LOAD_MAXPOWER;
            break;
        }
        screen_SetDefaultScreenString(setValBuf, 14, 1);
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
            if (menu_getInputValue(&loadFunctions.current, "'load current'", 0,
            LOAD_MAXCURRENT, 3)) {
                loadFunctions.mode = FUNCTION_CC;
            }
        }
        if (button & HAL_BUTTON_CV) {
            if (menu_getInputValue(&loadFunctions.voltage, "'load voltage'", 0,
            LOAD_MAXCURRENT, 3)) {
                loadFunctions.mode = FUNCTION_CV;
            }
        }
        if (button & HAL_BUTTON_CR) {
            if (menu_getInputValue(&loadFunctions.resistance,
                    "'load resistance'", 0,
                    LOAD_MAXCURRENT, 3)) {
                loadFunctions.mode = FUNCTION_CR;
            }
        }
        if (button & HAL_BUTTON_CP) {
            if (menu_getInputValue(&loadFunctions.power, "'load power'", 0,
            LOAD_MAXCURRENT, 3)) {
                loadFunctions.mode = FUNCTION_CP;
            }
        }
        /*********************************************************
         * load on/off
         ********************************************************/
        if (button & HAL_BUTTON_ONOFF) {
            // toggle on/off
            loadFunctions.powerOn ^= 1;
        }
        /*********************************************************
         * encoder inkrement setting
         ********************************************************/
        if (button & HAL_BUTTON_LEFT) {
            // move inkrement one position to the left, skip dot
            if (encoderIndicator > 14) {
                // encoder inkrement is not all the way to the left
                // -> move it
                encoderIndicator--;
                if (encoderIndicator == 17) {
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
                if (encoderIndicator == 17) {
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
    if (menu.nentries == 0) {
        // no menu entries
        // -> no main menu
        return;
    }
    uint8_t menuActive = 1;
    static uint8_t selectedEntry = 0;
    static uint8_t firstDisplayedEntry = 0;
    do {
        // wait for all buttons to be released
        while (hal_getButton())
            ;
        // display menu surroundings
        screen_Clear();
        screen_FastString6x8(
                "\xCD\xCD\xCD\xCD\xCD\xCDMAIN MENU\xCD\xCD\xCD\xCD\xCD\xCD", 0,
                0);
        screen_FastString6x8("Use |,|,ESC and Enter", 0, 7);
        // display menu entries
        uint8_t i;
        for (i = 0; i < 6 && i + firstDisplayedEntry < menu.nentries; i++) {
            screen_FastString6x8(menu.entries[i + firstDisplayedEntry].descr, 6,
                    i + 1);
        }
        // display arrow at selected menu entry
        screen_FastChar6x8(0, 1 + selectedEntry - firstDisplayedEntry, 0x1A);

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
            if (selectedEntry > 0) {
                selectedEntry--;
                // scroll if necessary
                if (selectedEntry < firstDisplayedEntry)
                    firstDisplayedEntry = selectedEntry;
            }
        }
        if ((button & HAL_BUTTON_DOWN) || encoder > 0) {
            // move entry selection one down
            if (selectedEntry < menu.nentries - 1) {
                selectedEntry++;
                // scroll if necessary
                if (selectedEntry > firstDisplayedEntry + 5)
                    firstDisplayedEntry = selectedEntry - 5;
            }
        }
        /*********************************************************
         * leaving menu
         ********************************************************/
        if (button & HAL_BUTTON_ESC) {
            menuActive = 0;
        }
        /*********************************************************
         * entering submenu
         ********************************************************/
        if (button & (HAL_BUTTON_ENTER | HAL_BUTTON_ENCODER)) {
            // call submenu function
            menu.entries[selectedEntry].menuFunction();
        }
    } while (menuActive);
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
            menu.entries[menu.nentries].descr[i] = *descr++;
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
        uint8_t inputPosition;
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
                    } else if ((button & HAL_BUTTON_DOT)
                            && dotPosition == 0xff) {
                        // dot button pressed and dot not set yet
                        dotPosition = inputPosition;
                        input[inputPosition++] = '.';
                    }
                }
            }
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
                        afterDot = i - dotPosition;
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
            uint32_t timeout = timer_SetTimeout(3000);
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
