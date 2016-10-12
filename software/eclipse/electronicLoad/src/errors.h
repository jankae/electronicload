#ifndef ERRORS_H_
#define ERRORS_H_

#include "loadFunctions.h"

// time in ms after which an error condition is reported
#define LOAD_MAX_ERROR_DURATION 10
// error bits encoded into load.error:
// load draws current while input is switched off
#define LOAD_ERROR_OFF_CURRENT              0x01
// load drawing wrong amount of current (voltage/power)
#define LOAD_ERROR_WRONG_CURRENT            0x02
#define LOAD_ERROR_WRONG_VOLTAGE            0x04
#define LOAD_ERROR_WRONG_POWER              0x08

struct {
    uint32_t code;
} error;

void errors_Check(void);

#endif
