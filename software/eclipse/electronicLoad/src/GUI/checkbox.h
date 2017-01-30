#ifndef GUI_CHECKBOX_H_
#define GUI_CHECKBOX_H_

#include "widget.h"
#include "screen.h"

typedef struct {
    widget_t base;
    void (*callback)(void);
    uint8_t *value;
    coords_t fontStart;
} checkbox_t;

void checkbox_create(checkbox_t *c, uint8_t *value, void *cb);
GUIResult_t checkbox_draw(widget_t *w, coords_t offset);
GUISignal_t checkbox_input(widget_t *w, GUISignal_t signal);

#endif
