
#ifndef GUI_ENTRY_H_
#define GUI_ENTRY_H_

#include "widget.h"
#include "screen.h"
#include "font.h"
#include "units.h"

#define ENTRY_INPUT_MAX_LENGTH      9

typedef struct {
    widget_t base;
    int32_t *value;
    int32_t *max;
    int32_t *min;
    font_t font;
    unit_t unit;
    uint8_t digits;
    void (*changeCallback)(void);

    struct {
        uint8_t editing :1;
        uint8_t dotSet :1;
        uint8_t encoderEdit :1;
    } flags;
    uint8_t encEditPos;
    char inputString[ENTRY_INPUT_MAX_LENGTH];
} entry_t;

void entry_create(entry_t *e, int32_t *value, int32_t *max, int32_t *min, font_t font, uint8_t digits, unit_t unit, void *cb);

uint32_t entry_GetInputStringValue(entry_t *e, uint32_t multiplier);
uint32_t entry_getIncrement(entry_t *e);
int32_t entry_constrainValue(entry_t *e, int32_t value);
void entry_setIncrement(entry_t *e, uint32_t inkrement);
GUIResult_t entry_draw(widget_t *w, coords_t offset);
GUISignal_t entry_input(widget_t *w, GUISignal_t signal);


#endif
