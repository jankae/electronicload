#include "units.h"

const char unitNames[UNIT_NUM][3][4] = {
    {"uA", "mA", "A"},
    {"uV", "mV", "V"},
    {"mR", "R", "kR"},
    {"uW", "mW", "W"},
    {"ms", "s", "\0"},
    {"mHz", "Hz", "kHz"},
    {"uWh", "mWh", "Wh"},
    {"m\xF8", "\xF8", "\0"},
    {"\x23", "\0", "\0"}
};
