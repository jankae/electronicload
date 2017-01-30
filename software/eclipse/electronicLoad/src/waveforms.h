#ifndef WAVEFORMS_H_
#define WAVEFORMS_H_

#include "GUI/gui.h"
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
    int32_t min;
    int32_t max;
    Waveform_t form;
    uint32_t period;
    uint32_t frequency;
    int32_t *param;
    uint8_t paramNum;
    uint16_t phase;
} waveform;

void waveform_Init(void);

widget_t* waveform_getWidget(void);

void waveform_Update(void);

void waveform_WaveChanged(void);

void waveform_ParamChanged(void);

void waveform_MinMaxChanged(void);

void waveform_AmplitudeOffsetChanged(void);

void waveform_PeriodChanged(void);

void waveform_FrequencyChanged(void);

int32_t waveform_GetValue(uint16_t wavetime);

int32_t waveform_Sine(uint16_t arg);

//void waveform_Menu(void);

#endif
