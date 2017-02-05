
#ifndef GUI_VALUELABEL_H_
#define GUI_VALUELABEL_H_

#include "widget.h"
#include "screen.h"
#include "font.h"
#include "units.h"

typedef struct {
    widget_t base;
    int32_t *value;
    font_t font;
    unit_t unit;
    uint8_t digits;
} valueLabel_t;

void valueLabel_create(valueLabel_t *e, int32_t *value, font_t font, uint8_t digits, unit_t unit);
GUIResult_t valueLabel_draw(widget_t *w, coords_t offset);
GUISignal_t valueLabel_input(widget_t *w, GUISignal_t signal);


#endif
