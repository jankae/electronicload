#ifndef GUI_NOTEBOOK_H_
#define GUI_NOTEBOOK_H_

#include "widget.h"
#include "screen.h"
#include "font.h"

#define NOTEBOOK_MAX_PAGES          20

typedef struct {
    widget_t base;
    uint8_t selectedPage;
    uint32_t pageStartOffset;
    char *pageNames[NOTEBOOK_MAX_PAGES];
    font_t font;
    uint8_t numItems;
    struct {
        uint8_t editing :1;
        uint8_t focussed :1;
    } flags;
} notebook_t;

GUIResult_t notebook_create(notebook_t *n, font_t font, uint8_t x, uint8_t y);
GUIResult_t notebook_addPage(notebook_t *n, widget_t *page, char *name);
GUIResult_t notebook_draw(widget_t *w, coords_t offset);
GUISignal_t notebook_input(widget_t *w, GUISignal_t signal);

#endif
