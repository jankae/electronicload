#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <string.h>
#include "common.h"
#include "menu.h"
#include "stringFunctions.h"

#define SETTINGS_NUM_ENTRIES        8

#define LOAD_MAXVOLTAGE_LOWP    100000000
#define LOAD_MINVOLTAGE_LOWP    100000
#define LOAD_MAXCURRENT_LOWP    200000
#define LOAD_MAXRESISTANCE_LOWP 99999999
#define LOAD_MINRESISTANCE_LOWP 1000
#define LOAD_MAXPOWER_LOWP      2000000

#define LOAD_MAXVOLTAGE_HIGHP    100000000
#define LOAD_MINVOLTAGE_HIGHP    100000
#define LOAD_MAXCURRENT_HIGHP    20000000
#define LOAD_MAXRESISTANCE_HIGHP 999999
#define LOAD_MINRESISTANCE_HIGHP 100
#define LOAD_MAXPOWER_HIGHP      200000000

#define SETTINGS_DEF_BAUDRATE   9600

struct {
    uint32_t baudrate;
    uint8_t powerMode;
    uint32_t maxCurrent[2];
    uint32_t maxPower[2];
    uint32_t minVoltage[2];
    uint32_t maxVoltage[2];
    uint32_t minResistance[2];
    uint32_t maxResistance[2];
} settings;

void settings_Init(void);

void settings_Menu(void);

void settings_SelectBaudrate(void);

#endif
