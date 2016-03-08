/**
 * \file
 * \brief   String conversion header file.
 */
#ifndef STRINGFUNCTIONS_H_
#define STRINGFUNCTIONS_H_

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

/**
 * \brief copies a string from the FLASH into a char array
 *
 * Copies bytes from src to dest until it reaches a string terminator (the terminator
 * itself will also be copied)
 *
 * \param *dest Pointer to destination array (must be large enough to hold the string)
 * \param *src String to be copied into the array
 */
void string_copy(char *dest, const char *src);

/**
 * \brief copies a string of bytes from the FLASH into a char array
 *
 * Like string_copy() but with an maximum limit of copied bytes and the terminator won't be copied
 *
 * \param *dest Pointer to destination array (must be large enough to hold the string)
 * \param *src String to be copied into the array
 * \param n Maximum number of copied bytes
 */
void string_copyn(char *dest, const char *src, uint8_t n);

#endif
