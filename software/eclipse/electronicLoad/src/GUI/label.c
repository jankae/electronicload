#include "label.h"

void label_createWithText(label_t *l, char *name, font_t font) {
    /* initialize common widget values */
    widget_init((widget_t*) l);
    /* set widget functions */
    l->base.func.draw = label_draw;
    l->base.func.input = label_input;
    l->font = font;
    uint8_t i = 0;
    while (*name && i < LABEL_MAX_NAME) {
        l->name[i++] = *name++;
    }
    l->name[i] = 0;
    l->base.flags.selectable = 0;
    /* calculate size */
    l->base.size.y = fontSize[font].height;
    l->base.size.x = fontSize[font].width * i;
}

void label_createWithLength(label_t *l, uint8_t length, font_t font) {
    /* initialize common widget values */
    widget_init((widget_t*) l);
    /* set widget functions */
    l->base.func.draw = label_draw;
    l->base.func.input = label_input;
    l->font = font;
    /* no text so far */
    l->name[0] = 0;
    l->base.flags.selectable = 0;
    /* calculate size */
    l->base.size.y = fontSize[font].height;
    l->base.size.x = fontSize[font].width * length;
}

void label_SetText(label_t *l, char *text) {
    uint8_t i = 0;
    while (*text && i * fontSize[l->font].width <= l->base.size.x
            && i < LABEL_MAX_NAME) {
        l->name[i++] = *text++;
    }
    l->name[i] = 0;
}

GUIResult_t label_draw(widget_t *w, coords_t offset) {
    label_t *l = (label_t*) w;
    screen_String(l->name, l->font, offset.x + l->base.position.x,
            offset.y + l->base.position.y);
    return GUI_OK;
}

GUISignal_t label_input(widget_t *w, GUISignal_t signal) {
    /* label doesn't handle any input */
    return signal;
}
