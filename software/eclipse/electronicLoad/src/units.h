#ifndef UNITS_H_
#define UNITS_H_

#include <string.h>

#define UNIT_NUM        6

typedef enum {
    UNIT_CURRENT, UNIT_VOLTAGE, UNIT_RESISTANCE, UNIT_POWER, UNIT_TIME, UNIT_FREQ
} unit_t;

extern const char unitNames[UNIT_NUM][3][3];

#endif
