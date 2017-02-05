/**
 * \file
 * \brief   Statistics source file.
 *
 * Collects data during operation
 */
#include "statistics.h"

static container_t c;
static label_t lMin, lMax, lAvg, lEnergy;
static valueLabel_t eCurrentMin, eCurrentAvg, eCurrentMax;
static valueLabel_t eVoltageMin, eVoltageAvg, eVoltageMax;
static valueLabel_t ePowerMin, ePowerAvg, ePowerMax;
static valueLabel_t eEnergy;
static button_t bReset;

void stats_Init() {
    stats_Reset();

    /* build GUI */
    label_createWithText(&lMin, "MIN:", FONT_SMALL);
    label_createWithText(&lAvg, "AVG:", FONT_SMALL);
    label_createWithText(&lMax, "MAX:", FONT_SMALL);
    label_createWithText(&lEnergy, "ENERGY:", FONT_SMALL);

    valueLabel_create(&eCurrentMin, &stats.current.min, FONT_SMALL, 4,
            UNIT_CURRENT);
    valueLabel_create(&eCurrentAvg, &stats.current.avg, FONT_SMALL, 4,
            UNIT_CURRENT);
    valueLabel_create(&eCurrentMax, &stats.current.max, FONT_SMALL, 4,
            UNIT_CURRENT);

    valueLabel_create(&eVoltageMin, &stats.voltage.min, FONT_SMALL, 4,
            UNIT_VOLTAGE);
    valueLabel_create(&eVoltageAvg, &stats.voltage.avg, FONT_SMALL, 4,
            UNIT_VOLTAGE);
    valueLabel_create(&eVoltageMax, &stats.voltage.max, FONT_SMALL, 4,
            UNIT_VOLTAGE);

    valueLabel_create(&ePowerMin, &stats.power.min, FONT_SMALL, 4,
            UNIT_POWER);
    valueLabel_create(&ePowerAvg, &stats.power.avg, FONT_SMALL, 4,
            UNIT_POWER);
    valueLabel_create(&ePowerMax, &stats.power.max, FONT_SMALL, 4,
            UNIT_POWER);

    valueLabel_create(&eEnergy, &stats.energyConsumed, FONT_SMALL, 7,
            UNIT_ENERGY);

    button_create(&bReset, "Reset", FONT_MEDIUM, 0, stats_Reset);

    container_create(&c, 128, 55);

    container_attach(&c, &lMin, 0, 3);
    container_attach(&c, &lAvg, 0, 13);
    container_attach(&c, &lMax, 0, 23);
    container_attach(&c, &lEnergy, 0, 38);

    container_attach(&c, &eCurrentMin, 16, 1);
    container_attach(&c, &eCurrentAvg, 16, 11);
    container_attach(&c, &eCurrentMax, 16, 21);

    container_attach(&c, &eVoltageMin, 48, 1);
    container_attach(&c, &eVoltageAvg, 48, 11);
    container_attach(&c, &eVoltageMax, 48, 21);

    container_attach(&c, &ePowerMin, 80, 1);
    container_attach(&c, &ePowerAvg, 80, 11);
    container_attach(&c, &ePowerMax, 80, 21);

    container_attach(&c, &eEnergy, 32, 36);

    container_attach(&c, &bReset, 94, 43);
}

widget_t* stats_getWidget(void) {
    return (widget_t*) &c;
}

void stats_ResetValue(struct statStruct *s) {
    s->max = 0;
    s->min = UINT32_MAX;
    s->avg = 0;
    s->sum = 0;
    s->nsamples = 0;
}

void stats_UpdateValue(struct statStruct *s, uint32_t currentValue) {
    if (currentValue > s->max)
        s->max = currentValue;
    if (currentValue < s->min)
        s->min = currentValue;
    s->sum += currentValue;
    s->nsamples++;
    s->avg = s->sum / s->nsamples;
}

void stats_Reset(void) {
    stats_ResetValue(&stats.voltage);
    stats_ResetValue(&stats.current);
    stats_ResetValue(&stats.power);
    stats.energyConsumed = 0;
}

void stats_Update(void) {
// update min/max/avg values
    stats_UpdateValue(&stats.voltage, load.state.voltage);
    stats_UpdateValue(&stats.current, load.state.current);
    stats_UpdateValue(&stats.power, load.state.power);
// update consumed energy
    stats.energyConsumed = stats.power.sum / 3600000UL;
}
