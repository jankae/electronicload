#include "notebook.h"

GUIResult_t notebook_create(notebook_t *n, font_t font, uint8_t x, uint8_t y) {
    /* Check if size is sufficient for font */
    if (y < fontSize[font].height + 3) {
        /* Too small for even just the page names */
        return GUI_UNABLE;
    }
    widget_init((widget_t*) n);
    /* set widget functions */
    n->base.func.draw = notebook_draw;
    n->base.func.input = notebook_input;
    /* set member variables */
    n->flags.editing = 0;
    n->flags.focussed = 0;
    n->font = font;
    n->numItems = 0;
    n->selectedPage = 0;
    n->pageStartOffset = 0;
    /* set size */
    n->base.size.x = x;
    n->base.size.y = y;
    return GUI_OK;
}

GUIResult_t notebook_addPage(notebook_t *n, widget_t *page, const char *name) {
    if (n->numItems >= NOTEBOOK_MAX_PAGES) {
        /* already at maximum number of pages */
        return GUI_UNABLE;
    }
    /* check size limit */
    if (page->size.x > n->base.size.x
            || page->size.y > n->base.size.y - fontSize[n->font].height - 3) {
        /* widget doesn't fit into notebook page */
        return GUI_UNABLE;
    }
    if (n->base.firstChild) {
        /* find end of children list */
        widget_t *child = n->base.firstChild;
        while (child->next) {
            child = child->next;
        }
        /* add widget to the end */
        child->next = page;
    } else {
        /* this is the first child */
        n->base.firstChild = page;
    }
    page->position.x = 0;
    page->position.y = fontSize[n->font].height + 3;
    page->parent = (widget_t*) n;
    n->pageNames[n->numItems] = name;
    n->numItems++;
    return GUI_OK;
}

GUIResult_t notebook_draw(widget_t *w, coords_t offset) {
    offset.x += w->position.x;
    offset.y += w->position.y;
    notebook_t *n = (notebook_t*) w;
    if (!n->base.firstChild) {
        /* notebook needs at least one page */
        return GUI_UNABLE;
    }
    /* calculate corners */
    coords_t upperLeft = offset;
    coords_t lowerRight = upperLeft;
    lowerRight.x += n->base.size.x - 1;
    lowerRight.y += n->base.size.y - 1;

    /* calculate start position of selected entry */
    uint8_t i;
    uint32_t startSelected = 0;
    for (i = 0; i != n->selectedPage; i++) {
        startSelected += strlen(n->pageNames[i]) * fontSize[n->font].width + 2;
    }
    uint32_t stopSelected = startSelected
            + strlen(n->pageNames[i]) * fontSize[n->font].width + 2;
    /* adjust offset if necessary */
    if (startSelected < n->pageStartOffset) {
        n->pageStartOffset = startSelected;
    } else if (stopSelected - n->pageStartOffset >= n->base.size.x) {
        n->pageStartOffset = stopSelected - n->base.size.x + 1;
    }
    /* draw divider line */
    screen_HorizontalLine(upperLeft.x,
            upperLeft.y + fontSize[n->font].height + 2,
            startSelected - n->pageStartOffset + 1);
    screen_HorizontalLine(upperLeft.x + stopSelected - n->pageStartOffset,
            upperLeft.y + fontSize[n->font].height + 2,
            lowerRight.x - stopSelected + n->pageStartOffset + 1);

    /* draw page names */
    int32_t poffset = -n->pageStartOffset;
    for (i = 0; i < n->numItems; i++) {
        if (poffset >= 0) {
            screen_VerticalLine(upperLeft.x + poffset, upperLeft.y + 1,
                    fontSize[n->font].height + 1);
        }
        poffset++;
        uint32_t nameLength = strlen(n->pageNames[i]) * fontSize[n->font].width;
        if (poffset + nameLength > 0) {
            /* page name is at least partially visible */
            screen_String(n->pageNames[i], n->font, upperLeft.x + poffset,
                    upperLeft.y + 2);
            if (upperLeft.x + poffset > 0) {
                screen_HorizontalLine(upperLeft.x + poffset, upperLeft.y,
                        nameLength + 1);
            } else {
                screen_HorizontalLine(0, upperLeft.y,
                        nameLength + 1 + upperLeft.x + poffset);
            }
            if (n->flags.editing && n->flags.focussed && i == n->selectedPage) {
                /* invert pagename */
                screen_FullRectangle(upperLeft.x + poffset, upperLeft.y + 1,
                        upperLeft.x + poffset + nameLength,
                        upperLeft.y + fontSize[n->font].height + 1,
                        PIXEL_INVERT);
            }
        }
        poffset += nameLength + 1;
        if (poffset >= 0) {
            screen_VerticalLine(upperLeft.x + poffset, upperLeft.y + 1,
                    fontSize[n->font].height + 1);
        }
        if (poffset >= n->base.size.x) {
            /* reached end of notebook */
            break;
        }
    }
    /* draw the selected child */
    widget_t *selPage = n->base.firstChild;
    for (i = 0; i != n->selectedPage; i++) {
        selPage = selPage->next;
    }
    if (!selPage) {
        /* selected page is a not set, this shouldn't be possible */
        return GUI_ERROR;
    }
    GUIResult_t res = selPage->func.draw(selPage, offset);
    if (res != GUI_OK) {
        return res;
    }
    if (n->base.flags.selected && !n->flags.editing) {
        screen_FullRectangle(upperLeft.x, upperLeft.y + 1, lowerRight.x,
                upperLeft.y + fontSize[n->font].height + 1, PIXEL_INVERT);
    }
    return GUI_OK;
}

GUISignal_t notebook_input(widget_t *w, GUISignal_t signal) {
    notebook_t *n = (notebook_t*) w;
    if (n->flags.editing) {
        if (signal.encoder < 0) {
            /* move active item one up */
            if (n->selectedPage > 0
                    && widget_selectPrevious(n->base.firstChild) == GUI_OK)
                n->selectedPage--;
        } else if (signal.encoder > 0) {
            /* move active item one down */
            if (n->selectedPage < n->numItems - 1
                    && widget_selectNext(n->base.firstChild) == GUI_OK)
                n->selectedPage++;
        }

        signal.encoder = 0;
        if (signal.clicked & HAL_BUTTON_ESC) {
            /* leave editing mode */
            if (n->base.parent) {
                n->flags.editing = 0;
                /* notify parent of focus */
                widget_gotFocus(n->base.parent);
                widget_deselectAll(n->base.firstChild);
                signal.clicked &= ~HAL_BUTTON_ESC;
            } else {
                /* no parent -> can't leave highest level editing mode */
            }
        }
    } else {
        /* not in editing mode */
        if (signal.clicked & HAL_BUTTON_ENTER) {
            /* start editing mode */
            if (widget_selectWidget(n->base.firstChild, n->selectedPage)
                    == GUI_OK) {
                n->flags.editing = 1;
                /* notify parent of lost focus */
                if (n->base.parent) {
                    widget_lostFocus(n->base.parent);
                }
            }
            signal.clicked &= ~HAL_BUTTON_ENTER;
        }
    }
    if (signal.lostFocus) {
        signal.lostFocus = 0;
        n->flags.focussed = 0;
    } else if (signal.gotFocus) {
        signal.gotFocus = 0;
        n->flags.focussed = 1;
    }
    return signal;
}
