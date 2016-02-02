/**
 * \file
 * \brief Menu routines header file
 *
 * Contains menu and user interface functions
 */
#ifndef SYSTEM_MENU_H_
#define SYSTEM_MENU_H_

#include "screen.h"

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
        uint32_t max, uint8_t ndot);

#endif /* SYSTEM_MENU_H_ */
