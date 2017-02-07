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

void string_fromUintUnit(uint32_t value, char *dest, uint8_t digits, int8_t dot,
        char baseUnit) {
    // find position of first digit
    uint32_t divider = 1000000000UL;
    int8_t firstDigit = 10;
    for (; divider > 1; divider /= 10) {
        if (value / divider > 0)
            break;
        firstDigit--;
    }
    for (; firstDigit < digits; digits--) {
        *dest++ = ' ';
    }
    uint8_t _dot = dot;
    char prefix = 0;
    if (baseUnit) {
        // calculate prefix
        if (firstDigit > dot + 3) {
            dot += 3;
            prefix = 'k';
        } else if (firstDigit <= dot - 3 && dot >= 6) {
            dot -= 6;
            prefix = 'u';
        } else if (firstDigit <= dot && dot >= 3) {
            dot -= 3;
            prefix = 'm';
        }
    }
    if (firstDigit < dot || (dot <= firstDigit - digits && _dot)) {
        *dest++ = ' ';
    }
    // display value
    if (baseUnit && !prefix) {
        *dest++ = ' ';
    }
    for (; firstDigit > 0; firstDigit--) {
        if (!digits)
            break;
        if (firstDigit == dot) {
            *dest++ = '.';
        }
        *dest = value / divider;
        value -= *dest * divider;
        divider /= 10;
        *dest++ += '0';
        digits--;
    }
    if (baseUnit) {
        // display Unit
        if (prefix)
            *dest++ = prefix;
        *dest++ = baseUnit;
    }
    *dest = 0;
}

void string_fromUintUnits(uint32_t value, char *dest, uint8_t digits,
        const char *unit0, const char *unit3, const char *unit6) {
    // find position of first digit
    uint32_t divider = 1000000000UL;
    int8_t firstDigit = 10;
    for (; divider > 1; divider /= 10) {
        if (value / divider > 0)
            break;
        firstDigit--;
    }
    char prefix = 0;
    uint8_t dot = 0;
    char *unit = unit0;
    // calculate prefix
    if (firstDigit > 6 && unit6 && *unit6) {
        dot = 6;
        unit = unit6;
    } else if (firstDigit > 3 && unit3 && *unit3) {
        dot = 3;
        unit = unit3;
    } else if (!unit0 || !*unit0) {
        /* no dot necessary but no unit0 specified */
        if (unit3 && *unit3 && digits > 3) {
            unit = unit3;
            dot = 3;
            while (firstDigit < 4) {
                divider *= 10;
                firstDigit++;
            }
        }
    }
    for (; firstDigit < digits; digits--) {
        *dest++ = ' ';
    }
    if (!dot) {
        *dest++ = ' ';
    }
    for (; firstDigit > 0; firstDigit--) {
        if (!digits)
            break;
        if (firstDigit == dot) {
            *dest++ = '.';
        }
        *dest = value / divider;
        value -= *dest * divider;
        divider /= 10;
        *dest++ += '0';
        digits--;
    }
    while (*unit) {
        *dest++ = *unit++;
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
