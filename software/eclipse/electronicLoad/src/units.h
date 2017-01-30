#ifndef UNITS_H_
#define UNITS_H_

#include <string.h>

#define UNIT_NUM        7

typedef enum {
    UNIT_CURRENT, UNIT_VOLTAGE, UNIT_RESISTANCE, UNIT_POWER, UNIT_TIME, UNIT_FREQ, UNIT_ENERGY
} unit_t;

extern const char unitNames[UNIT_NUM][3][4];

#endif
