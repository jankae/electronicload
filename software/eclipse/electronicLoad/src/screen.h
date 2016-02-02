
#ifndef SCREEN_H_
#define SCREEN_H_

#include "display.h"
#include "string.h"
#include "currentSink.h"

typedef enum {PIXEL_OFF, PIXEL_ON} PixelState_t;

struct {
	// buffer for the two lines of the default screen
	char defScreen[2][21];
} screen;

/*
 * clears the entire dispaly
 */
void screen_Clear(void);

/*
 * sets or clears a specific pixel in the display data buffer
 * (no data is actual transmitted to the display, only the *internal*
 * buffer is modified)
 */
void screen_SetPixel(uint8_t x, uint8_t y, PixelState_t s);

void screen_SetByte(uint8_t x, uint8_t page, uint8_t b);

/*
 * transfers a character into the display buffer by copying the whole bytes
 * (fast but limited to 8 lines in y direction)
 * 0<=x<=115
 * 0<=y<=6
 */
void screen_FastChar12x16(uint8_t x, uint8_t ypage, char c);

/*
 * transfers a character into the display buffer by copying the whole bytes
 * (fast but limited to 8 lines in y direction)
 * 0<=x<=121
 * 0<=y<=7
 */
void screen_FastChar6x8(uint8_t x, uint8_t ypage, char c);

void screen_FastString12x16(char *src, uint8_t x, uint8_t ypage);
void screen_FastString6x8(char *src, uint8_t x, uint8_t ypage);

void screen_UpdateDefaultScreen(void);

#endif
