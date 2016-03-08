/**
 * \file
 * \brief   Statistics header file.
 *
 * Collects data during operation
 */
#ifndef STATISTICS_H_
#define STATISTICS_H_

#include <stdint.h>
#include "loadFunctions.h"

struct statStruct {
    uint32_t min, max, avg;
    uint64_t sum;
    uint32_t nsamples;
};

struct {
    // everything in mV
    struct statStruct voltage;
    // everyting in mA
    struct statStruct current;
    // everything in mW
    struct statStruct power;
    // mWh
    uint32_t energyConsumed;
} stats;

void stats_ResetValue(struct statStruct *s);
void stats_UpdateValue(struct statStruct *s, uint32_t currentValue);

void stats_Reset(void);
void stats_Update(void);

void stats_Display(void);

#endif
