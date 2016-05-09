#include "common.h"

int32_t common_Map(int32_t value, int32_t scaleFromLow, int32_t scaleFromHigh,
        int32_t scaleToLow, int32_t scaleToHigh) {
    int32_t result;
    value -= scaleFromLow;
    int32_t rangeFrom = scaleFromHigh - scaleFromLow;
    value *= 16384;
    value /= rangeFrom;
    // value is now mapped to 0-16384
    int32_t rangeTo = scaleToHigh - scaleToLow;
    result = rangeTo * value;
    result /= 16384;
    result += scaleToLow;
    return result;
}
