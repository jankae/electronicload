#include "checkbox.h"

void checkbox_create(checkbox_t *c, uint8_t *value, void *cb) {
    /* initialize common widget values */
    widget_init((widget_t*) c);
    /* set widget functions */
    c->base.func.draw = checkbox_draw;
    c->base.func.input = checkbox_input;
    /* set callback */
    c->value = value;
    c->callback = cb;
    c->base.size.x = 11;
    c->base.size.y = 11;

}

GUIResult_t checkbox_draw(widget_t *w, coords_t offset) {
    checkbox_t *c = (checkbox_t*) w;
    /* calculate corners */
    coords_t upperLeft = offset;
    upperLeft.x += c->base.position.x;
    upperLeft.y += c->base.position.y;
    coords_t lowerRight = upperLeft;
    lowerRight.x += c->base.size.x - 1;
    lowerRight.y += c->base.size.y - 1;
    screen_Rectangle(upperLeft.x, upperLeft.y, lowerRight.x, lowerRight.y);
    if (*c->value) {
        screen_Line(upperLeft.x + 2, upperLeft.y + 6, upperLeft.x + 4,
                lowerRight.y - 2);
        screen_Line(lowerRight.x - 2, upperLeft.y + 2, upperLeft.x + 4,
                lowerRight.y - 2);
    } else {
        screen_Line(upperLeft.x + 3, upperLeft.y + 3, lowerRight.x - 3,
                lowerRight.y - 3);
        screen_Line(upperLeft.x + 3, lowerRight.y - 3, lowerRight.x - 3,
                upperLeft.y + 3);
    }
    if (c->base.flags.selected) {
        /* invert checkbox area */
        screen_FullRectangle(upperLeft.x + 1, upperLeft.y + 1, lowerRight.x - 1,
                lowerRight.y - 1, PIXEL_INVERT);
    }
    return GUI_OK;
}

GUISignal_t checkbox_input(widget_t *w, GUISignal_t signal) {
    checkbox_t *c = (checkbox_t*) w;
    if ((signal.clicked & HAL_BUTTON_ENTER)
            || (signal.clicked & HAL_BUTTON_ENCODER)) {
        *c->value = !*c->value;
        /* call button clicked callback */
        if (c->callback) {
            c->callback();
        }
        /* clear this signal */
        signal.clicked &= ~(HAL_BUTTON_ENTER | HAL_BUTTON_ENCODER);
    }
    return signal;
}
