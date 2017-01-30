#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <string.h>
#include "common.h"
#include "GUI/gui.h"
#include "stringFunctions.h"

// position of settings in flash memory (take care not to collide with calibration data)
#define FLASH_SETTINGS_DATA             0x0801E004
#define FLASH_VALID_SETTINGS_INDICATOR  0x0801E000

#define SETTINGS_INDICATOR              0x04

#define SETTINGS_NUM_ENTRIES            9

#define LOAD_MAXVOLTAGE_LOWP            100000000
#define LOAD_MINVOLTAGE_LOWP            100000
#define LOAD_MAXCURRENT_LOWP            200000
#define LOAD_MAXRESISTANCE_LOWP         99999999
#define LOAD_MINRESISTANCE_LOWP         5000
#define LOAD_MAXPOWER_LOWP              2000000

#define LOAD_MAXVOLTAGE_HIGHP           100000000
#define LOAD_MINVOLTAGE_HIGHP           100000
#define LOAD_MAXCURRENT_HIGHP           20000000
#define LOAD_MAXRESISTANCE_HIGHP        99999999
#define LOAD_MINRESISTANCE_HIGHP        100
#define LOAD_MAXPOWER_HIGHP             200000000

#define SETTINGS_DEF_BAUDRATE           2

struct {
    uint32_t baudrate;
    uint8_t powerMode;
    uint32_t maxCurrent;
    uint32_t maxPower;
    uint32_t minVoltage;
    uint32_t maxVoltage;
    uint32_t minResistance;
    uint32_t maxResistance;
    uint8_t turnOffOnError;
} settings;

void settings_Init(void);

widget_t* settings_getWidget(void);

uint8_t settings_readFromFlash(void);

void settings_writeToFlash(void);

void settings_UpdateBaudrate(void);

void settings_ChangedValue(void);

void settings_setDefaultLow(void);

void settings_setDefaultHigh(void);

//void settings_Menu(void);

//void settings_SelectBaudrate(void);

void settings_ResetToDefaultMenu(void);

void settings_LoadMenu(void);

void settings_SaveMenu(void);

#endif
