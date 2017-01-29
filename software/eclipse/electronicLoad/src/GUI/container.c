#include "container.h"

void container_create(container_t *c, uint8_t width, uint8_t height) {
    widget_init((widget_t*) c);
    c->base.size.x = width;
    c->base.size.y = height;
    c->base.func.draw = container_draw;
    c->base.func.input = container_input;
    c->flags.editing = 0;
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
    }
    w->position.x = x;
    w->position.y = y;
    w->parent = (widget_t*) c;
    return GUI_OK;
}

GUIResult_t container_draw(widget_t *w, coords_t offset) {
    /* container itself is invisible */
    GUIResult_t res = GUI_OK;
    offset.x += w->position.x;
    offset.y += w->position.y;
    /* draw its children */
    widget_t *child = w->firstChild;
    widget_t *selected = NULL;
    for (; child; child = child->next) {
        if (res != GUI_OK) {
            /* abort on error */
            break;
        }
        if (child->flags.visible) {
            if(child->flags.selected) {
                /* store this child for later drawing */
                selected = child;
            } else {
                /* draw this child */
                res = child->func.draw(child, offset);
            }
        }
    }
    /* always draw selected child last (might overwrite other children) */
    if(selected) {
        selected->func.draw(selected, offset);
    }
    return res;
}

GUISignal_t container_input(widget_t *w, GUISignal_t signal) {
    container_t *c = (container_t*) w;
    if (c->flags.editing) {
        if (signal.encoder < 0) {
            /* move active item one up */
            widget_selectPrevious(c->base.firstChild);
        } else if (signal.encoder > 0) {
            /* move active item one down */
            widget_selectNext(c->base.firstChild);
        }
        signal.encoder = 0;
        if(signal.clicked & HAL_BUTTON_ESC) {
            /* leave editing mode */
            c->flags.editing = 0;
            widget_deselectAll(c->base.firstChild);
            signal.clicked &= ~HAL_BUTTON_ESC;
        }
    } else {
        /* not in editing mode */
        if(signal.clicked & HAL_BUTTON_ENTER) {
            /* start editing mode */
            if(widget_selectFirst(c->base.firstChild)==GUI_OK) {
                c->flags.editing = 1;
            }
            signal.clicked &= ~HAL_BUTTON_ENTER;
        }
    }
    return signal;
}
