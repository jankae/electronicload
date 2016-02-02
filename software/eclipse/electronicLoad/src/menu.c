/**
 * \file
 * \brief Menu routines header file
 *
 * Contains menu and user interface functions
 */
#include "menu.h"

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
 *                  (will only be written to, if input was successful)
 * \param *descr    Pointer to a char array describing the input paramer
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
