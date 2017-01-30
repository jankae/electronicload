#ifndef UNITS_H_
#define UNITS_H_

#include <string.h>

#define UNIT_NUM        5

typedef enum {
    UNIT_CURRENT, UNIT_VOLTAGE, UNIT_RESISTANCE, UNIT_POWER, UNIT_TIME
} unit_t;

extern const char unitNames[UNIT_NUM][3][3];

#endif
