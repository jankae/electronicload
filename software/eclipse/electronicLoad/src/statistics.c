/**
 * \file
 * \brief   Statistics source file.
 *
 * Collects data during operation
 */
#include "statistics.h"

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

void stats_Display(void) {
    uint32_t button;
    do {
        char voltageString[22];
        char currentString[22];
        char powerString[22];
        char energyString[9];

        string_fromUint(stats.voltage.min, &voltageString[0], 5, 3);
        voltageString[6] = '/';
        string_fromUint(stats.voltage.avg, &voltageString[7], 5, 3);
        voltageString[13] = '/';
        string_fromUint(stats.voltage.max, &voltageString[14], 5, 3);

        string_fromUint(stats.current.min, &currentString[0], 5, 3);
        currentString[6] = '/';
        string_fromUint(stats.current.avg, &currentString[7], 5, 3);
        currentString[13] = '/';
        string_fromUint(stats.current.max, &currentString[14], 5, 3);

        string_fromUint(stats.power.min / 10, &powerString[0], 5, 2);
        powerString[6] = '/';
        string_fromUint(stats.power.avg / 10, &powerString[7], 5, 2);
        powerString[13] = '/';
        string_fromUint(stats.power.max / 10, &powerString[14], 5, 2);

        string_fromUint(stats.energyConsumed, energyString, 7, 3);

        screen_Clear();
        screen_FastString6x8("Voltage(min/avg/max):", 0, 0);
        screen_FastString6x8(voltageString, 0, 1);
        screen_FastString6x8("Current(min/avg/max):", 0, 2);
        screen_FastString6x8(currentString, 0, 3);
        screen_FastString6x8("Power(min/avg/max):", 0, 4);
        screen_FastString6x8(powerString, 0, 5);
        screen_FastString6x8("Energy[Wh]:", 0, 6);
        screen_FastString6x8(energyString, 66, 6);

        screen_FastString6x8("CC: reset stats", 6, 7);

        uint32_t time = timer_SetTimeout(1000);
        do {
            button = hal_getButton();
        } while (!button && !timer_TimeoutElapsed(time));
        if(button&HAL_BUTTON_CC)
            stats_Reset();

    } while (!(button & HAL_BUTTON_ESC));
}
