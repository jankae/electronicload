
#ifndef ARBITRARY_H_
#define ARBITRARY_H_

#include "screen.h"
#include "common.h"

#define ARB_MAX_POINTS      20
#define ARB_NUM_PARAMS      4

struct arbDataPoint {
    int32_t value;
    int32_t time;
    uint8_t hold;
};

struct {
    struct arbDataPoint points[ARB_MAX_POINTS];
    uint8_t numPoints;
    int32_t *param;
    uint8_t paramNum;
    uint8_t active;
} arbitrary;

void arb_Init(void);

int32_t arb_getValue(uint32_t time);

void arb_editSequence(void);

#endif /* ARBITRARY_H_ */
