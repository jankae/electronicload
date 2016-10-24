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
 * \brief Creates a string from an unsigned integer and includes the unit
 *
 * The string be will be created without leading zeros
 * and an optional fixed decimal point which might be shifted to get
 * optimal results. In this case a prefix is automatically added to the baseUnit
 * \param value     Integer value which will be written to string
 * \param dest      Pointer to char array. The array must be at least digits+4 long
 *                  (1 decimal point/space holder + 1 prefix/spaceholder + baseUnit
 *                  + string terminator)
 * \param digits    Number of displayed digits (not counting the decimal point).
 *                  If value has more digits than specified only the first digits
 *                  will displayed. If value has less digits the first characters
 *                  will be left blank
 * \param dot       Number of digits behind decimal point at baseUnit
 *                  (e.g. if 10000 denotes 10A, with A being the baseUnit,
 *                  dot has to be 3)
 */
void string_fromUintUnit(uint32_t value, char *dest, uint8_t digits, int8_t dot,
        char baseUnit);

/**
 * \brief Creates a string from an unsigned integer and includes the unit
 *
 * Similar to string_fromUintUnit but no decimal point can be specified and no
 * prefixes are added. Instead the user supplies the units for 10^0, 10^3 and 10^6 LSB.
 * The value will be displayed with a maximum of 3 digits before the decimal point
 * (provided the unit for that decimal point position is != NULL)
 * \param value     Integer value which will be written to string
 * \param dest      Pointer to char array. The array must be long enough to hold
 *                  the result. Result length can vary.
 * \param digits    Number of displayed digits (not counting the decimal point).
 *                  If value has more digits than specified only the first digits
 *                  will displayed. If value has less digits the first characters
 *                  will be left blank
 * \param *unit0    LSB*10^0 unit (e.g. "uA" if LSB denotes uA)
 * \param *unit3    LSB*10^3 unit (e.g. "mA" if LSB denotes uA)
 * \param *unit6    LSB*10^6 unit (e.g. "A" if LSB denotes uA)
 */
void string_fromUintUnits(uint32_t value, char *dest, uint8_t digits,
        char *unit0, char *unit3, char *unit6);

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

/**
 * \brief comares two strings
 *
 * Like strcmp, but terminates with 0 if it reaches a string terminator (0) in either of
 * the input strings (provided they are equal up to that point)
 *
 * \param *s1 Pointer to the first string
 * \param *s2 Pointer to the second string
 */
int string_compare(const char *s1, const char *s2);

#endif
