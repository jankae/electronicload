#ifndef MAINMENU_H_
#define MAINMENU_H_

#include "GUI/gui.h"
#include "loadFunctions.h"

void mainMenu_Init();

widget_t* mainMenu_getWidget(void);

GUISignal_t mainMenu_Input(GUISignal_t signal);

void mainMenu_updateWidgets(void);

#endif
