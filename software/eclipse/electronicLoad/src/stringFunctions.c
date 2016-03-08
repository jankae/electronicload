/**
 * \file
 * \brief   String conversion source file.
 */
#include <stringFunctions.h>

/**
 * \brief Creates a string from an unsigned integer
 *
 * The string be will be created with leading zeros
 * and an optional fixed decimal point
 * \param value     Integer value which will be written to string
 * \param dest      Pointer to (big enough!) char array
 * \param digits    Number of displayed digits (not counting the decimal point).
 *                  If value has more digits than specified only the last
 *                  digits will be displayed. If value has less digits the
 *                  string will be filled with leading zeros
 * \param dot       Number of digits behind decimal point. If 0 the decimal point
 *                  will be omitted
 */
void string_fromUint(uint32_t value, char *dest, uint8_t digits, uint8_t dot) {
    uint32_t divider = 1;
    uint8_t i;
    for (i = 1; i < digits; i++)
        divider *= 10;

    for (i = digits; i > 0; i--) {
        if (i == dot)
            *dest++ = '.';
        uint8_t c = value / divider;
        *dest++ = (c % 10) + '0';
        value -= c * divider;
        divider /= 10;
    }
    *dest = 0;
}

void string_copy(char *dest, const char *src) {
    while (*src) {
        *dest++ = *src++;
    }
    *dest = 0;
}

void string_copyn(char *dest, const char *src, uint8_t n) {
    for (; n > 0; n++) {
        if (!*src)
            break;
        *dest++ = *src++;
    }
}
int string_compare(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        int diff = *s1++ - *s2++;
        if (diff)
            return diff;
    }
    return 0;
}
