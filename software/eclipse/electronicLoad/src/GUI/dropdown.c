#include "dropdown.h"

void dropdown_create(dropdown_t *d, char **items, uint8_t *value, font_t font,
        uint8_t minSize, void (*cb)(void)) {
    widget_init((widget_t*) d);
    /* set widget functions */
    d->base.func.draw = dropdown_draw;
    d->base.func.input = dropdown_input;
    /* set member variables */
    d->flags.editing = 0;
    d->changeCallback = cb;
    d->font = font;
    d->itemlist = items;
    d->value = value;
    /* find number of items and longest item */
    uint8_t maxLength = 0;
    for (d->numItems = 0; d->itemlist[d->numItems]; d->numItems++) {
        uint8_t length = strlen(d->itemlist[d->numItems]);
        if (length > maxLength)
            maxLength = length;
    }
    /* calculate size */
    d->base.size.y = fontSize[font].height + 3;
    d->base.size.x = fontSize[font].width * (maxLength + 1) + 3;
}

GUIResult_t dropdown_draw(widget_t *w, coords_t offset) {
    dropdown_t *d = (dropdown_t*) w;
    /* calculate corners */
    coords_t upperLeft = offset;
    upperLeft.x += d->base.position.x;
    upperLeft.y += d->base.position.y;
    coords_t lowerRight = upperLeft;
    lowerRight.x += d->base.size.x - 1;
    lowerRight.y += d->base.size.y - 1;
    if (!d->flags.editing) {
        screen_Rectangle(upperLeft.x, upperLeft.y, lowerRight.x, lowerRight.y);
        /* print currently selected item */
        screen_String(d->itemlist[*d->value], d->font, upperLeft.x + 1,
                upperLeft.y + 2);
        /* print dropdown arrow */
        screen_String("\x1F", d->font,
                lowerRight.x - fontSize[d->font].width - 1, upperLeft.y + 2);
        /* invert arrow area */
        screen_FullRectangle(lowerRight.x - fontSize[d->font].width - 1,
                upperLeft.y + 1, lowerRight.x - 1, lowerRight.y - 1,
                PIXEL_INVERT);
        if (d->base.flags.selected) {
            /* invert button area */
            screen_FullRectangle(upperLeft.x + 1, upperLeft.y + 1,
                    lowerRight.x - 1, lowerRight.y - 1, PIXEL_INVERT);
        }
    } else {
        /* currently in edit mode */
        /* calculate amount of lines available */
        uint8_t linesBelow = (SCREEN_HEIGHT - 1 - lowerRight.y)
                / fontSize[d->font].height;
        uint8_t linesAbove = (upperLeft.y - 0) / fontSize[d->font].height;
        if (linesBelow + 1 >= d->numItems) {
            /* space below dropdown menu is sufficient */
            linesAbove = 0;
            linesBelow = d->numItems - 1;
        } else if (linesBelow + linesAbove + 1 > d->numItems) {
            /* space above is not fully used */
            linesAbove = d->numItems - linesBelow - 1;
        }
        /* total number of displayed items */
        uint8_t displayedItems = linesBelow + linesAbove + 1;
        /* calculate coordinates of menu corners */
        coords_t editViewUpperLeft;
        coords_t editViewLowerRight;
        editViewUpperLeft.x = upperLeft.x;
        editViewUpperLeft.y = upperLeft.y
                - linesAbove * fontSize[d->font].height;
        editViewLowerRight.x = lowerRight.x;
        editViewLowerRight.y = lowerRight.y
                + linesBelow * fontSize[d->font].height;
        /* clear this area because we are exceeding the size of this widget */
        screen_FullRectangle(editViewUpperLeft.x, editViewUpperLeft.y,
                editViewLowerRight.x, editViewLowerRight.y, PIXEL_OFF);
        /* shift viewing window if necessary */
        if (d->editPos < d->editFirstView) {
            /* selected item is above current view */
            d->editFirstView = d->editPos;
        } else if (d->editPos - d->editFirstView >= displayedItems) {
            /* selected item is below current view */
            d->editFirstView = d->editPos - displayedItems + 1;
        }
        /* show border of edit window */
        screen_Rectangle(editViewUpperLeft.x, editViewUpperLeft.y,
                editViewLowerRight.x, editViewLowerRight.y);
        /* show items */
        uint8_t i;
        for (i = 0; i < displayedItems; i++) {
            /* print item string */
            screen_String(d->itemlist[i + d->editFirstView], d->font,
                    upperLeft.x + 1,
                    upperLeft.y + 2
                            + (i - linesAbove) * fontSize[d->font].height);
        }
        /* invert selected item */
        screen_FullRectangle(editViewUpperLeft.x + 1,
                editViewUpperLeft.y + 1
                        + (d->editPos - d->editFirstView)
                                * fontSize[d->font].height,
                editViewLowerRight.x - fontSize[d->font].width - 2,
                editViewUpperLeft.y + 1
                        + (d->editPos - d->editFirstView + 1)
                                * fontSize[d->font].height, PIXEL_INVERT);
        /* display scroll bar */
        /* calculate beginning and end of scrollbar */
        uint8_t scrollBegin = common_Map(d->editFirstView, 0, d->numItems - 1,
                0, editViewLowerRight.y - editViewUpperLeft.y);
        uint8_t scrollEnd = common_Map(d->editFirstView + displayedItems, 0,
                d->numItems, 0, editViewLowerRight.y - editViewUpperLeft.y);
        screen_VerticalLine(editViewLowerRight.x - fontSize[d->font].width,
                editViewUpperLeft.y,
                editViewLowerRight.y - editViewUpperLeft.y);
        /* display position indicator */
        screen_FullRectangle(editViewLowerRight.x - fontSize[d->font].width + 1,
                editViewUpperLeft.y + scrollBegin + 1, editViewLowerRight.x - 1,
                editViewUpperLeft.y + scrollEnd - 1, PIXEL_ON);
    }
    return GUI_OK;
}

GUISignal_t dropdown_input(widget_t *w, GUISignal_t signal) {
    dropdown_t *d = (dropdown_t*) w;
    if (d->flags.editing) {
        if (signal.clicked & HAL_BUTTON_ESC) {
            /* abort editing, don't save value */
            d->flags.editing = 0;
            signal.clicked &= ~HAL_BUTTON_ESC;
        } else if (signal.clicked & HAL_BUTTON_ENTER) {
            /* finish editing, save current value */
            d->flags.editing = 0;
            if (*d->value != d->editPos) {
                *d->value = d->editPos;
                if (d->changeCallback) {
                    d->changeCallback();
                }
            }
            signal.clicked &= ~HAL_BUTTON_ESC;
        } else if (signal.encoder != 0) {
            /* change currently selected position */
            if (signal.encoder < 0 && d->editPos <= -signal.encoder) {
                /* go all the way to the first item */
                d->editPos = 0;
            } else if (signal.encoder > 0
                    && (d->numItems - d->editPos - 1) <= signal.encoder) {
                /* go all the way to the last item */
                d->editPos = d->numItems - 1;
            } else {
                d->editPos += signal.encoder;
            }
            signal.encoder = 0;
        }
    } else {
        /* not in edit mode */
        if (signal.clicked & HAL_BUTTON_ENTER) {
            /* start editing */
            d->flags.editing = 1;
            d->editPos = *d->value;
            d->editFirstView = 0;
            signal.clicked &= ~HAL_BUTTON_ESC;
        }
    }
    return signal;
}
