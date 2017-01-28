#ifndef GUI_FONT_H_
#define GUI_FONT_H_

#include <stdint.h>

#define FONT_NUM_SIZES      3

typedef enum {
    FONT_SMALL = 0, FONT_MEDIUM = 1, FONT_BIG = 2
} font_t;

typedef struct {
    uint8_t width;
    uint8_t height;
} fontSize_t;

extern const fontSize_t fontSize[FONT_NUM_SIZES];

extern const uint8_t font12x16[256][24];
extern const uint8_t font6x8[256][6];
extern const uint8_t font4x6[256][4];
#endif
