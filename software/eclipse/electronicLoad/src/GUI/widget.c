#include "widget.h"

void widget_init(widget_t *w) {
    memset(w, 0, sizeof(widget_t));
    w->flags.visible = 1;
}

GUIResult_t widget_draw(widget_t *w, coords_t parentOffset) {
    GUIResult_t res;
    parentOffset.x += w->position.x;
    parentOffset.y += w->position.y;
    /* first, draw this widget */
    res = w->func.draw(w, parentOffset);
    /* then draw its children */
//    parentOffset.x += w->childrenOffset.x;
//    parentOffset.y += w->childrenOffset.y;
    widget_t *child = w->firstChild;
    for (; child; child = child->next) {
        if (res != GUI_OK) {
            /* abort on error */
            break;
        }
        if (child->flags.visible) {
            /* draw this child */
            res = widget_draw(child, parentOffset);
        }
    }
    return res;
}

GUISignal_t widget_input(widget_t *w, GUISignal_t signal) {
    /* First pass it on to any active child */
    widget_t *child = w->firstChild;
    for (; child; child = child->next) {
        if (child->flags.active) {
            signal = widget_input(child, signal);
        }
    }
    /* Then try to handle the signal itself */
    signal = w->func.input(w, signal);
    return signal;
}
