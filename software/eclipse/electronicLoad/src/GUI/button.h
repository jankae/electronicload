#ifndef GUI_BUTTON_H_
#define GUI_BUTTON_H_

#include "widget.h"
#include "screen.h"
#include "font.h"

typedef struct {
    widget_t base;
    void (*callback)(void);
    char *name;
    font_t font;
    coords_t fontStart;
} button_t;

void button_create(button_t *button, char *name, font_t font, uint8_t minWidth, void *cb);
GUIResult_t button_draw(widget_t *w, coords_t offset);
GUISignal_t button_input(widget_t *w, GUISignal_t signal);

#endif
