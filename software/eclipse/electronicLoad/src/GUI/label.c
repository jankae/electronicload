#include "label.h"

void label_create(label_t *l, char *name, font_t font) {
    /* initialize common widget values */
    widget_init((widget_t*) l);
    /* set widget functions */
    l->base.func.draw = label_draw;
    l->base.func.input = label_input;
    l->font = font;
    l->name = name;
    /* calculate size */
    l->base.size.y = fontSize[font].height;
    l->base.size.x = fontSize[font].width * strlen(name);
}

GUIResult_t label_draw(widget_t *w, coords_t offset) {
    label_t *l = (label_t*) w;
    screen_String(l->name, l->font, offset.x, offset.y);
    return GUI_OK;
}

GUISignal_t label_input(widget_t *w, GUISignal_t signal) {
    /* label doesn't handle any input */
    return signal;
}
