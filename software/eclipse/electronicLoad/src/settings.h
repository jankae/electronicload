#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <string.h>
#include "common.h"
#include "menu.h"
#include "stringFunctions.h"

// position of settings in flash memory (take care not to collide with calibration data)
#define FLASH_SETTINGS_DATA             0x0801E004
#define FLASH_VALID_SETTINGS_INDICATOR  0x0801E000

#define SETTINGS_INDICATOR              0x02

#define SETTINGS_NUM_ENTRIES            10

#define LOAD_MAXVOLTAGE_LOWP            100000000
#define LOAD_MINVOLTAGE_LOWP            100000
#define LOAD_MAXCURRENT_LOWP            200000
#define LOAD_MAXRESISTANCE_LOWP         99999999
#define LOAD_MINRESISTANCE_LOWP         5000
#define LOAD_MAXPOWER_LOWP              2000000

#define LOAD_MAXVOLTAGE_HIGHP           100000000
#define LOAD_MINVOLTAGE_HIGHP           100000
#define LOAD_MAXCURRENT_HIGHP           20000000
#define LOAD_MAXRESISTANCE_HIGHP        999999
#define LOAD_MINRESISTANCE_HIGHP        100
#define LOAD_MAXPOWER_HIGHP             200000000

#define SETTINGS_DEF_BAUDRATE           9600

typedef enum {
    CONTROL_ANALOG = 0, CONTROL_DIGITAL = 1
} controlMode_t;

struct {
    uint32_t baudrate;
    uint8_t powerMode;
    uint32_t maxCurrent[2];
    uint32_t maxPower[2];
    uint32_t minVoltage[2];
    uint32_t maxVoltage[2];
    uint32_t minResistance[2];
    uint32_t maxResistance[2];
    controlMode_t powerControl;
    controlMode_t resistanceControl;
} settings;

void settings_Init(void);

uint8_t settings_readFromFlash(void);

void settings_writeToFlash(void);

void settings_Menu(void);

void settings_SelectBaudrate(void);

void settings_ResetToDefaultMenu(void);

void settings_LoadMenu(void);

void settings_SaveMenu(void);

#endif
