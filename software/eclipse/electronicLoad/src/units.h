#ifndef UNITS_H_
#define UNITS_H_

#define UNIT_NUM        4

typedef enum {UNIT_CURRENT, UNIT_VOLTAGE, UNIT_RESISTANCE, UNIT_POWER} unit_t;

extern const char unitNames[UNIT_NUM][3][3];

#endif
