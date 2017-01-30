#ifndef GUI_CONTAINER_H_
#define GUI_CONTAINER_H_

#include "widget.h"
#include "screen.h"
#include "frontPanel.h"

#define CONTAINER_SCROLLBAR_SIZE    3

typedef struct {
    widget_t base;
    coords_t canvasSize;
    coords_t viewingSize;
    coords_t canvasOffset;
    struct {
        uint8_t editing :1;
        uint8_t focussed :1;
        uint8_t scrollVertical :1;
        uint8_t scrollHorizontal :1;
    } flags;
} container_t;

void container_create(container_t *c, uint8_t width , uint8_t height);
GUIResult_t container_attach(container_t *c, widget_t *w, uint8_t x, uint8_t y);
GUIResult_t container_draw(widget_t *w, coords_t offset);
GUISignal_t container_input(widget_t *w, GUISignal_t signal);
void container_focussed(widget_t *w);

#endif
