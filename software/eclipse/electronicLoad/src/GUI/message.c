#include "message.h"

GUIMessageResult_t message_Box(const char *text, uint8_t lines, uint8_t columns,
        font_t font, GUIMessageType_t msg, widget_t *callee) {
    /* calculate message size */
    coords_t size;
    size.x = columns * fontSize[font].width + 5;
    size.y = lines * fontSize[font].height + 5;
    coords_t upperLeft;
    upperLeft.x = (SCREEN_WIDTH - size.x) / 2;
    upperLeft.y = (SCREEN_HEIGHT - size.y) / 2;
    /* clear area */
    screen_Clear();
//    screen_FullRectangle(upperLeft.x - 2, upperLeft.y - 2,
//            upperLeft.x + size.x + 1, upperLeft.y + size.y + 1, PIXEL_OFF);
    /* draw border */
    screen_Rectangle(upperLeft.x, upperLeft.y, upperLeft.x + size.x - 1,
            upperLeft.y + size.y - 1);
    screen_Rectangle(upperLeft.x + 1, upperLeft.y + 1, upperLeft.x + size.x - 2,
            upperLeft.y + size.y - 2);
    /* draw message text */
    uint8_t i, j;
    for (i = 0; i < lines; i++) {
        for (j = 0; j < columns; j++) {
            if (*text == 0)
                break;
            if (*text == '\n') {
                text++;
                /* move to next line */
                break;
            } else {
                /* print this char */
                screen_Char(*text, font,
                        upperLeft.x + j * fontSize[font].width + 2,
                        upperLeft.y + i * fontSize[font].height + 3);
                text++;
            }
        }
        if (*text == 0)
            break;
        if (*text == '\n') {
            /* newline at forced linebreak -> skip */
            text++;
        }
    }
    /* setup soft buttons */
    switch (msg) {
    case MESSAGE_OK:
        screen_SetSoftButton("OK", 2);
        break;
    case MESSAGE_OK_ABORT:
        screen_SetSoftButton("OK", 2);
        screen_SetSoftButton("ABORT", 0);
        break;
    case MESSAGE_YES_NO:
        screen_SetSoftButton("YES", 2);
        screen_SetSoftButton("NO", 0);
        break;
    }
    /* wait for user to release buttons */
    while (hal_getButton())
        ;
    /* wait for user to press a button */
    GUIMessageResult_t res = MESSAGE_RESULT_NONE;
    uint32_t button;
    do {
        button = hal_getButton();
        switch (msg) {
        case MESSAGE_OK:
            if (button & HAL_BUTTON_SOFT2)
                res = MESSAGE_RES_OK;
            break;
        case MESSAGE_OK_ABORT:
            if (button & HAL_BUTTON_SOFT2)
                res = MESSAGE_RES_OK;
            if (button & HAL_BUTTON_SOFT0)
                res = MESSAGE_RES_ABORT;
            break;
        case MESSAGE_YES_NO:
            if (button & HAL_BUTTON_SOFT2)
                res = MESSAGE_RES_YES;
            if (button & HAL_BUTTON_SOFT0)
                res = MESSAGE_RES_NO;
            break;
        }
    } while (res == MESSAGE_RESULT_NONE && !(button & HAL_BUTTON_ESC));
    /* wait for user to release buttons */
    while (hal_getButton())
        ;
    if (callee) {
        /* redraw screen */
        widget_Redraw(callee);
    }
    return res;
}
