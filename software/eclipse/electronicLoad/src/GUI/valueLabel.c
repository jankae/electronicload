#include "valueLabel.h"

void valueLabel_create(valueLabel_t *e, int32_t *value, font_t font,
        uint8_t digits, unit_t unit) {
    widget_init((widget_t*) e);
    /* set widget functions */
    e->base.func.draw = valueLabel_draw;
    e->base.func.input = valueLabel_input;
    e->base.flags.selectable = 0;
    /* set member variables */
    e->value = value;
    e->font = font;
    e->unit = unit;
    e->digits = digits;
    e->base.size.y = fontSize[font].height + 3;
    /* get longest unit name */
    uint8_t longestUnit = 0;
    uint8_t i;
    for (i = 0; i < 3; i++) {
        uint8_t length = strlen(unitNames[unit][i]);
        if (length > longestUnit)
            longestUnit = length;
    }
    e->base.size.x = fontSize[font].width * (digits + longestUnit + 1) + 3;
}

GUIResult_t valueLabel_draw(widget_t *w, coords_t offset) {
    valueLabel_t *e = (valueLabel_t*) w;
    /* calculate corners */
    coords_t upperLeft = offset;
    upperLeft.x += e->base.position.x;
    upperLeft.y += e->base.position.y;
    coords_t lowerRight = upperLeft;
    lowerRight.x += e->base.size.x - 1;
    lowerRight.y += e->base.size.y - 1;
    screen_Rectangle(upperLeft.x, upperLeft.y, lowerRight.x, lowerRight.y);
    /* construct value string */
    char buffer[e->base.size.x / fontSize[e->font].width + 1];
    string_fromUintUnits(*e->value, buffer, e->digits,
            unitNames[e->unit][0], unitNames[e->unit][1],
            unitNames[e->unit][2]);
    /* display string */
    screen_String(buffer, e->font, upperLeft.x + 1, upperLeft.y + 2);

    return GUI_OK;
}

GUISignal_t valueLabel_input(widget_t *w, GUISignal_t signal) {
    /* valueLabel doesn't handle any input */
    return signal;}
