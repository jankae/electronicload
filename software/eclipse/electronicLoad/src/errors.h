#ifndef ERRORS_H_
#define ERRORS_H_

#include "loadFunctions.h"

// time in ms after which an error condition is reported (max 254ms)
#define LOAD_MAX_ERROR_DURATION 200
// error bits encoded into load.error:
// load draws current while input is switched off
#define LOAD_ERROR_OFF_CURRENT              0x01
// load drawing wrong amount of current (voltage/power)
#define LOAD_ERROR_WRONG_CURRENT            0x02
#define LOAD_ERROR_WRONG_VOLTAGE            0x03
#define LOAD_ERROR_WRONG_POWER              0x04
// load draws more than maximum current
#define LOAD_ERROR_OVERCURRENT              0x05

struct {
    uint32_t code;
    uint8_t Duration[32];
} error;

void error_Menu(void);

void errors_Check(void);

#endif
