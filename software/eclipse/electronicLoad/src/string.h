/**
 * \file
 * \brief   String conversion header file.
 */
#ifndef STRING_H_
#define STRING_H_

#include <stdint.h>

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
void string_fromUint(uint32_t value, char *dest, uint8_t digits, uint8_t dot);

#endif
