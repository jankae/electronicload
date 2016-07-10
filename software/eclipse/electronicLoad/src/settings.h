#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "common.h"
#include "menu.h"
#include "stringFunctions.h"

#define SETTINGS_NUM_ENTRIES        1

struct{
    uint32_t baudrate;
} settings;

void settings_Menu(void);

void settings_SelectBaudrate(void);

#endif
