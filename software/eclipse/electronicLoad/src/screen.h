/**
 * \file
 * \brief   High level display header file.
 *
 * Contains functions to write and draw on the display.
 * All functions don't modify the actual display content
 * but rather the internal display buffer from display.h
 */
#ifndef SCREEN_H_
#define SCREEN_H_

#include <stringFunctions.h>
#include "GUI/font.h"
#include "display.h"
#include "currentSink.h"
#include "calibration.h"
#include "loadFunctions.h"

#define SCREEN_HEIGHT               64
#define SCREEN_WIDTH                128
#define SCREEN_DISPLAY_TIMEOUT      10

typedef enum {
    PIXEL_OFF, PIXEL_ON, PIXEL_INVERT
} PixelState_t;

/**
 * \brief Clears the entire display
 */
void screen_Clear(void);

/*
 * \brief Sets or clears a specific pixel in the display data buffer
 *
 * \param x X-coordinate, (left = 0, right = 127)
 * \param y Y-coordinate, (up = 0, down = 63)
 * \param s new pixel state (PIXEL_OFF or PIXEL_ON)
 */
void screen_SetPixel(uint8_t x, uint8_t y, PixelState_t s);

/**
 * \brief Writes an entire byte (8 vertical pixel) in the display data buffer
 *
 * \param x X-coordinate, (left = 0, right = 127)
 * \param page Y-coordinate (up = 0, down = 7)
 * \param b Byte containing the pixeldata
 */
void screen_SetByte(uint8_t x, uint8_t page, uint8_t b);

/**
 * \brief Draws a vertical line on the screen
 *
 * \param x         X-coordinate of line
 * \param y         Y-coordinate of upper line end
 * \param length    length of the line in pixels
 */
void screen_VerticalLine(uint8_t x, uint8_t y, uint8_t length);

/**
 * \brief Draws a horizontal line on the screen
 *
 * \param x         X-coordinate of left line end
 * \param y         Y-coordinate
 * \param length    length of the line in pixels
 */
void screen_HorizontalLine(uint8_t x, uint8_t y, uint8_t length);


/**
 * \brief Draws an arbitrary line on the screen
 *
 * Draws a line using the Bresenham algorithm
 * \param x0        beginning of the line X-coordinate
 * \param y0        beginning of the line Y-coordinate
 * \param x1        end of the line X-coordinate
 * \param y1        end of the line Y-coordinate
 */
void screen_Line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);

void screen_Circle(int x0, int y0, int radius);

/**
 * \brief Draws a rectangle on the screen
 *
 * \param x1    X-coordinate of top left corner
 * \param y1    Y-coordinate of top left corner
 * \param x2    X-coordinate of bottom right corner
 * \param y2    Y-coordinate of bottom right corner
 */
void screen_Rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void screen_FullRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, PixelState_t p);

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
void screen_FastChar12x16(char c, uint8_t x, uint8_t ypage);
void screen_Char12x16(char c, uint8_t x, uint8_t y);

void screen_InvertChar12x16(uint8_t x, uint8_t ypage);

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
void screen_FastChar6x8(char c, uint8_t x, uint8_t ypage);
void screen_Char6x8(char c, uint8_t x, uint8_t y);

void screen_Char4x6(char c, uint8_t x, uint8_t y);

void screen_InvertChar6x8(uint8_t x,uint8_t ypage);

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
void screen_FastString12x16(const char *src, uint8_t x, uint8_t ypage);
void screen_String12x16(const char *src, uint8_t x, uint8_t y);

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
void screen_FastString6x8(const char *src, uint8_t x, uint8_t ypage);
void screen_String6x8(const char *src, uint8_t x, uint8_t y);

void screen_String4x6(const char *src, uint8_t x, uint8_t y);

void screen_String(const char *src, font_t font, uint8_t x, uint8_t y);
/**
 * \brief Displays a soft button
 *
 * \param descr Name of the soft button (up to 6 characters)
 * \num Number (and thus also position) of the button (0-2)
 */
void screen_SetSoftButton(const char *descr, uint8_t num);

void screen_Text6x8(const char *src, uint8_t x, uint8_t ypage);



#endif
