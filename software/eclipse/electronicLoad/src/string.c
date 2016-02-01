#include "string.h"

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
}
