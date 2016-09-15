#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "common.h"
#include "menu.h"
#include "stringFunctions.h"

#define SETTINGS_NUM_ENTRIES        5

#define LOAD_MAXVOLTAGE         100000000
#define LOAD_MINVOLTAGE         100000
#define LOAD_MAXCURRENT         20000000
#define LOAD_MAXRESISTANCE      999999
#define LOAD_MINRESISTANCE      50
#define LOAD_MAXPOWER           200000000

#define SETTINGS_DEF_BAUDRATE   9600

struct{
    uint32_t baudrate;
    uint32_t maxCurrent;
    uint32_t maxPower;
    uint32_t maxVoltage;
    uint32_t minResistance;
} settings;

void settings_Init(void);

void settings_Menu(void);

void settings_SelectBaudrate(void);

#endif
