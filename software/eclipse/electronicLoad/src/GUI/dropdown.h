#ifndef GUI_DROPDOWN_H_
#define GUI_DROPDOWN_H_

#include "widget.h"
#include "screen.h"
#include "font.h"

typedef struct {
    widget_t base;
    uint8_t *value;
    char **itemlist;
    font_t font;
    void (*changeCallback)(void);
    uint8_t numItems;
    struct {
        uint8_t editing :1;
    } flags;
    /* selected position while editing */
    uint8_t editPos;
    /* first visible entry while viewing */
    uint8_t editFirstView;
} dropdown_t;

void dropdown_create(dropdown_t *d, char **items, uint8_t *value, font_t font,
        uint8_t minSize, void *cb);
GUIResult_t dropdown_draw(widget_t *w, coords_t offset);
GUISignal_t dropdown_input(widget_t *w, GUISignal_t signal);

#endif
