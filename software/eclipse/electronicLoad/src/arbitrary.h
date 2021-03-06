
#ifndef ARBITRARY_H_
#define ARBITRARY_H_

#include "screen.h"
#include "common.h"

#define ARB_MAX_POINTS      20
#define ARB_NUM_PARAMS      4

typedef enum {
    ARB_SINGLE_SHOT = 0,
    ARB_CONTINUOUS = 1
} ArbMode_t;

typedef enum {
    ARB_DISABLED = 0,
    ARB_ARMED = 1,
    ARB_RUNNING = 2
} ArbStatus_t;

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
    uint32_t sequenceLength;
    ArbStatus_t status;
    ArbMode_t mode;
    uint32_t time;
} arbitrary;

void arb_Init(void);

int32_t arb_getValue(uint32_t time);

void arb_Update(void);

void arb_Menu(void);

void arb_AdjustPointsToLength(void);

void arb_editSequence(void);

#endif /* ARBITRARY_H_ */
