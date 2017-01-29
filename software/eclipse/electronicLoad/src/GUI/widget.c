#include "widget.h"

void widget_init(widget_t *w) {
    memset(w, 0, sizeof(widget_t));
    w->flags.visible = 1;
    w->flags.selectable = 1;
}

GUIResult_t widget_selectNext(widget_t *first){
    if(!first)
        return GUI_ERROR;
    widget_t *eligible = NULL;
    /* find selected item */
    while(first) {
        if(first->flags.selected)
            break;
        first = first->next;
    }
    if(!first) {
        /* no widget selected */
        return GUI_UNABLE;
    }
    /* find next selectable item */
    eligible = first->next;
    while(eligible) {
        if(eligible->flags.selectable && eligible->flags.visible)
            /* this item can be selected */
            break;
        eligible = eligible->next;
    }
    if(!eligible) {
        /* no next item selectable */
        return GUI_UNABLE;
    }
    /* select next item */
    first->flags.selected = 0;
    eligible->flags.selected = 1;
    return GUI_OK;
}

GUIResult_t widget_selectPrevious(widget_t *first){
    if(!first)
        return GUI_ERROR;
    widget_t *eligible = NULL;
    /* find selected item */
    while(first) {
        if(first->flags.selected)
            break;
        if(first->flags.selectable && first->flags.visible)
            /* this widget could be selected */
            eligible = first;
        first = first->next;
    }
    if(!first) {
        /* no widget selected */
        return GUI_UNABLE;
    }
    if(!eligible) {
        /* no previous item selectable */
        return GUI_UNABLE;
    }
    /* select previous item */
    first->flags.selected = 0;
    eligible->flags.selected = 1;
    return GUI_OK;
}

GUIResult_t widget_selectFirst(widget_t *first){
    if(!first)
        return GUI_ERROR;
    /* find first eligible item */
    while(first) {
        if(first->flags.selectable && first->flags.visible)
            break;
        first = first->next;
    }
    if(!first) {
        /* no widget eligible */
        return GUI_UNABLE;
    }
    /* select first eligible widget */
    first->flags.selected = 1;
    return GUI_OK;
}

GUIResult_t widget_deselectAll(widget_t *first){
    if(!first)
        return GUI_ERROR;
    /* iterate over all widgets and deselect them */
    while(first) {
        first->flags.selected = 0;
        first = first->next;
    }
    return GUI_OK;
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
        if (child->flags.selected) {
            signal = widget_input(child, signal);
        }
    }
    /* Then try to handle the signal itself */
    signal = w->func.input(w, signal);
    return signal;
}
