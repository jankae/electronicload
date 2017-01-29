
#ifndef GUI_LABEL_H_
#define GUI_LABEL_H_

#include "widget.h"
#include "screen.h"
#include "font.h"

typedef struct {
    widget_t base;
    char *name;
    font_t font;
} label_t;

void label_create(label_t *l, char *name, font_t font);
GUIResult_t label_draw(widget_t *w, coords_t offset);
GUISignal_t label_input(widget_t *w, GUISignal_t signal);

#endif