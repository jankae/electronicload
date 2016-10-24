#ifndef WAVEFORMS_H_
#define WAVEFORMS_H_

#include "screen.h"

typedef enum {
    WAVE_NONE = 0,
    WAVE_SINE = 1,
    WAVE_SAW = 2,
    WAVE_SQUARE = 3,
    WAVE_TRIANGLE = 4
} Waveform_t;

struct {
    int32_t offset;
    int32_t amplitude;
    Waveform_t form;
    uint16_t period;
    int32_t *param;
    uint8_t paramNum;
    uint16_t phase;
} waveform;

void waveform_Init(void);

void waveform_Update(void);

int32_t waveform_GetValue(uint16_t wavetime);

int32_t waveform_Sine(uint16_t arg);

void waveform_Menu(void);

#endif
