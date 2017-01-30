#include "waveforms.h"

static const uint16_t wave_SineLookup[1025] = { 0, 101, 201, 302, 402, 503, 603,
        704, 804, 905, 1005, 1106, 1206, 1307, 1407, 1508, 1608, 1709, 1809,
        1910, 2010, 2111, 2211, 2312, 2412, 2513, 2613, 2714, 2814, 2914, 3015,
        3115, 3216, 3316, 3416, 3517, 3617, 3718, 3818, 3918, 4019, 4119, 4219,
        4320, 4420, 4520, 4621, 4721, 4821, 4921, 5022, 5122, 5222, 5322, 5422,
        5523, 5623, 5723, 5823, 5923, 6023, 6123, 6223, 6324, 6424, 6524, 6624,
        6724, 6824, 6924, 7024, 7123, 7223, 7323, 7423, 7523, 7623, 7723, 7823,
        7922, 8022, 8122, 8222, 8321, 8421, 8521, 8620, 8720, 8820, 8919, 9019,
        9118, 9218, 9318, 9417, 9517, 9616, 9715, 9815, 9914, 10014, 10113,
        10212, 10312, 10411, 10510, 10609, 10708, 10808, 10907, 11006, 11105,
        11204, 11303, 11402, 11501, 11600, 11699, 11798, 11897, 11996, 12094,
        12193, 12292, 12391, 12489, 12588, 12687, 12785, 12884, 12982, 13081,
        13179, 13278, 13376, 13475, 13573, 13671, 13770, 13868, 13966, 14064,
        14163, 14261, 14359, 14457, 14555, 14653, 14751, 14849, 14947, 15045,
        15142, 15240, 15338, 15436, 15533, 15631, 15729, 15826, 15924, 16021,
        16119, 16216, 16313, 16411, 16508, 16605, 16703, 16800, 16897, 16994,
        17091, 17188, 17285, 17382, 17479, 17576, 17673, 17769, 17866, 17963,
        18060, 18156, 18253, 18349, 18446, 18542, 18639, 18735, 18831, 18928,
        19024, 19120, 19216, 19312, 19408, 19504, 19600, 19696, 19792, 19888,
        19984, 20079, 20175, 20271, 20366, 20462, 20557, 20653, 20748, 20843,
        20939, 21034, 21129, 21224, 21319, 21414, 21509, 21604, 21699, 21794,
        21889, 21983, 22078, 22173, 22267, 22362, 22456, 22551, 22645, 22739,
        22834, 22928, 23022, 23116, 23210, 23304, 23398, 23492, 23586, 23679,
        23773, 23867, 23960, 24054, 24147, 24241, 24334, 24428, 24521, 24614,
        24707, 24800, 24893, 24986, 25079, 25172, 25265, 25358, 25450, 25543,
        25635, 25728, 25820, 25913, 26005, 26097, 26189, 26281, 26374, 26466,
        26557, 26649, 26741, 26833, 26925, 27016, 27108, 27199, 27291, 27382,
        27473, 27565, 27656, 27747, 27838, 27929, 28020, 28111, 28201, 28292,
        28383, 28473, 28564, 28654, 28745, 28835, 28925, 29015, 29106, 29196,
        29286, 29375, 29465, 29555, 29645, 29734, 29824, 29913, 30003, 30092,
        30181, 30271, 30360, 30449, 30538, 30627, 30716, 30804, 30893, 30982,
        31070, 31159, 31247, 31335, 31424, 31512, 31600, 31688, 31776, 31864,
        31952, 32039, 32127, 32215, 32302, 32390, 32477, 32564, 32651, 32738,
        32826, 32912, 32999, 33086, 33173, 33260, 33346, 33433, 33519, 33605,
        33692, 33778, 33864, 33950, 34036, 34122, 34208, 34293, 34379, 34465,
        34550, 34635, 34721, 34806, 34891, 34976, 35061, 35146, 35231, 35315,
        35400, 35485, 35569, 35654, 35738, 35822, 35906, 35990, 36074, 36158,
        36242, 36326, 36409, 36493, 36576, 36660, 36743, 36826, 36909, 36992,
        37075, 37158, 37241, 37324, 37406, 37489, 37571, 37653, 37736, 37818,
        37900, 37982, 38064, 38145, 38227, 38309, 38390, 38472, 38553, 38634,
        38715, 38797, 38877, 38958, 39039, 39120, 39200, 39281, 39361, 39442,
        39522, 39602, 39682, 39762, 39842, 39922, 40001, 40081, 40161, 40240,
        40319, 40398, 40478, 40557, 40635, 40714, 40793, 40872, 40950, 41029,
        41107, 41185, 41263, 41341, 41419, 41497, 41575, 41653, 41730, 41808,
        41885, 41962, 42039, 42117, 42194, 42270, 42347, 42424, 42500, 42577,
        42653, 42730, 42806, 42882, 42958, 43034, 43109, 43185, 43261, 43336,
        43411, 43487, 43562, 43637, 43712, 43787, 43861, 43936, 44011, 44085,
        44159, 44234, 44308, 44382, 44456, 44529, 44603, 44677, 44750, 44824,
        44897, 44970, 45043, 45116, 45189, 45262, 45334, 45407, 45479, 45552,
        45624, 45696, 45768, 45840, 45912, 45983, 46055, 46126, 46198, 46269,
        46340, 46411, 46482, 46553, 46624, 46694, 46765, 46835, 46905, 46976,
        47046, 47116, 47185, 47255, 47325, 47394, 47464, 47533, 47602, 47671,
        47740, 47809, 47877, 47946, 48014, 48083, 48151, 48219, 48287, 48355,
        48423, 48491, 48558, 48626, 48693, 48760, 48827, 48894, 48961, 49028,
        49095, 49161, 49228, 49294, 49360, 49426, 49492, 49558, 49624, 49689,
        49755, 49820, 49885, 49951, 50016, 50080, 50145, 50210, 50274, 50339,
        50403, 50467, 50531, 50595, 50659, 50723, 50787, 50850, 50913, 50977,
        51040, 51103, 51166, 51228, 51291, 51354, 51416, 51478, 51540, 51602,
        51664, 51726, 51788, 51849, 51911, 51972, 52033, 52094, 52155, 52216,
        52277, 52337, 52398, 52458, 52518, 52578, 52638, 52698, 52758, 52817,
        52877, 52936, 52995, 53054, 53113, 53172, 53231, 53289, 53348, 53406,
        53464, 53522, 53580, 53638, 53696, 53754, 53811, 53868, 53925, 53983,
        54039, 54096, 54153, 54210, 54266, 54322, 54378, 54434, 54490, 54546,
        54602, 54657, 54713, 54768, 54823, 54878, 54933, 54988, 55042, 55097,
        55151, 55206, 55260, 55314, 55367, 55421, 55475, 55528, 55582, 55635,
        55688, 55741, 55794, 55846, 55899, 55951, 56003, 56056, 56108, 56159,
        56211, 56263, 56314, 56366, 56417, 56468, 56519, 56570, 56620, 56671,
        56721, 56772, 56822, 56872, 56922, 56972, 57021, 57071, 57120, 57169,
        57218, 57267, 57316, 57365, 57413, 57462, 57510, 57558, 57606, 57654,
        57702, 57749, 57797, 57844, 57891, 57938, 57985, 58032, 58079, 58125,
        58171, 58218, 58264, 58310, 58356, 58401, 58447, 58492, 58537, 58583,
        58628, 58672, 58717, 58762, 58806, 58850, 58895, 58939, 58983, 59026,
        59070, 59113, 59157, 59200, 59243, 59286, 59329, 59371, 59414, 59456,
        59498, 59540, 59582, 59624, 59666, 59707, 59749, 59790, 59831, 59872,
        59913, 59953, 59994, 60034, 60075, 60115, 60155, 60194, 60234, 60274,
        60313, 60352, 60391, 60430, 60469, 60508, 60546, 60585, 60623, 60661,
        60699, 60737, 60775, 60812, 60850, 60887, 60924, 60961, 60998, 61034,
        61071, 61107, 61144, 61180, 61216, 61252, 61287, 61323, 61358, 61393,
        61429, 61463, 61498, 61533, 61567, 61602, 61636, 61670, 61704, 61738,
        61772, 61805, 61838, 61872, 61905, 61938, 61970, 62003, 62035, 62068,
        62100, 62132, 62164, 62196, 62227, 62259, 62290, 62321, 62352, 62383,
        62414, 62444, 62475, 62505, 62535, 62565, 62595, 62625, 62654, 62684,
        62713, 62742, 62771, 62800, 62829, 62857, 62886, 62914, 62942, 62970,
        62998, 63025, 63053, 63080, 63107, 63134, 63161, 63188, 63214, 63241,
        63267, 63293, 63319, 63345, 63371, 63396, 63422, 63447, 63472, 63497,
        63522, 63546, 63571, 63595, 63620, 63644, 63668, 63691, 63715, 63738,
        63762, 63785, 63808, 63831, 63853, 63876, 63898, 63921, 63943, 63965,
        63986, 64008, 64030, 64051, 64072, 64093, 64114, 64135, 64155, 64176,
        64196, 64216, 64236, 64256, 64276, 64295, 64315, 64334, 64353, 64372,
        64391, 64409, 64428, 64446, 64464, 64482, 64500, 64518, 64535, 64553,
        64570, 64587, 64604, 64621, 64638, 64654, 64671, 64687, 64703, 64719,
        64734, 64750, 64765, 64781, 64796, 64811, 64826, 64840, 64855, 64869,
        64883, 64898, 64911, 64925, 64939, 64952, 64966, 64979, 64992, 65005,
        65017, 65030, 65042, 65054, 65066, 65078, 65090, 65102, 65113, 65125,
        65136, 65147, 65158, 65168, 65179, 65189, 65199, 65210, 65219, 65229,
        65239, 65248, 65258, 65267, 65276, 65285, 65293, 65302, 65310, 65319,
        65327, 65335, 65342, 65350, 65357, 65365, 65372, 65379, 65386, 65392,
        65399, 65405, 65412, 65418, 65424, 65429, 65435, 65441, 65446, 65451,
        65456, 65461, 65466, 65470, 65475, 65479, 65483, 65487, 65491, 65494,
        65498, 65501, 65504, 65507, 65510, 65513, 65515, 65518, 65520, 65522,
        65524, 65526, 65527, 65529, 65530, 65531, 65532, 65533, 65534, 65534,
        65535, 65535, 65535 };

static const char waveform_Names[4][9] = { "SINE", "SAW", "SQUARE", "TRIANGLE" };
static const char *waveformItems[5] = { waveform_Names[0], waveform_Names[1],
        waveform_Names[2], waveform_Names[3], NULL };

static const char waveSetParamNames[4][11] = { "CURRENT", "VOLTAGE",
        "RESISTANCE", "POWER" };
static const char *paramItems[5] = { waveSetParamNames[0], waveSetParamNames[1],
        waveSetParamNames[2], waveSetParamNames[3], NULL };

static uint32_t *waveSetParamPointers[4] = { &load.current, &load.voltage,
        &load.resistance, &load.power };

static container_t c;
static dropdown_t dWave, dParam;
static entry_t eMin, eMax, eAmplitude, eOffset, ePeriod, eFreq;
static label_t lWave, lParam, lMin, lMax, lAmplitude, lOffset, lPeriod, lFreq;
static checkbox_t bOn;

static uint32_t minPeriod = 1, maxPeriod = 500000;
static uint32_t minFreq = 2, maxFreq = 1000000;

void waveform_Init(void) {
    waveform.amplitude = 1000;
    waveform.form = WAVE_SINE;
    waveform.switchedOn = 0;
    waveform.offset = 1000;
    waveform_AmplitudeOffsetChanged();
    waveform.param = &load.current;
    waveform.paramNum = 0;
    waveform.period = 1000;
    waveform_PeriodChanged();

    /* create GUI elements */
    label_create(&lWave, "Wave:", FONT_MEDIUM);
    label_create(&lParam, "Param:", FONT_MEDIUM);
    label_create(&lMin, "Min:", FONT_MEDIUM);
    label_create(&lMax, "Max:", FONT_MEDIUM);
    label_create(&lAmplitude, "Amplitude:", FONT_MEDIUM);
    label_create(&lOffset, "Offset:", FONT_MEDIUM);
    label_create(&lPeriod, "Period:", FONT_MEDIUM);
    label_create(&lFreq, "Frequency:", FONT_MEDIUM);

    checkbox_create(&bOn, &waveform.switchedOn, NULL);

    dropdown_create(&dWave, waveformItems, &waveform.form, FONT_MEDIUM, 0,
            waveform_WaveChanged);
    dropdown_create(&dParam, paramItems, &waveform.paramNum, FONT_MEDIUM, 0,
            waveform_ParamChanged);

    entry_create(&eMin, &waveform.min, NULL, NULL, FONT_MEDIUM, 4, UNIT_CURRENT,
            waveform_MinMaxChanged);
    entry_create(&eMax, &waveform.max, NULL, NULL, FONT_MEDIUM, 4, UNIT_CURRENT,
            waveform_MinMaxChanged);
    entry_create(&eAmplitude, &waveform.amplitude, NULL, NULL, FONT_MEDIUM, 4,
            UNIT_CURRENT, waveform_AmplitudeOffsetChanged);
    entry_create(&eOffset, &waveform.offset, NULL, NULL, FONT_MEDIUM, 4,
            UNIT_CURRENT, waveform_AmplitudeOffsetChanged);
    entry_create(&ePeriod, &waveform.period, &maxPeriod, &minPeriod,
            FONT_MEDIUM, 4, UNIT_TIME, waveform_PeriodChanged);
    entry_create(&eFreq, &waveform.frequency, &maxFreq, &minFreq, FONT_MEDIUM,
            4, UNIT_FREQ, waveform_FrequencyChanged);

    container_create(&c, 128, 55);

    container_attach(&c, &lWave, 0, 2);
    container_attach(&c, &lParam, 0, 14);
    container_attach(&c, &lMin, 0, 26);
    container_attach(&c, &lMax, 0, 38);
    container_attach(&c, &lAmplitude, 0, 50);
    container_attach(&c, &lOffset, 0, 62);
    container_attach(&c, &lPeriod, 0, 74);
    container_attach(&c, &lFreq, 0, 86);

    container_attach(&c, &bOn, 36, 0);
    container_attach(&c, &dWave, 48, 0);
    container_attach(&c, &dParam, 36, 12);
    container_attach(&c, &eMin, 60, 24);
    container_attach(&c, &eMax, 60, 36);
    container_attach(&c, &eAmplitude, 60, 48);
    container_attach(&c, &eOffset, 60, 60);
    container_attach(&c, &ePeriod, 60, 72);
    container_attach(&c, &eFreq, 60, 84);
}

widget_t* waveform_getWidget(void) {
    return (widget_t*) &c;
}

void waveform_WaveChanged(void) {

}

void waveform_ParamChanged(void) {
    /* change unit of waveform entries */
    eMin.unit = waveform.paramNum;
    eMax.unit = waveform.paramNum;
    eAmplitude.unit = waveform.paramNum;
    eOffset.unit = waveform.paramNum;
    waveform.param = waveSetParamPointers[waveform.paramNum];
}

void waveform_MinMaxChanged(void) {
    /* calculate offset and amplitude from min/max */
    waveform.offset = (waveform.min + waveform.max) / 2;
    waveform.amplitude = waveform.max - waveform.offset;
}

void waveform_AmplitudeOffsetChanged(void) {
    /* calculate min/max from amplitude/offset */
    waveform.min = waveform.offset - waveform.amplitude;
    if (waveform.min < 0) {
        waveform.min = 0;
    }
    waveform.max = waveform.offset + waveform.amplitude;
}

void waveform_PeriodChanged(void) {
    /* calculate frequency based on period */
    waveform.frequency = 1000000UL / waveform.period;
}

void waveform_FrequencyChanged(void) {
    /* calculate period based on frequency */
    waveform.period = 1000000UL / waveform.frequency;
    waveform_PeriodChanged();
}

void waveform_Update(void) {
    static uint32_t phaseAcc;
    phaseAcc += (UINT32_MAX / waveform.period);
    waveform.phase = phaseAcc >> 16;

    if (!waveform.switchedOn)
        return;
    if (waveform.param) {
        *(waveform.param) = waveform_GetValue(waveform.phase);
    }
}

int32_t waveform_GetValue(uint16_t wavetime) {
    int32_t value;
    switch (waveform.form) {
    case WAVE_SQUARE:
        if (wavetime < 32768)
            value = waveform.amplitude;
        else
            value = -waveform.amplitude;
        break;
    case WAVE_SAW:
        value = -waveform.amplitude
                + ((int64_t) wavetime * waveform.amplitude * 2) / 65536;
        break;
    case WAVE_TRIANGLE:
        if (wavetime >= 32768)
            wavetime = 65535 - wavetime;
        value = -waveform.amplitude
                + ((int64_t) wavetime * waveform.amplitude * 2) / 32768;
        break;
    case WAVE_SINE:
        value = ((int64_t) waveform_Sine(wavetime) * waveform.amplitude)
                / 65536;
        break;
    }
    value += waveform.offset;
    return value;
}

int32_t waveform_Sine(uint16_t arg) {
    int8_t sign;
    if (arg >= 49152) {
        sign = -1;
        arg = 65535 - arg;
    } else if (arg >= 32768) {
        sign = -1;
        arg = arg - 32768;
    } else if (arg >= 16384) {
        sign = 1;
        arg = 32767 - arg;
    } else {
        sign = 1;
    }
    int32_t valueLow = wave_SineLookup[arg / 16];
    int32_t valueHigh = wave_SineLookup[arg / 16 + 1];
    arg %= 16;
    int32_t retvalue = valueLow * (16 - arg) + valueHigh * arg;
    retvalue /= 16;
    return retvalue * sign;
}

//void waveform_Menu(void) {
//    uint8_t selectedRow = 1;
//    uint32_t button;
//    int32_t encoder;
//    do {
//        while (hal_getButton())
//            ;
//        // create menu display
//        screen_Clear();
//        screen_FastString6x8("\xCD\xCD\xCD\xCDWAVEFORM MENU\xCD\xCD\xCD\xCD", 0,
//                0);
//        screen_FastString6x8("Waveform:", 6, 1);
//        screen_FastString6x8(waveform_Names[waveform.form], 66, 1);
//
//        char baseUnit;
//        uint8_t dotPosition;
//        uint32_t maxValue;
//        uint32_t minValue;
//        char unit0_s[5];
//        char unit3_s[5];
//        char unit6_s[5];
//        char *unit0 = unit0_s;
//        char *unit3 = unit3_s;
//        char *unit6 = unit6_s;
//        switch (waveform.paramNum) {
//        case 0:
//            baseUnit = 'A';
//            dotPosition = 6;
//            maxValue = settings.maxCurrent;
//            minValue = 0;
//            unit0 = NULL;
//            strcpy(unit3_s, "mA");
//            strcpy(unit6_s, "A");
//            break;
//        case 1:
//            baseUnit = 'V';
//            dotPosition = 6;
//            maxValue = settings.maxVoltage;
//            minValue = settings.minVoltage;
//            unit0 = NULL;
//            strcpy(unit3_s, "mV");
//            strcpy(unit6_s, "V");
//            break;
//        case 2:
//            baseUnit = 'R';
//            dotPosition = 3;
//            maxValue = settings.maxResistance;
//            minValue = settings.minResistance;
//            strcpy(unit0_s, "mOhm");
//            strcpy(unit3_s, "Ohm");
//            strcpy(unit6_s, "kOhm");
//            break;
//        case 3:
//            baseUnit = 'W';
//            dotPosition = 6;
//            maxValue = settings.maxPower;
//            minValue = 0;
//            unit0 = NULL;
//            strcpy(unit3, "mW");
//            strcpy(unit6, "W");
//            break;
//        }
//
//        screen_FastString6x8("Amplitude:", 6, 2);
//        char value[11];
//        string_fromUintUnit(waveform.amplitude, value, 4, dotPosition,
//                baseUnit);
//        screen_FastString6x8(value, 66, 2);
//
//        screen_FastString6x8("Offset:", 6, 3);
//        string_fromUintUnit(waveform.offset, value, 4, dotPosition, baseUnit);
//        screen_FastString6x8(value, 66, 3);
//
//        screen_FastString6x8("Period:", 6, 4);
//        string_fromUintUnit(waveform.period, value, 4, 3, 's');
//        screen_FastString6x8(value, 66, 4);
//
//        screen_FastString6x8("Param:", 6, 5);
//        screen_FastString6x8(waveSetParamNames[waveform.paramNum], 66, 5);
//
//        // display selected line
//        screen_FastChar6x8(0x1A, 0, selectedRow);
//
//        /*
//         * display waveform in last two rows
//         */
//        // display offset line
//        uint8_t i;
//        for (i = 0; i < 128; i += 2) {
//            screen_SetPixel(i, 55, PIXEL_ON);
//        }
//        // display period lines
//        for (i = 48; i < 64; i += 2) {
//            screen_SetPixel(32, i, PIXEL_ON);
//            screen_SetPixel(96, i, PIXEL_ON);
//        }
//        // display waveform
//        uint8_t y_last = -(waveform_GetValue(32768) - waveform.offset) * 8
//                / waveform.amplitude + 55;
//        for (i = 1; i < 128; i++) {
//            uint8_t y = -(waveform_GetValue(i * 1024 + 32768) - waveform.offset)
//                    * 8 / waveform.amplitude + 55;
//            screen_Line(i, y, i - 1, y_last);
//            y_last = y;
//        }
//
//        // wait for user input
//        do {
//            button = hal_getButton();
//            encoder = hal_getEncoderMovement();
//        } while (!button && !encoder);
//
//        if ((button & HAL_BUTTON_DOWN) || encoder > 0) {
//            // move one entry down (if possible)
//            if (selectedRow < 5)
//                selectedRow++;
//        }
//
//        if ((button & HAL_BUTTON_UP) || encoder < 0) {
//            // move one entry up
//            if (selectedRow > 1) {
//                selectedRow--;
//            }
//        }
//
//        if ((button & HAL_BUTTON_ENTER) || (button & HAL_BUTTON_ENCODER)) {
//            // change selected setting
//            if (selectedRow == 1) {
//                // change waveform
//                char *itemList[5];
//                uint8_t i;
//                for (i = 0; i < 5; i++) {
//                    itemList[i] = waveform_Names[i];
//                }
//                int8_t sel = menu_ItemChooseDialog("Select waveform:", itemList,
//                        5, waveform.form);
//                if (sel >= 0) {
//                    waveform.form = sel;
//                    load.powerOn = 0;
//                    if (waveform.form != WAVE_NONE) {
//                        // set load in correct mode
//                        // This works correctly because load.mode and paramNum
//                        // are using the same coding for the 4 different
//                        // modes/paramters
//                        load.mode = waveform.paramNum;
//                    }
//                }
//            } else if (selectedRow == 2) {
//                // change amplitude
//                uint32_t val;
//                if (menu_getInputValue(&val, "Amplitude:", minValue, maxValue,
//                        unit0, unit3, unit6)) {
//                    waveform.amplitude = val;
//                    load.powerOn = 0;
//                }
//            } else if (selectedRow == 3) {
//                // change offset
//                uint32_t val;
//                if (menu_getInputValue(&val, "Offset:", minValue, maxValue,
//                        unit0, unit3, unit6)) {
//                    waveform.offset = val;
//                    load.powerOn = 0;
//                }
//            } else if (selectedRow == 4) {
//                // change period
//                uint32_t val;
//                if (menu_getInputValue(&val, "Period:", 0, 30000, "ms", "s",
//                NULL)) {
//                    waveform.period = val;
//                    load.powerOn = 0;
//                }
//            } else if (selectedRow == 5) {
//                // change set parameter
//                char *itemList[4];
//                uint8_t i;
//                for (i = 0; i < 4; i++) {
//                    itemList[i] = waveSetParamNames[i];
//                }
//                int8_t sel = menu_ItemChooseDialog("Select parameter:",
//                        itemList, EV_NUM_SETPARAMS, waveform.paramNum);
//                if (sel >= 0) {
//                    waveform.paramNum = sel;
//                    waveform.param = waveSetParamPointers[sel];
//                    if (waveform.form != WAVE_NONE) {
//                        // set load in correct mode
//                        // This works correctly because load.mode and paramNum
//                        // are using the same coding for the 4 different
//                        // modes/paramters
//                        load.mode = waveform.paramNum;
//                    }
//                }
//            }
//        }
//
//    } while (button != HAL_BUTTON_ESC);
//}
