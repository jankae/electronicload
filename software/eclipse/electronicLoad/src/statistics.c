/**
 * \file
 * \brief   Statistics source file.
 *
 * Collects data during operation
 */
#include "statistics.h"

static container_t c;
static label_t lMin, lMax, lAvg, lEnergy;
static entry_t eCurrentMin, eCurrentAvg, eCurrentMax;
static entry_t eVoltageMin, eVoltageAvg, eVoltageMax;
static entry_t ePowerMin, ePowerAvg, ePowerMax;
static entry_t eEnergy;
static button_t bReset;

void stats_Init() {
    stats_Reset();

    /* build GUI */
    label_create(&lMin, "MIN:", FONT_SMALL);
    label_create(&lAvg, "AVG:", FONT_SMALL);
    label_create(&lMax, "MAX:", FONT_SMALL);
    label_create(&lEnergy, "ENERGY:", FONT_SMALL);

    entry_create(&eCurrentMin, &stats.current.min, NULL, NULL, FONT_SMALL, 4,
            UNIT_CURRENT, NULL);
    entry_create(&eCurrentAvg, &stats.current.avg, NULL, NULL, FONT_SMALL, 4,
            UNIT_CURRENT, NULL);
    entry_create(&eCurrentMax, &stats.current.max, NULL, NULL, FONT_SMALL, 4,
            UNIT_CURRENT, NULL);

    entry_create(&eVoltageMin, &stats.voltage.min, NULL, NULL, FONT_SMALL, 4,
            UNIT_VOLTAGE, NULL);
    entry_create(&eVoltageAvg, &stats.voltage.avg, NULL, NULL, FONT_SMALL, 4,
            UNIT_VOLTAGE, NULL);
    entry_create(&eVoltageMax, &stats.voltage.max, NULL, NULL, FONT_SMALL, 4,
            UNIT_VOLTAGE, NULL);

    entry_create(&ePowerMin, &stats.power.min, NULL, NULL, FONT_SMALL, 4,
            UNIT_POWER, NULL);
    entry_create(&ePowerAvg, &stats.power.avg, NULL, NULL, FONT_SMALL, 4,
            UNIT_POWER, NULL);
    entry_create(&ePowerMax, &stats.power.max, NULL, NULL, FONT_SMALL, 4,
            UNIT_POWER, NULL);

    entry_create(&eEnergy, &stats.energyConsumed, NULL, NULL, FONT_SMALL, 7,
            UNIT_ENERGY, NULL);

    /* all entries are read-only */
    // TODO replace entries with something like a textfield widget */
    eCurrentMin.base.flags.selectable = 0;
    eCurrentAvg.base.flags.selectable = 0;
    eCurrentMax.base.flags.selectable = 0;
    eVoltageMin.base.flags.selectable = 0;
    eVoltageAvg.base.flags.selectable = 0;
    eVoltageMax.base.flags.selectable = 0;
    ePowerMin.base.flags.selectable = 0;
    ePowerAvg.base.flags.selectable = 0;
    ePowerMax.base.flags.selectable = 0;
    eEnergy.base.flags.selectable = 0;

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
