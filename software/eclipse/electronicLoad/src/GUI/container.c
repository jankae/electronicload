#include "container.h"

void container_create(container_t *c, uint8_t width, uint8_t height) {
    widget_init((widget_t*) c);
    c->base.size.x = width;
    c->base.size.y = height;
    c->base.func.draw = container_draw;
    c->base.func.input = container_input;
    c->flags.editing = 0;
    c->flags.focussed = 0;
    c->flags.scrollHorizontal = 0;
    c->flags.scrollVertical = 0;
    c->canvasSize.x = 0;
    c->canvasSize.y = 0;
    c->canvasOffset.x = 0;
    c->canvasOffset.y = 0;
    c->viewingSize = c->base.size;
}

GUIResult_t container_attach(container_t *c, widget_t *w, uint8_t x, uint8_t y) {
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

    /* extend canvas size if necessary */
    if (c->canvasSize.x < x + w->size.x) {
        c->canvasSize.x = x + w->size.x;
    }
    if (c->canvasSize.y < y + w->size.y) {
        c->canvasSize.y = y + w->size.y;
    }
    /* add scroll bars if necessary */
    if (c->canvasSize.x > c->base.size.x) {
        c->flags.scrollHorizontal = 1;
        c->viewingSize.y = c->base.size.y - CONTAINER_SCROLLBAR_SIZE;
    }
    if (c->canvasSize.y > c->viewingSize.y) {
        c->flags.scrollVertical = 1;
        c->viewingSize.x = c->base.size.x - CONTAINER_SCROLLBAR_SIZE;
        if (!c->flags.scrollHorizontal) {
            /* check again for horizontal scroll */
            if (c->canvasSize.x > c->viewingSize.x) {
                c->flags.scrollHorizontal = 1;
                c->viewingSize.y = c->base.size.y - CONTAINER_SCROLLBAR_SIZE;
            }
        }
    }
    return GUI_OK;
}

GUIResult_t container_draw(widget_t *w, coords_t offset) {
    container_t *c = (container_t*) w;
    GUIResult_t res = GUI_OK;
    offset.x += w->position.x;
    offset.y += w->position.y;
    widget_t *child = w->firstChild;
    widget_t *selected = child;
    for (; selected; selected = selected->next) {
        if (selected->flags.selected)
            break;
    }
    if (selected) {
        /* move canvas offset so that selected is visible */
        if (selected->position.x < c->canvasOffset.x) {
            c->canvasOffset.x = selected->position.x;
        } else if (selected->position.x + selected->size.x
                > c->viewingSize.x + c->canvasOffset.x) {
            c->canvasOffset.x = selected->position.x + selected->size.x
                    - c->viewingSize.x;
        }
        if (selected->position.y < c->canvasOffset.y) {
            c->canvasOffset.y = selected->position.y;
        } else if (selected->position.y + selected->size.y
                > c->viewingSize.y + c->canvasOffset.y) {
            c->canvasOffset.y = selected->position.y + selected->size.y
                    - c->viewingSize.y;
        }
    }
    /* draw scroll bars if necessary */
    if (c->flags.scrollVertical) {
        screen_VerticalLine(
                offset.x + c->base.size.x - CONTAINER_SCROLLBAR_SIZE, offset.y,
                c->base.size.y);
        /* calculate beginning and end of scrollbar */
        uint8_t scrollBegin = common_Map(c->canvasOffset.y, 0, c->canvasSize.y,
                0,
                c->base.size.y
                        - CONTAINER_SCROLLBAR_SIZE * c->flags.scrollHorizontal);
        uint8_t scrollEnd = common_Map(c->canvasOffset.y + c->viewingSize.y, 0,
                c->canvasSize.y, 0,
                c->base.size.y
                        - CONTAINER_SCROLLBAR_SIZE * c->flags.scrollHorizontal);
        /* display position indicator */
        screen_FullRectangle(
                offset.x + c->base.size.x - CONTAINER_SCROLLBAR_SIZE + 1,
                offset.y + scrollBegin, offset.x + c->base.size.x - 1,
                offset.y + scrollEnd - 1, PIXEL_ON);
    }
    if (c->flags.scrollHorizontal) {
        screen_HorizontalLine(offset.x,
                offset.y + c->base.size.y - CONTAINER_SCROLLBAR_SIZE,
                c->base.size.x);
        /* calculate beginning and end of scrollbar */
        uint8_t scrollBegin = common_Map(c->canvasOffset.x, 0, c->canvasSize.x,
                0,
                c->base.size.x
                        - CONTAINER_SCROLLBAR_SIZE * c->flags.scrollVertical);
        uint8_t scrollEnd = common_Map(c->canvasOffset.x + c->viewingSize.x, 0,
                c->canvasSize.x, 0,
                c->base.size.x
                        - CONTAINER_SCROLLBAR_SIZE * c->flags.scrollVertical);
        /* display position indicator */
        screen_FullRectangle(offset.x + scrollBegin,
                offset.y + c->base.size.y - CONTAINER_SCROLLBAR_SIZE + 1,
                offset.x + scrollEnd - 1, offset.y + c->base.size.y - 1,
                PIXEL_ON);
    }

    offset.x -= c->canvasOffset.x;
    offset.y -= c->canvasOffset.y;

    /* draw its children */
    for (; child; child = child->next) {
        if (res != GUI_OK) {
            /* abort on error */
            break;
        }
        if (child->flags.visible && !child->flags.selected) {
            /* check if child is fully in viewing field */
            if (child->position.x >= c->canvasOffset.x
                    && child->position.y >= c->canvasOffset.y
                    && child->position.x + child->size.x
                            <= c->canvasOffset.x + c->viewingSize.x
                    && child->position.y + child->size.y
                            <= c->canvasOffset.y + c->viewingSize.y) {
                /* draw this child */
                res = child->func.draw(child, offset);
            }
        }
    }
    /* always draw selected child last (might overwrite other children) */
    if (selected) {
        res = selected->func.draw(selected, offset);
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
        if (signal.clicked & HAL_BUTTON_ESC) {
            /* leave editing mode */
            c->flags.editing = 0;
            /* notify parent of focus */
            if (c->base.parent) {
                widget_gotFocus(c->base.parent);
            }
            widget_deselectAll(c->base.firstChild);
            signal.clicked &= ~HAL_BUTTON_ESC;
        }
    } else {
        /* not in editing mode */
        if (signal.clicked & HAL_BUTTON_ENTER) {
            /* start editing mode */
            if (widget_selectFirst(c->base.firstChild) == GUI_OK) {
                c->flags.editing = 1;
                /* notify parent of lost focus */
                if (c->base.parent) {
                    widget_lostFocus(c->base.parent);
                }
            }
            signal.clicked &= ~HAL_BUTTON_ENTER;
        }
    }
    if (signal.lostFocus) {
        signal.lostFocus = 0;
        c->flags.focussed = 0;
    } else if (signal.gotFocus) {
        signal.gotFocus = 0;
        c->flags.focussed = 1;
    }
    return signal;
}

