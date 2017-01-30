/**
 * \file
 * \brief   Statistics header file.
 *
 * Collects data during operation
 */
#ifndef STATISTICS_H_
#define STATISTICS_H_

#include <stdint.h>
#include "GUI/gui.h"
#include "loadFunctions.h"

struct statStruct {
    uint32_t min, max, avg;
    uint64_t sum;
    uint32_t nsamples;
};

struct {
    struct statStruct voltage;
    struct statStruct current;
    struct statStruct power;
    uint32_t energyConsumed;
} stats;

void stats_Init();

widget_t* stats_getWidget(void);

void stats_ResetValue(struct statStruct *s);
void stats_UpdateValue(struct statStruct *s, uint32_t currentValue);

void stats_Reset(void);
void stats_Update(void);

#endif
