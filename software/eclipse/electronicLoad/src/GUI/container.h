#ifndef GUI_CONTAINER_H_
#define GUI_CONTAINER_H_

#include "widget.h"
#include "frontPanel.h"

typedef struct {
    widget_t base;
    struct {
        uint8_t editing :1;
    } flags;
} container_t;

void container_create(container_t *c, uint8_t width , uint8_t height);
GUIResult_t container_attach(container_t *c, widget_t *w, uint8_t x, uint8_t y);
GUIResult_t container_draw(widget_t *w, coords_t offset);
GUISignal_t container_input(widget_t *w, GUISignal_t signal);

#endif
