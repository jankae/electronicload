#include "button.h"

void button_create(button_t *button, uint8_t width, uint8_t height, char *name,
        void *cb) {
    /* initialize common widget values */
    widget_init((widget_t*) button);
    /* set widget functions */
    button->base.func.draw = button_draw;
    button->base.func.input = button_input;
    /* set name and callback */
    button->name = name;
    button->callback = cb;

    /* calculate biggest compatible font */
    button->base.size.x = width;
    button->base.size.y = height;

    uint8_t nameLength = strlen(button->name);
    button->font = FONT_NUM_SIZES;
    uint8_t fits = 0;
    uint8_t spaceHeight = button->base.size.y - 3;
    uint8_t spaceWidth = button->base.size.x - 3;
    do {
        button->font--;
        if (fontSize[button->font].height <= spaceHeight
                && fontSize[button->font].width * nameLength <= spaceWidth) {
            fits = 1;
        }
    } while (!fits && button->font);
    if (!fits) {
        /* even the smallest font didn't fit */
        button->name = NULL;
    }
    /* calculate font start position */
    button->fontStart.y = (button->base.size.y - fontSize[button->font].height)
            / 2;
    button->fontStart.x = (button->base.size.x
            - fontSize[button->font].width * nameLength) / 2;
}

GUIResult_t button_draw(widget_t *w, coords_t offset) {
    button_t *b = (button_t*) w;
    /* calculate corners */
    coords_t upperLeft = offset;
    upperLeft.x += b->base.position.x;
    upperLeft.y += b->base.position.y;
    coords_t lowerRight = upperLeft;
    lowerRight.x += b->base.size.x - 1;
    lowerRight.y += b->base.size.y - 1;
    /* draw outline */
    screen_VerticalLine(upperLeft.x, upperLeft.y + 1, b->base.size.y - 2);
    screen_VerticalLine(lowerRight.x - 1, upperLeft.y + 1, b->base.size.y - 2);
    screen_VerticalLine(lowerRight.x, upperLeft.y + 1, b->base.size.y - 2);
    screen_HorizontalLine(upperLeft.x + 1, upperLeft.y, b->base.size.x - 2);
    screen_HorizontalLine(upperLeft.x + 1, lowerRight.y - 1,
            b->base.size.x - 2);
    screen_HorizontalLine(upperLeft.x + 1, lowerRight.y, b->base.size.x - 2);
    if (b->name) {
        screen_String(b->name, b->font, upperLeft.x + b->fontStart.x,
                upperLeft.y + b->fontStart.y);
    }
    if (b->base.flags.selected) {
        /* invert button area */
        screen_FullRectangle(upperLeft.x + 1, upperLeft.y + 1,
                lowerRight.x - 2, lowerRight.y - 2, PIXEL_ON);
    }
    return GUI_OK;
}

GUISignal_t button_input(widget_t *w, GUISignal_t signal) {
    button_t *b = (button_t*) w;
    if ((signal.clicked & HAL_BUTTON_ENTER)
            || (signal.clicked & HAL_BUTTON_ENCODER)) {
        /* call button clicked callback */
        if (b->callback) {
            b->callback();
        }
        /* clear this signal */
        signal.clicked &= ~(HAL_BUTTON_ENTER | HAL_BUTTON_ENCODER);
    }
    return signal;
}
