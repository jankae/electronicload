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

#include "display.h"
#include "string.h"
#include "currentSink.h"
#include "calibration.h"

typedef enum {
    PIXEL_OFF, PIXEL_ON
} PixelState_t;

struct {
    // buffer for the two lines of the default screen
    char defScreen[2][21];
} screen;

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
 * \brief Draws a rectangle on the screen
 *
 * \param x1    X-coordinate of top left corner
 * \param y1    Y-coordinate of top left corner
 * \param x2    X-coordinate of bottom right corner
 * \param y2    Y-coordinate of bottom right corner
 */
void screen_Rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);

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
void screen_FastChar12x16(uint8_t x, uint8_t ypage, char c);

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
void screen_FastChar6x8(uint8_t x, uint8_t ypage, char c);

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
void screen_FastString12x16(char *src, uint8_t x, uint8_t ypage);

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
void screen_FastString6x8(char *src, uint8_t x, uint8_t ypage);

/**
 * \brief Copies a char array into the default screen
 *
 * \param *src      Pointer to the array to be read from
 * \param x         Startposition in the default screen line
 * \param line      Choose between line 0 and 1 of the default screen
 */
void screen_SetDefaultScreenString(char *src, uint8_t x, uint8_t line);

/**
 * \brief Fills the display with the default screen
 *
 * The first three lines consist of the measured
 * voltage, current and power currently sinked.
 * In the last to lines the content of screen.defscreen
 * will be displayed
 */
void screen_UpdateDefaultScreen(void);

#endif
