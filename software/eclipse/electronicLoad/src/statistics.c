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
    // 0: max
    // 1: min
    // 2: average
    // 3: Energy consumed
    uint8_t mode = 0;
    do {
        // display statistic information
        screen_Clear();
        screen_SetSoftButton("Reset", 1);
        if (mode < 3) {
            uint32_t voltage;
            uint32_t current;
            uint32_t power;
            switch (mode) {
            case 0:
                voltage = stats.voltage.max;
                current = stats.current.max;
                power = stats.power.max;
                screen_SetSoftButton("Min", 0);
                screen_FastString6x8("Max.", 0, 0);
                break;
            case 1:
                voltage = stats.voltage.min;
                current = stats.current.min;
                power = stats.power.min;
                screen_SetSoftButton("Avg", 0);
                screen_FastString6x8("Min.", 0, 0);
                break;
            case 2:
                voltage = stats.voltage.avg;
                current = stats.current.avg;
                power = stats.power.avg;
                screen_SetSoftButton("Max", 0);
                screen_FastString6x8("Avg.", 0, 0);
                break;
            }
            screen_FastString6x8("value", 0, 1);
            char buf[15];
            string_fromUintUnit(voltage, buf, 4, 6, 'V');
            screen_FastString12x16(buf, 40, 0);
            string_fromUintUnit(current, buf, 4, 6, 'A');
            screen_FastString12x16(buf, 40, 2);
            string_fromUintUnit(power, buf, 4, 6, 'W');
            screen_FastString12x16(buf, 40, 4);
            screen_SetSoftButton("Wh", 2);
        } else {
            char buf[15];
            string_fromUintUnit(stats.energyConsumed, buf, 6, 6, 'W');
            screen_FastString6x8("Consumed energy:", 0, 0);
            screen_FastString12x16(buf, 0, 2);
            screen_FastChar12x16(108, 2, 'h');
            screen_SetSoftButton("Max", 0);
        }

        while(hal_getButton());
        // wait for 500ms or until a button is pressed
        uint8_t i;
        for (i = 0; i < 50; i++) {
            timer_waitms(10);
            button = hal_getButton();
            if (button & HAL_BUTTON_ESC) {
                break;
            }
            if (button & HAL_BUTTON_SOFT1) {
                stats_Reset();
                break;
            }
            if ((button & HAL_BUTTON_SOFT2) && mode < 3) {
                mode = 3;
                break;
            }
            if (button & HAL_BUTTON_SOFT0) {
                if (mode == 3)
                    mode = 0;
                else
                    mode = (mode + 1) % 3;
                break;
            }
        }
    } while (!(button & HAL_BUTTON_ESC));
}
