/**
 * \file
 * \brief Menu routines header file
 *
 * Contains menu and user interface functions
 */
#ifndef SYSTEM_MENU_H_
#define SYSTEM_MENU_H_

#include "screen.h"
#include "loadFunctions.h"

#define MENU_MAIN_MAX_ENRIES       10

struct mainMenuEntry {
    char descr[21];
    void (*menuFunction)();
};

struct {
    uint8_t nentries;
    struct mainMenuEntry entries[MENU_MAIN_MAX_ENRIES];
} menu;

/**
 * \brief Handles user inputs while the default screen is active
 *
 * Functionality includes: displaying selected mode and value,
 * switching load on/off, switching between standard modes (CC,
 * CV, CR, CP) and handling encoder inputs (changes the value
 * correspondent to the selected mode)
 */
void menu_DefaultScreenHandler(void);

/**
 * \brief Displays the main menu and handles user inputs
 *
 * Displays the main menu with all entries registered using
 * menu_AddMainMenuEntry(). The user can move through the menu
 * by using the UP and DOWN buttons or turning the encoder.
 * A submenu item is entered by pressing ENTER or the encoder.
 */
void menu_MainMenu(void);

/**
 * \brief Adds an entry to the main menu
 *
 * \param *descr        Name of the menu entry (up to 20 chararcters)
 * \param *menuFunction Pointer to function that will be called upon
 *                      selecting the menu entry
 */
void menu_AddMainMenuEntry(char *descr, void (*menuFunction)());

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
 * \return 0: input process aborted, 1: input parameter set
 */
uint8_t menu_getInputValue(uint32_t *value, char *descr, uint32_t min, uint32_t max,
        const char *unit1e0, const char *unit1e3, const char *unit1e6);

/**
 * \brief Allows the user to choose an item out of a list
 *
 * It displays the chooser dialog and handles every button
 * press.
 *
 * \param *title Title of the menu being displayed
 * \param **items List of pointers to char array describing the items
 * \param nitems Number of items
 *
 * \return item number if item was chosen, otherwise -1
 */
int8_t menu_ItemChooseDialog(char *title, char **items, uint8_t nitems);

#endif /* SYSTEM_MENU_H_ */
