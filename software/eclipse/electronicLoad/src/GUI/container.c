#include "container.h"

void container_create(container_t *c, uint8_t width, uint8_t height) {
    widget_init((widget_t*) c);
    c->base.size.x = width;
    c->base.size.y = height;
    c->base.func.draw = container_draw;
    c->base.func.input = container_input;
}

GUIResult_t container_attach(container_t *c, widget_t *w, uint8_t x, uint8_t y) {
    /* check whether there is space for the widget */
    if (x + w->size.x > c->base.size.x || y + w->size.y > c->base.size.y) {
        /* widget extends over the edges of the container */
        return GUI_ERROR;
    }
    if (c->base.firstChild) {
        /* find end of children list */
        widget_t *child = c->base.firstChild;
        while (child->next) {
            child = child->next;
        }
        /* add widget to the end */
        child->next = w;
    } else {
        /* this is the first child */
        c->base.firstChild = w;
        /* this item has to be active */
        w->flags.active = 1;
    }
    w->position.x = x;
    w->position.y = y;
    w->parent = (widget_t*) c;
    return GUI_OK;
}

GUIResult_t container_draw(widget_t *w, coords_t offset) {
    /* container itself is invisible */
    return GUI_OK;
}

GUISignal_t container_input(widget_t *w, GUISignal_t signal) {
    container_t *c = (container_t*) w;
    if (signal.encoder < 0) {
        /* move active item one up */
        if (c->base.firstChild) {
            /* find current active item */
            widget_t *w = c->base.firstChild;
            if (w->flags.active == 0) {
                /* the first item is inactive */
                /* loop over all items until the active one is found */
                while (w->next) {
                    if (w->next->flags.active) {
                        /* the next item is active */
                        /* set current item active */
                        w->flags.active = 1;
                        /* set next item inactive */
                        w->next->flags.active = 0;
                        break;
                    }
                    w = w->next;
                }
                if (!w->next) {
                    /* no item was active -> shouldn't happen */
                    /* set first item active */
                    c->base.firstChild->flags.active = 1;
                }
            }
        }
    } else if (signal.encoder > 0) {
        /* move active item one down */
        if (c->base.firstChild) {
            /* find current active item */
            widget_t *w = c->base.firstChild;
            while (w) {
                if (w->flags.active) {
                    /* this item is active */
                    if (w->next) {
                        /* set next item active */
                        w->next->flags.active = 1;
                        /* set current item inactive */
                        w->flags.active = 0;
                    }
                    break;
                }
                w = w->next;
            }
            if (!w) {
                /* no item was active -> shouldn't happen */
                /* set first item active */
                c->base.firstChild->flags.active = 1;
            }
        }
    }
    signal.encoder = 0;
    return signal;
}
