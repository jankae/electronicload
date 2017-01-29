/**
 * \file
 * \brief   High level display source file.
 *
 * Contains functions to write and draw on the display.
 * All functions don't modify the actual display content
 * but rather the internal display buffer from display.h
 */
#include "screen.h"

/**
 * \brief Clears the entire display
 */
void screen_Clear(void) {
    uint16_t i;
    for (i = 0; i < 1024; i++)
        display.buffer[i] = 0;
    display.updateTime = timer_SetTimeout(SCREEN_DISPLAY_TIMEOUT);
}

/*
 * \brief Sets or clears a specific pixel in the display data buffer
 *
 * \param x X-coordinate, (left = 0, right = 127)
 * \param y Y-coordinate, (up = 0, down = 63)
 * \param s new pixel state (PIXEL_OFF or PIXEL_ON)
 */
void screen_SetPixel(uint8_t x, uint8_t y, PixelState_t s) {
    if (x >= 128 || y >= 64)
        return;
    // calculate byteoffset
    uint16_t byte = x + (y / 8) * 128;
    uint8_t bit = 1 << (y % 8);
    if (s == PIXEL_ON) {
        display.buffer[byte] |= bit;
    } else if (s==PIXEL_OFF){
        display.buffer[byte] &= ~bit;
    } else {
        display.buffer[byte] ^= bit;
    }
    display.updateTime = timer_SetTimeout(SCREEN_DISPLAY_TIMEOUT);
}

/**
 * \brief Writes an entire byte (8 vertical pixel) in the display data buffer
 *
 * \param x X-coordinate, (left = 0, right = 127)
 * \param page Y-coordinate (up = 0, down = 7)
 * \param b Byte containing the pixeldata
 */
void screen_SetByte(uint8_t x, uint8_t page, uint8_t b) {
    if (x >= 128 || page >= 8)
        return;
    display.buffer[x + page * 128] = b;
    display.updateTime = timer_SetTimeout(SCREEN_DISPLAY_TIMEOUT);
}

void screen_VerticalLine(uint8_t x, uint8_t y, uint8_t length) {
    uint8_t s = y;
    for (; y < s + length; y++) {
        screen_SetPixel(x, y, PIXEL_ON);
    }
}

void screen_HorizontalLine(uint8_t x, uint8_t y, uint8_t length) {
    uint8_t s = x;
    for (; x < s + length; x++) {
        screen_SetPixel(x, y, PIXEL_ON);
    }
}

void screen_Line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
    uint8_t dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    uint8_t dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int8_t err = (dx > dy ? dx : -dy) / 2, e2;

    for (;;) {
        screen_SetPixel(x0, y0, PIXEL_ON);
        if (x0 == x1 && y0 == y1)
            break;
        e2 = err;
        if (e2 > -dx) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dy) {
            err += dx;
            y0 += sy;
        }
    }
}

void screen_Circle(int x0, int y0, int radius) {
    int f = 1 - radius;
    int ddF_x = 0;
    int ddF_y = -2 * radius;
    int x = 0;
    int y = radius;

    screen_SetPixel(x0, y0 + radius, PIXEL_ON);
    screen_SetPixel(x0, y0 - radius, PIXEL_ON);
    screen_SetPixel(x0 + radius, y0, PIXEL_ON);
    screen_SetPixel(x0 - radius, y0, PIXEL_ON);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x + 1;

        screen_SetPixel(x0 + x, y0 + y, PIXEL_ON);
        screen_SetPixel(x0 - x, y0 + y, PIXEL_ON);
        screen_SetPixel(x0 + x, y0 - y, PIXEL_ON);
        screen_SetPixel(x0 - x, y0 - y, PIXEL_ON);
        screen_SetPixel(x0 + y, y0 + x, PIXEL_ON);
        screen_SetPixel(x0 - y, y0 + x, PIXEL_ON);
        screen_SetPixel(x0 + y, y0 - x, PIXEL_ON);
        screen_SetPixel(x0 - y, y0 - x, PIXEL_ON);
    }
}

/**
 * \brief Draws a rectangle on the screen
 *
 * \param x1    X-coordinate of top left corner
 * \param y1    Y-coordinate of top left corner
 * \param x2    X-coordinate of bottom right corner
 * \param y2    Y-coordinate of bottom right corner
 */
void screen_Rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
    uint8_t i;
    for (i = x1; i <= x2; i++) {
        screen_SetPixel(i, y1, PIXEL_ON);
        screen_SetPixel(i, y2, PIXEL_ON);
    }
    for (i = y1 + 1; i < y2; i++) {
        screen_SetPixel(x1, i, PIXEL_ON);
        screen_SetPixel(x2, i, PIXEL_ON);
    }
}
void screen_FullRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, PixelState_t p){
    uint8_t i, j;
    for (i = x1; i <= x2; i++) {
        for (j = y1; j <= y2; j++) {
            screen_SetPixel(i, j, p);
        }
    }
}

/**
 * \brief Writes a 12x16 font character into the display data buffer
 *
 * Function is fast because it transfers the character into the
 * buffer by copying the whole bytes. This also limits the y-coordinate
 * to full pages (e.i. every 8th row)
 * \param x X-coordinate, (left = 0, right = 127)
 * \param ypage Y-coordinate (up = 0, down = 6)
 * \param c Character to be displayed
 */
void screen_FastChar12x16(char c, uint8_t x, uint8_t ypage) {
    if (x >= 128 || ypage >= 7)
        return;
    uint8_t i;
    for (i = 0; i < 12; i++) {
        screen_SetByte(x + i, ypage, font12x16[(uint8_t) c][i * 2 + 1]);
        screen_SetByte(x + i, ypage + 1, font12x16[(uint8_t) c][i * 2]);
    }
}

void screen_Char12x16(char c, uint8_t x, uint8_t y) {
    if (x >= 128 || y >= 64)
        return;

    uint8_t shiftUpper = y % 8;
    uint8_t maskUpper = 0xFF << shiftUpper;
    uint8_t shiftLower = 8 - shiftUpper;
    uint8_t maskLower = 0xFF >> shiftLower;

    uint8_t ypageUpper = y / 8;
    uint8_t ypageMiddle = ypageUpper + 1;
    uint8_t ypageLower = ypageMiddle + 1;

    uint8_t i;
    for (i = 0; i < 12; i++) {
        if (x + i >= 128)
            break;
        display.buffer[x + i + ypageUpper * 128] &= ~maskUpper;
        display.buffer[x + i + ypageUpper * 128] |= font12x16[(uint8_t) c][i * 2
                + 1] << shiftUpper;
        if (ypageMiddle >= 8)
            continue;
        display.buffer[x + i + ypageMiddle * 128] =
                font12x16[(uint8_t) c][i * 2] << shiftUpper;
        if (shiftUpper) {
            /* not aligned with page */
            display.buffer[x + i + ypageMiddle * 128] &= ~maskLower;
            display.buffer[x + i + ypageMiddle * 128] |=
                    font12x16[(uint8_t) c][i * 2 + 1] >> shiftLower;
            if (ypageLower >= 8)
                continue;
            display.buffer[x + i + ypageLower * 128] &= ~maskLower;
            display.buffer[x + i + ypageLower * 128] |= font12x16[(uint8_t) c][i
                    * 2] >> shiftLower;
        }
    }
    display.updateTime = timer_SetTimeout(SCREEN_DISPLAY_TIMEOUT);
}

void screen_InvertChar12x16(uint8_t x, uint8_t ypage) {
    if (x >= 128 || ypage >= 7)
        return;
    uint8_t i;
    for (i = 0; i < 12; i++) {
        display.buffer[x + i + ypage * 128] ^= 0xFF;
        display.buffer[x + i + (ypage + 1) * 128] ^= 0xFF;
        display.updateTime = timer_SetTimeout(SCREEN_DISPLAY_TIMEOUT);
    }
}

/**
 * \brief Writes a 6x8 font character into the display data buffer
 *
 * Function is fast because it transfers the character into the
 * buffer by copying the whole bytes. This also limits the y-coordinate
 * to full pages (e.i. every 8th row)
 * \param x X-coordinate, (left = 0, right = 127)
 * \param ypage Y-coordinate (up = 0, down = 7)
 * \param c Character to be displayed
 */
void screen_FastChar6x8(char c, uint8_t x, uint8_t ypage) {
    if (x >= 128 || ypage >= 8)
        return;
    uint8_t i;
    for (i = 0; i < 6; i++) {
        screen_SetByte(x + i, ypage, font6x8[(uint8_t) c][i]);
    }
}
void screen_Char6x8(char c, uint8_t x, uint8_t y) {
    if (x >= 128 || y >= 64)
        return;

    uint8_t shiftUpper = y % 8;
    uint8_t maskUpper = 0xFF << shiftUpper;
    uint8_t shiftLower = 8 - shiftUpper;
    uint8_t maskLower = 0xFF >> shiftLower;

    uint8_t ypageUpper = y / 8;
    uint8_t ypageLower = ypageUpper + 1;

    uint8_t i;
    for (i = 0; i < 6; i++) {
        if (x + i >= 128)
            break;
        display.buffer[x + i + ypageUpper * 128] &= ~maskUpper;
        display.buffer[x + i + ypageUpper * 128] |= font6x8[(uint8_t) c][i]
                << shiftUpper;
        if(ypageLower >= 8)
            continue;
        if (shiftUpper) {
            /* not aligned with page */
            display.buffer[x + i + ypageLower * 128] &= ~maskLower;
            display.buffer[x + i + ypageLower * 128] |= font6x8[(uint8_t) c][i]
                    >> shiftLower;
        }
    }
    display.updateTime = timer_SetTimeout(SCREEN_DISPLAY_TIMEOUT);
}

void screen_Char4x6(char c, uint8_t x, uint8_t y){
    if (x >= 128 || y >= 64)
        return;

    uint8_t shiftUpper = y % 8;
    uint8_t maskUpper = 0x3F << shiftUpper;
    uint8_t shiftLower = 8 - shiftUpper;
    uint8_t maskLower = 0x3F >> shiftLower;

    uint8_t ypageUpper = y / 8;
    uint8_t ypageLower = ypageUpper + 1;

    uint8_t i;
    for (i = 0; i < 4; i++) {
        if (x + i >= 128)
            break;
        display.buffer[x + i + ypageUpper * 128] &= ~maskUpper;
        display.buffer[x + i + ypageUpper * 128] |= (font4x6[(uint8_t) c][i]
                >> 2) << shiftUpper;
        if(ypageLower >= 8)
            continue;
        if (shiftUpper > 2) {
            /* not aligned with page */
            display.buffer[x + i + ypageLower * 128] &= ~maskLower;
            display.buffer[x + i + ypageLower * 128] |= (font4x6[(uint8_t) c][i]
                    >> 2) >> shiftLower;
        }
    }
    display.updateTime = timer_SetTimeout(SCREEN_DISPLAY_TIMEOUT);
}

void screen_InvertChar6x8(uint8_t x, uint8_t ypage) {
    if (x >= 128 || ypage >= 8)
        return;
    uint8_t i;
    for (i = 0; i < 6; i++) {
        display.buffer[x + i + ypage * 128] ^= 0xFF;
        display.updateTime = timer_SetTimeout(SCREEN_DISPLAY_TIMEOUT);
    }
}

/**
 * \brief Writes a 12x16 string into the display data buffer
 *
 * Uses screen_FastChar12x16(), thus it is also limited to
 * full pages in the y-coordinate.
 *
 * \param src Pointer to the string
 * \param x X-coordinate, (left = 0, right = 127)
 * \param ypage Y-coordinate (up = 0, down = 6)
 */
void screen_FastString12x16(const char *src, uint8_t x, uint8_t ypage) {
    while (*src) {
        screen_FastChar12x16(*src++, x, ypage);
        x += 12;
    }
}
void screen_String12x16(const char *src, uint8_t x, uint8_t y){
    while (*src) {
        screen_Char12x16(*src++, x, y);
        x += 12;
    }
}

/**
 * \brief Writes a 6x8 string into the display data buffer
 *
 * Uses screen_FastChar6x8(), thus it is also limited to
 * full pages in the y-coordinate.
 *
 * \param src Pointer to the string
 * \param x X-coordinate, (left = 0, right = 127)
 * \param ypage Y-coordinate (up = 0, down = 7)
 */
void screen_FastString6x8(const char *src, uint8_t x, uint8_t ypage) {
    while (*src) {
        screen_FastChar6x8(*src++, x, ypage);
        x += 6;
    }
}
void screen_String6x8(const char *src, uint8_t x, uint8_t y){
    while (*src) {
        screen_Char6x8(*src++, x, y);
        x += 6;
    }
}

void screen_String4x6(const char *src, uint8_t x, uint8_t y){
    while (*src) {
        screen_Char4x6(*src++, x, y);
        x += 4;
    }
}

void screen_String(const char *src, font_t font, uint8_t x, uint8_t y){
    switch(font) {
    case FONT_SMALL:
        screen_String4x6(src, x, y);
        break;
    case FONT_MEDIUM:
        screen_String6x8(src, x, y);
        break;
    case FONT_BIG:
        screen_String12x16(src, x, y);
        break;
    }
}

void screen_SetSoftButton(const char *descr, uint8_t num) {
    // calculate descr length to center text (up to 6 chars)
    uint8_t length;
    for (length = 0; descr[length] != 0; length++)
        ;
    // calculate text start
    uint8_t start = 20 + 44 * num - length * 6 / 2;
    // draw box outline
    screen_VerticalLine(44 * num, 55, 9);
    screen_VerticalLine(44 * num + 39, 55, 9);
    screen_HorizontalLine(44 * num + 1, 54, 38);
    // set text
    screen_FastString6x8(descr, start, 7);
}

void screen_Text6x8(const char *src, uint8_t x, uint8_t ypage) {
    uint8_t xbuf = x;
    while (*src) {
        // try to display next word
        char *c = src;
        uint8_t wordlength = 0;
        while (*c != ' ' && *c) {
            wordlength++;
            c++;
        }
        if (xbuf + wordlength * 6 > 128) {
            // word doesn't fit in any line
            // -> write it with line break
            while (*src != ' ' && *src) {
                screen_FastChar6x8(*src++, x, ypage);
                x += 6;
                if (x >= 128) {
                    ypage++;
                    x = xbuf;
                    if (ypage > 7) {
                        // screen is full -> abort
                        return;
                    }
                }
            }
        } else if (x + wordlength * 6 > 128) {
            // the word doesn't fit into current line
            // -> switch to next line
            x = xbuf;
            ypage++;
            if (ypage > 7) {
                // screen is full -> abort
                return;
            }
            while (*src != ' ' && *src) {
                screen_FastChar6x8(*src++, x, ypage);
                x += 6;
            }
        } else {
            // word fits into current line
            while (*src != ' ' && *src) {
                screen_FastChar6x8(*src++, x, ypage);
                x += 6;
            }
        }
        if (*src == ' ') {
            // skip spaces
            src++;
            if (x <= 116) {
                // enough room for 'space'
                x += 6;
            } else {
                // end of line, start next word at next line
                // -> switch to next line
                x = xbuf;
                ypage++;
                if (ypage > 7) {
                    // screen is full -> abort
                    return;
                }
            }
        }
    }
}

