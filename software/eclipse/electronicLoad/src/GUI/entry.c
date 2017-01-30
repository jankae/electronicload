#include "entry.h"

void entry_create(entry_t *e, int32_t *value, int32_t *max, int32_t *min,
        font_t font, uint8_t digits, unit_t unit, void *cb) {
    widget_init((widget_t*) e);
    /* set widget functions */
    e->base.func.draw = entry_draw;
    e->base.func.input = entry_input;
    /* set member variables */
    e->value = value;
    e->max = max;
    e->min = min;
    e->font = font;
    e->unit = unit;
    e->digits = digits;
    e->changeCallback = cb;
    e->flags.editing = 0;
    e->flags.encoderEdit = 0;
    e->encEditPos = digits - 1;
    e->base.size.y = fontSize[font].height + 3;
    e->base.size.x = fontSize[font].width * (digits + 3) + 3;
}

uint32_t entry_GetInputStringValue(entry_t *e, uint32_t multiplier) {
    uint32_t value = 0;
    uint8_t i;
    uint32_t div = 0;
    for (i = 0; i < e->digits + 1; i++) {
        if (e->inputString[i] >= '0' && e->inputString[i] <= '9') {
            value *= 10;
            value += e->inputString[i] - '0';
            if (div) {
                div *= 10;
            }
        } else if (e->inputString[i] == '.') {
            div = 1;
        }
    }
    value *= multiplier;
    if (div)
        value /= div;
    return value;
}

uint32_t entry_getIncrement(entry_t *e) {
    /* get dot position from unit prefix */
    uint32_t dotMult = 1;
    if (!strcmp(&e->inputString[e->digits + 1], unitNames[e->unit][1])) {
        dotMult = 1000;
    } else if (!strcmp(&e->inputString[e->digits + 1], unitNames[e->unit][2])) {
        dotMult = 1000000;
    }
    /* find dot position */
    uint8_t dot;
    for (dot = 0; dot <= e->digits; dot++) {
        if (e->inputString[dot] == '.')
            break;
    }
    while (e->encEditPos < dot - 1) {
        dotMult *= 10;
        dot--;
    }
    while (e->encEditPos > dot) {
        dotMult /= 10;
        dot++;
    }
    return dotMult;
}

void entry_setIncrement(entry_t *e, uint32_t increment) {
    /* get dot position from unit prefix */
    uint32_t dotMult = 1;
    if (!strcmp(&e->inputString[e->digits + 1], unitNames[e->unit][1])) {
        dotMult = 1000;
    } else if (!strcmp(&e->inputString[e->digits + 1], unitNames[e->unit][2])) {
        dotMult = 1000000;
    }
    /* find dot position */
    uint8_t dot;
    for (dot = 0; dot <= e->digits; dot++) {
        if (e->inputString[dot] == '.')
            break;
    }
    e->encEditPos = dot - 1;
    if (increment > dotMult) {
        while (increment > dotMult) {
            if (!e->encEditPos) {
                /* reached end of setting */
                return;
            }
            e->encEditPos--;
            dotMult *= 10;
        }
    } else if (increment < dotMult) {
        e->encEditPos++;
        while (increment < dotMult) {
            e->encEditPos++;
            dotMult /= 10;
        }
    }
    if (e->encEditPos > e->digits) {
        e->encEditPos = e->digits;
    }
}

int32_t entry_constrainValue(entry_t *e, int32_t value) {
    if (e->max && value > *e->max) {
        return *e->max;
    } else if (e->min && value < *e->min) {
        return *e->min;
    } else {
        return value;
    }
}

GUIResult_t entry_draw(widget_t *w, coords_t offset) {
    entry_t *e = (entry_t*) w;
    /* calculate corners */
    coords_t upperLeft = offset;
    upperLeft.x += e->base.position.x;
    upperLeft.y += e->base.position.y;
    coords_t lowerRight = upperLeft;
    lowerRight.x += e->base.size.x - 1;
    lowerRight.y += e->base.size.y - 1;
    screen_Rectangle(upperLeft.x, upperLeft.y, lowerRight.x, lowerRight.y);
    if (e->flags.encoderEdit) {
        screen_String(e->inputString, e->font, upperLeft.x + 1,
                upperLeft.y + 2);
        /* invert char at current edit position */
        screen_FullRectangle(
                upperLeft.x + 1 + fontSize[e->font].width * e->encEditPos,
                upperLeft.y + 1,
                upperLeft.x + 1 + fontSize[e->font].width * (e->encEditPos + 1)
                        - 1, lowerRight.y - 1, PIXEL_INVERT);
        screen_SetSoftButton("\x1b", 0);
        screen_SetSoftButton("\x1a", 1);
    } else if (e->flags.editing) {
        /* "popup" in the middle of the screen */
        coords_t size;
        size.x = (e->digits + 1) * fontSize[FONT_BIG].width + 5;
        size.y = fontSize[FONT_BIG].height + 5;
        upperLeft.x = (SCREEN_WIDTH - size.x) / 2;
        upperLeft.y = (SCREEN_HEIGHT - size.y) / 2;
        /* clear area */
        screen_FullRectangle(upperLeft.x-2, upperLeft.y-2, upperLeft.x + size.x + 1,
                upperLeft.y + size.y + 1, PIXEL_OFF);
        /* draw border */
        screen_Rectangle(upperLeft.x, upperLeft.y, upperLeft.x + size.x - 1,
                upperLeft.y + size.y - 1);
        screen_Rectangle(upperLeft.x + 1, upperLeft.y + 1,
                upperLeft.x + size.x - 2, upperLeft.y + size.y - 2);
        /* draw current input string */
        screen_String(e->inputString, FONT_BIG, upperLeft.x + 2,
                upperLeft.y + 3);
        screen_SetSoftButton(unitNames[e->unit][0], 0);
        screen_SetSoftButton(unitNames[e->unit][1], 1);
        screen_SetSoftButton(unitNames[e->unit][2], 2);
    } else {
        /* construct value string */
        string_fromUintUnits(*e->value, e->inputString, e->digits,
                unitNames[e->unit][0], unitNames[e->unit][1],
                unitNames[e->unit][2]);
        /* display string */
        screen_String(e->inputString, e->font, upperLeft.x + 1,
                upperLeft.y + 2);
        if (e->base.flags.selected) {
            /* invert button area */
            screen_FullRectangle(upperLeft.x + 1, upperLeft.y + 1,
                    lowerRight.x - 1, lowerRight.y - 1, PIXEL_INVERT);
        }
    }

    return GUI_OK;
}

GUISignal_t entry_input(widget_t *w, GUISignal_t signal) {
    entry_t *e = (entry_t*) w;
    if (signal.clicked & (HAL_BUTTON_ISDIGIT | HAL_BUTTON_DOT)) {
        if (!e->flags.editing) {
            /* initialize editing mode */
            e->flags.editing = 1;
            memset(e->inputString, ' ', e->digits + 1);
            e->inputString[e->digits + 1] = 0;
            e->flags.dotSet = 0;
            e->flags.encoderEdit = 0;
        }
        if (e->inputString[0] == ' ') {
            /* still room for more input */
            if (!e->flags.dotSet || !(signal.clicked & HAL_BUTTON_DOT)) {
                /* move input string one to the left */
                memmove(e->inputString, &e->inputString[1], e->digits + 1);
                /* add pressed character to end of string */
                char c;
                if (signal.clicked & HAL_BUTTON_0)
                    c = '0';
                else if (signal.clicked & HAL_BUTTON_1)
                    c = '1';
                else if (signal.clicked & HAL_BUTTON_2)
                    c = '2';
                else if (signal.clicked & HAL_BUTTON_3)
                    c = '3';
                else if (signal.clicked & HAL_BUTTON_4)
                    c = '4';
                else if (signal.clicked & HAL_BUTTON_5)
                    c = '5';
                else if (signal.clicked & HAL_BUTTON_6)
                    c = '6';
                else if (signal.clicked & HAL_BUTTON_7)
                    c = '7';
                else if (signal.clicked & HAL_BUTTON_8)
                    c = '8';
                else if (signal.clicked & HAL_BUTTON_9)
                    c = '9';
                else if (signal.clicked & HAL_BUTTON_DOT) {
                    c = '.';
                    e->flags.dotSet = 1;
                }
                e->inputString[e->digits] = c;
            }
        }
        signal.clicked &= ~(HAL_BUTTON_ISDIGIT | HAL_BUTTON_DOT);
    } else if (e->flags.editing) {
        /* leave editing mode by escape or setting the new value with one of the soft buttons */
        if (signal.clicked & HAL_BUTTON_ESC) {
            e->flags.editing = 0;
            signal.clicked &= ~HAL_BUTTON_ESC;
        } else if (signal.clicked & HAL_BUTTON_SOFT) {
            /* editing done, convert string to value */
            /* multiply with soft button factor */
            int32_t val;
            if (signal.clicked & HAL_BUTTON_SOFT0) {
                val = entry_GetInputStringValue(e, 1);
            } else if (signal.clicked & HAL_BUTTON_SOFT1) {
                val = entry_GetInputStringValue(e, 1000);
            } else if (signal.clicked & HAL_BUTTON_SOFT2) {
                val = entry_GetInputStringValue(e, 1000000);
            }
            *e->value = entry_constrainValue(e, val);
            signal.clicked &= ~HAL_BUTTON_SOFT;
            e->flags.editing = 0;
            if (e->changeCallback)
                e->changeCallback();
        }
    } else if (e->flags.encoderEdit) {
        /* leave editing mode by escape */
        if (signal.clicked & HAL_BUTTON_ESC) {
            e->flags.encoderEdit = 0;
            signal.clicked &= ~HAL_BUTTON_ESC;
        }
        if (signal.clicked & HAL_BUTTON_SOFT0) {
            /* move edit position to the left */
            if (e->encEditPos > 0) {
                e->encEditPos--;
                if (e->inputString[e->encEditPos] == '.')
                    e->encEditPos--;
            }
            signal.clicked &= ~HAL_BUTTON_SOFT0;
        } else if (signal.clicked & HAL_BUTTON_SOFT1) {
            /* move edit position to the right */
            if (e->encEditPos < e->digits) {
                e->encEditPos++;
                if (e->inputString[e->encEditPos] == '.')
                    e->encEditPos++;
            }
            signal.clicked &= ~HAL_BUTTON_SOFT1;
        }
        if (signal.encoder != 0) {
            /* entry field is in encoder edit mode */
            uint32_t increment = entry_getIncrement(e);
            uint32_t tempValue = *e->value + signal.encoder * increment;
            if (*e->value != entry_constrainValue(e, tempValue)) {
                *e->value = entry_constrainValue(e, tempValue);
                if (e->changeCallback)
                    e->changeCallback();
            }
            string_fromUintUnits(*e->value, e->inputString, e->digits,
                    unitNames[e->unit][0], unitNames[e->unit][1],
                    unitNames[e->unit][2]);
            signal.encoder = 0;
            entry_setIncrement(e, increment);
        }
    } else if (signal.clicked & HAL_BUTTON_ENTER) {
        e->flags.encoderEdit = 1;
        if (e->inputString[e->encEditPos] == '.') {
            /* can't edit the dot position */
            e->encEditPos++;
        }
        signal.clicked &= ~HAL_BUTTON_ENTER;
    }
    return signal;
}
