#include "test.h"

void test_Menu(void) {
    int8_t sel;
    do {
        const char *entries[TEST_NUM_TESTS];
        const char availableBaudrates[TEST_NUM_TESTS][21] = { "Font", "Buttons",
                "AVR GPIO", "AVR ADC", "DAC", "ADC"
#ifdef TEST_INCLUDE_EASTER
                , "Snake"
#endif
                };
        for (sel = 0; sel < TEST_NUM_TESTS; sel++) {
            entries[sel] = availableBaudrates[sel];
        }
        sel = menu_ItemChooseDialog(
                "\xCD\xCD\xCD\xCD\xCD\xCDTEST MENU\xCD\xCD\xCD\xCD\xCD\xCD",
                entries, TEST_NUM_TESTS);
        switch (sel) {
        case 0:
            test_Font();
            break;
        case 1:
            test_Buttons();
            break;
        case 2:
            test_AVRGPIO();
            break;
        case 3:
            test_AVRADC();
            break;
        case 4:
            test_DAC();
            break;
        case 5:
            test_ADC();
            break;
#ifdef TEST_INCLUDE_EASTER
        case TEST_NUM_TESTS - 1:
            test_Snake();
            break;
#endif
        }
    } while (sel >= 0);
}

void test_Font(void) {
    while (hal_getButton())
        ;
    screen_Clear();
    screen_FastString12x16("FONT TEST", 10, 0);
    screen_FastString6x8("Navigate: 2,4,6,8", 0, 3);
    screen_FastString6x8("Toggle font size: 5", 0, 4);
    screen_SetSoftButton("Start", 2);
    while (!(hal_getButton() & HAL_BUTTON_SOFT2))
        ;
    uint8_t active = 1;
    uint8_t bigFont = 0;
    uint8_t topRow = 0;
    uint8_t leftColumn = 0;
    uint8_t cursorX = 0, cursorY = 0;
    do {
        uint8_t maxX, maxY;
        uint8_t maxColumn, maxRow;
        screen_Clear();
        // print font
        if (bigFont) {
            // print 8x4 array of chars
            uint8_t i, j;
            for (i = 0; i < 8; i++) {
                for (j = 0; j < 4; j++) {
                    screen_FastChar12x16(i * 12, j * 2,
                            (j + topRow) * 16 + i + leftColumn);
                }
            }
            screen_InvertChar12x16(cursorX * 12, cursorY * 2);
            maxX = 7;
            maxY = 3;
            maxRow = 12;
            maxColumn = 8;
        } else {
            // print 16x8 array of chars
            uint8_t i, j;
            for (i = 0; i < 16; i++) {
                for (j = 0; j < 8; j++) {
                    screen_FastChar6x8(i * 6, j,
                            (j + topRow) * 16 + i + leftColumn);
                }
            }
            screen_InvertChar6x8(cursorX * 6, cursorY);
            maxX = 15;
            maxY = 7;
            maxRow = 8;
            maxColumn = 0;
        }
        // print hexcode
        screen_FastString6x8("Hex:", 102, 0);
        screen_FastString6x8("0x", 102, 1);
        uint8_t selectedChar = ((topRow + cursorY) << 4) + leftColumn + cursorX;
        screen_FastChar12x16(108, 3, selectedChar);
        screen_Rectangle(106, 22, 121, 41);
        screen_Rectangle(105, 21, 122, 42);
        char hex10 =
                (selectedChar >> 4) < 10 ?
                        (selectedChar >> 4) + '0' : (selectedChar >> 4) + '7';
        char hex1 =
                (selectedChar & 0x0f) < 10 ?
                        (selectedChar & 0x0f) + '0' :
                        (selectedChar & 0x0f) + '7';
        screen_FastChar6x8(114, 1, hex10);
        screen_FastChar6x8(120, 1, hex1);
        uint32_t button;
        // wait for button press
        while (!(button = hal_getButton()))
            ;
        while (hal_getButton())
            ;
        if (button & HAL_BUTTON_ESC) {
            active = 0;
        } else if (button & HAL_BUTTON_2) {
            // move cursor up
            if (cursorY > 0) {
                cursorY--;
            } else if (topRow > 0) {
                topRow--;
            } else {
                cursorY = maxY;
                topRow = maxRow;
            }
        } else if (button & HAL_BUTTON_4) {
            // move cursor left
            if (cursorX > 0) {
                cursorX--;
            } else if (leftColumn > 0) {
                leftColumn--;
            } else {
                cursorX = maxX;
                leftColumn = maxColumn;
            }
        } else if (button & HAL_BUTTON_6) {
            // move cursor right
            if (cursorX < maxX) {
                cursorX++;
            } else if (leftColumn < maxColumn) {
                leftColumn++;
            } else {
                cursorX = 0;
                leftColumn = 0;
            }
        } else if (button & HAL_BUTTON_8) {
            // move cursor down
            if (cursorY < maxY) {
                cursorY++;
            } else if (topRow < maxRow) {
                topRow++;
            } else {
                cursorY = 0;
                topRow = 0;
            }
        } else if (button & HAL_BUTTON_5) {
            // toggle font size
            if (bigFont) {
                cursorX += leftColumn;
                leftColumn = 0;
                cursorY += topRow;
                if (cursorY > 7) {
                    topRow = cursorY - 7;
                    cursorY = 7;
                } else {
                    topRow = 0;
                }
                bigFont = 0;
            } else {
                if (cursorX > 7) {
                    leftColumn = cursorX - 7;
                    cursorX = 7;
                }
                cursorY += topRow;
                if (cursorY > 3) {
                    topRow = cursorY - 3;
                    cursorY = 3;
                } else {
                    topRow = 0;
                }
                bigFont = 1;
            }
        }
    } while (active);
}

void test_Buttons(void) {
    while (hal_getButton())
        ;
    screen_Clear();
    screen_FastString12x16("BUTTONTEST", 4, 0);
    screen_FastString6x8("Press more than 4", 0, 3);
    screen_FastString6x8("buttons at once to", 0, 4);
    screen_FastString6x8("end test.", 0, 5);
    screen_SetSoftButton("Start", 0);
    screen_SetSoftButton("Start", 1);
    screen_SetSoftButton("Start", 2);
    while (!(hal_getButton()
            & (HAL_BUTTON_SOFT2 | HAL_BUTTON_SOFT1 | HAL_BUTTON_SOFT0)))
        ;
    uint8_t buttonsPressed;
    uint32_t button = hal_getButton();
    do {
        // display currenty pressed buttons
        screen_Clear();
        if (button & HAL_BUTTON_1)
            screen_FastChar6x8(6, 0, '1');
        if (button & HAL_BUTTON_2)
            screen_FastChar6x8(24, 0, '2');
        if (button & HAL_BUTTON_3)
            screen_FastChar6x8(42, 0, '3');
        if (button & HAL_BUTTON_ESC)
            screen_FastString6x8("Esc", 54, 0);
        if (button & HAL_BUTTON_CC)
            screen_FastString6x8("CC", 96, 0);
        if (button & HAL_BUTTON_CV)
            screen_FastString6x8("CV", 114, 0);
        if (button & HAL_BUTTON_4)
            screen_FastChar6x8(6, 1, '4');
        if (button & HAL_BUTTON_5)
            screen_FastChar6x8(24, 1, '5');
        if (button & HAL_BUTTON_6)
            screen_FastChar6x8(42, 1, '6');
        if (button & HAL_BUTTON_0)
            screen_FastChar6x8(60, 1, '0');
        if (button & HAL_BUTTON_CR)
            screen_FastString6x8("CR", 96, 1);
        if (button & HAL_BUTTON_CP)
            screen_FastString6x8("CP", 114, 1);
        if (button & HAL_BUTTON_7)
            screen_FastChar6x8(6, 2, '7');
        if (button & HAL_BUTTON_8)
            screen_FastChar6x8(24, 2, '8');
        if (button & HAL_BUTTON_9)
            screen_FastChar6x8(42, 2, '9');
        if (button & HAL_BUTTON_DOT)
            screen_FastString6x8("Dot", 54, 2);
        if (button & HAL_BUTTON_ENTER)
            screen_FastString6x8("ENT", 90, 2);
        if (button & HAL_BUTTON_ONOFF)
            screen_FastString6x8("ON", 114, 2);
        if (button & HAL_BUTTON_SOFT0)
            screen_SetSoftButton("Soft0", 0);
        if (button & HAL_BUTTON_SOFT1)
            screen_SetSoftButton("Soft1", 1);
        if (button & HAL_BUTTON_SOFT2)
            screen_SetSoftButton("Soft2", 2);
        while (hal_getButton() == button)
            ;
        button = hal_getButton();
        // calculate number of pressed buttons
        buttonsPressed = 0;
        uint32_t i;
        for (i = 0x01; i; i <<= 1) {
            if (i & button)
                buttonsPressed++;
        }
    } while (buttonsPressed < 5);
    screen_Clear();
}

void test_AVRGPIO(void) {
    // block load update stuff (includes all SPI communication)
    load.disableIOcontrol = 1;
    while (hal_getButton())
        ;
    uint8_t gpios = 0;
    uint8_t active = 1;

    char gpioNames[7][12] = { "1:SHUNT_EN2", "2:SHUNT_EN1", "3:SHUNTSEL",
            "4:FAN", "5:MODE_A", "6:MODE_B", "7:ANA_MUX" };
    do {
        screen_Clear();
        screen_FastString6x8("Manual GPIO control", 0, 0);
        // display current GPIO status
        uint8_t i;
        uint8_t j = 1;
        for (i = 0; i < 7; i++) {
            screen_FastString6x8(gpioNames[i], 0, i + 1);
            if (gpios & j) {
                // this GPIO is set
                screen_FastChar6x8(90, i + 1, '1');
            } else {
                screen_FastChar6x8(90, i + 1, '0');
            }
            j <<= 1;
        }
        // clear all GPIOs
        hal_ClearAVRGPIO(0xff);
        // set only active GPIOs
        hal_SetAVRGPIO(gpios);
        // update actual ports on AVR
        hal_UpdateAVRGPIOs();

        timer_waitms(10);

        uint32_t button = hal_getButton();
        if (button & HAL_BUTTON_ESC) {
            active = 0;
            continue;
        }
        if (button & HAL_BUTTON_1) {
            gpios ^= 0x01;
        }
        if (button & HAL_BUTTON_2) {
            gpios ^= 0x02;
        }
        if (button & HAL_BUTTON_3) {
            gpios ^= 0x04;
        }
        if (button & HAL_BUTTON_4) {
            gpios ^= 0x08;
        }
        if (button & HAL_BUTTON_5) {
            gpios ^= 0x10;
        }
        if (button & HAL_BUTTON_6) {
            gpios ^= 0x20;
        }
        if (button & HAL_BUTTON_7) {
            gpios ^= 0x40;
        }
        while (hal_getButton())
            ;
    } while (active);
    load.disableIOcontrol = 0;
}

void test_AVRADC(void) {
    // block load update stuff (includes all SPI communication)
    load.disableIOcontrol = 1;
    while (hal_getButton())
        ;
    uint8_t gpios = 0;
    uint8_t active = 1;

    do {
        screen_Clear();
        screen_FastString6x8("AVR ADC overview", 0, 0);
        // display all ADC channels
        uint8_t i;
        for (i = 0; i < 9; i++) {
            uint16_t result = hal_ReadAVRADC(i);
            char buf[5];
            string_fromUint(result, buf, 4, 0);
            uint8_t x = i < 5 ? 0 : 60;
            uint8_t y = i < 5 ? i + 1 : i - 4;
            screen_FastChar6x8(x, y, i + '0');
            screen_FastChar6x8(x + 6, y, ':');
            screen_FastString6x8(buf, x + 12, y);
        }

        timer_waitms(100);

        uint32_t button = hal_getButton();
        if (button & HAL_BUTTON_ESC) {
            active = 0;
            continue;
        }
        while (hal_getButton())
            ;
    } while (active);
    load.disableIOcontrol = 0;
}

void test_DAC(void) {
    // block load update stuff (includes all SPI communication)
    load.disableIOcontrol = 1;
    while (hal_getButton())
        ;
    uint8_t active = 1;
    uint32_t DACvalue = 0;
    do {
        screen_Clear();
        screen_FastString6x8("DAC Test", 0, 0);
        char buf[11];
        string_fromUint(DACvalue, buf, 5, 0);
        screen_FastString6x8("DAC value:", 0, 2);
        screen_FastString12x16(buf, 0, 3);
        screen_SetSoftButton("Value", 2);

        hal_setDAC(DACvalue);

        uint32_t button;
        while (!(button = hal_getButton()))
            ;
        if (button & HAL_BUTTON_ESC) {
            active = 0;
            continue;
        }
        if (button & HAL_BUTTON_SOFT2) {
            // get new DAC value
            menu_getInputValue(&DACvalue, "DAC value", 0, 65535, "LSB", NULL,
            NULL);
        }
        while (hal_getButton())
            ;
    } while (active);
    load.disableIOcontrol = 0;
}

void test_ADC(void) {
    // block load update stuff (includes all SPI communication)
    load.disableIOcontrol = 1;
    while (hal_getButton())
        ;

    uint8_t active = 1;

    do {
        screen_Clear();
        screen_FastString6x8("Raw ADC overview", 0, 0);
        // display ADC channels
        hal_SelectADCChannel(HAL_ADC_CURRENT);
        timer_waitms(1);
        uint16_t adc = hal_getADC(16);
        char buf[6];
        string_fromUint(adc, buf, 5, 0);
        screen_FastString6x8("Current:", 0, 2);
        screen_FastString6x8(buf, 54, 2);

        hal_SelectADCChannel(HAL_ADC_VOLTAGE);
        timer_waitms(1);
        adc = hal_getADC(16);
        string_fromUint(adc, buf, 5, 0);
        screen_FastString6x8("Voltage:", 0, 3);
        screen_FastString6x8(buf, 54, 3);

        timer_waitms(100);

        uint32_t button = hal_getButton();
        if (button & HAL_BUTTON_ESC) {
            active = 0;
            continue;
        }
        while (hal_getButton())
            ;
    } while (active);
    load.disableIOcontrol = 0;
}

#ifdef TEST_INCLUDE_EASTER
void test_Snake(void) {
    while (hal_getButton())
        ;
    static uint32_t highscore = 0;
    screen_Clear();
    screen_FastString12x16("SNAKE", 34, 0);
    screen_FastString6x8("Move: 2,4,6,8", 0, 3);
    screen_FastString6x8("Highscore:", 0, 5);
    char buf[4];
    string_fromUint(highscore, buf, 3, 0);
    screen_FastString6x8(buf, 60, 5);
    screen_SetSoftButton("Start", 2);
    while (!(hal_getButton() & HAL_BUTTON_SOFT2))
        ;
    int8_t snakeCoords[200][2];
    // initialize snake
    snakeCoords[0][0] = 0;
    snakeCoords[1][0] = 1;
    snakeCoords[2][0] = 2;
    snakeCoords[3][0] = 3;
    snakeCoords[0][1] = 0;
    snakeCoords[1][1] = 0;
    snakeCoords[2][1] = 0;
    snakeCoords[3][1] = 0;
    uint8_t head = 3;
    uint8_t tail = 0;
    uint8_t direction = SNAKE_RIGHT;
    uint8_t alive = 1;
    // crude 'random' number generator
    uint8_t mealX = timer.ms % 20;
    uint8_t mealY = (timer.ms / 20) % 10;
    uint8_t i;
    do {
        screen_Clear();
        // draw outline
        screen_Rectangle(2, 0, 125, 63);
        screen_Rectangle(3, 1, 124, 62);
        // draw snake
        for (i = tail; i != head; i = (i + 1) % 200) {
            screen_VerticalLine(4 + 6 * snakeCoords[i][0],
                    3 + 6 * snakeCoords[i][1], 4);
            screen_VerticalLine(5 + 6 * snakeCoords[i][0],
                    2 + 6 * snakeCoords[i][1], 6);
            screen_VerticalLine(6 + 6 * snakeCoords[i][0],
                    2 + 6 * snakeCoords[i][1], 6);
            screen_VerticalLine(7 + 6 * snakeCoords[i][0],
                    2 + 6 * snakeCoords[i][1], 6);
            screen_VerticalLine(8 + 6 * snakeCoords[i][0],
                    2 + 6 * snakeCoords[i][1], 6);
            screen_VerticalLine(9 + 6 * snakeCoords[i][0],
                    3 + 6 * snakeCoords[i][1], 4);
        }
        screen_VerticalLine(4 + 6 * snakeCoords[head][0],
                3 + 6 * snakeCoords[head][1], 4);
        screen_VerticalLine(5 + 6 * snakeCoords[head][0],
                2 + 6 * snakeCoords[head][1], 6);
        screen_VerticalLine(6 + 6 * snakeCoords[head][0],
                2 + 6 * snakeCoords[head][1], 6);
        screen_VerticalLine(7 + 6 * snakeCoords[head][0],
                2 + 6 * snakeCoords[head][1], 6);
        screen_VerticalLine(8 + 6 * snakeCoords[head][0],
                2 + 6 * snakeCoords[head][1], 6);
        screen_VerticalLine(9 + 6 * snakeCoords[head][0],
                3 + 6 * snakeCoords[head][1], 4);
        // draw meal
        screen_Rectangle(5 + 6 * mealX, 3 + 6 * mealY, 8 + 6 * mealX,
                6 + 6 * mealY);
        // handle user input
        timer_waitms(100);
        uint32_t timeout = timer_SetTimeout(400);
        uint32_t button;
        do {
            button = hal_getButton();
        } while (!timer_TimeoutElapsed(timeout) && !button);
        if (button & HAL_BUTTON_ESC) {
            return;
        } else if ((button & HAL_BUTTON_LEFT) && direction != SNAKE_RIGHT) {
            direction = SNAKE_LEFT;
        } else if ((button & HAL_BUTTON_RIGHT) && direction != SNAKE_LEFT) {
            direction = SNAKE_RIGHT;
        } else if ((button & HAL_BUTTON_DOWN) && direction != SNAKE_UP) {
            direction = SNAKE_DOWN;
        } else if ((button & HAL_BUTTON_UP) && direction != SNAKE_DOWN) {
            direction = SNAKE_UP;
        }
        uint8_t headX = snakeCoords[head][0];
        uint8_t headY = snakeCoords[head][1];
        head = (head + 1) % 200;
        // move snake
        switch (direction) {
        case SNAKE_LEFT:
            snakeCoords[head][0] = headX - 1;
            snakeCoords[head][1] = headY;
            break;
        case SNAKE_RIGHT:
            snakeCoords[head][0] = headX + 1;
            snakeCoords[head][1] = headY;
            break;
        case SNAKE_UP:
            snakeCoords[head][0] = headX;
            snakeCoords[head][1] = headY - 1;
            break;
        case SNAKE_DOWN:
            snakeCoords[head][0] = headX;
            snakeCoords[head][1] = headY + 1;
            break;
        }
        if (snakeCoords[head][0] == mealX && snakeCoords[head][1] == mealY) {
            // generate new meal
            mealX = timer.ms % 20;
            mealY = (timer.ms / 20) % 10;
            uint8_t mealPositionOK;
            do {
                mealPositionOK = 1;
                // check for collision with snake
                for (i = tail; i != head; i = (i + 1) % 200) {
                    if (mealX == snakeCoords[i][0]
                            && mealY == snakeCoords[i][1]) {
                        // new meal is within snake
                        mealPositionOK = 0;
                        break;
                    }
                }
                if (mealX == snakeCoords[head][0]
                        && mealY == snakeCoords[head][1])
                    // new meal is new head position
                    mealPositionOK = 0;
                if (!mealPositionOK) {
                    // shift meal position by one
                    mealX++;
                    if (mealX >= 20) {
                        mealY++;
                        mealX = 0;
                        if (mealY >= 10)
                            mealY = 0;
                    }
                }
            } while (!mealPositionOK);
        } else {
            tail = (tail + 1) % 200;
        }
        // check for collision with outline
        if (snakeCoords[head][0] < 0 || snakeCoords[head][0] >= 20
                || snakeCoords[head][1] < 0 || snakeCoords[head][1] >= 10) {
            alive = 0;
            continue;
        }
        // check for collision with itself
        for (i = tail; i != head; i = (i + 1) % 200) {
            if (snakeCoords[head][0] == snakeCoords[i][0]
                    && snakeCoords[head][1] == snakeCoords[i][1]) {
                alive = 0;
                continue;
            }
        }
    } while (alive);
    // 'blink' snake
    uint8_t j;
    for (j = 0; j < 16; j++) {
        screen_Clear();
        screen_Clear();
        // draw outline
        screen_Rectangle(2, 0, 125, 63);
        screen_Rectangle(3, 1, 124, 62);
        if (j & 1) {
            // draw snake
            for (i = tail; i != head; i = (i + 1) % 200) {
                screen_VerticalLine(4 + 6 * snakeCoords[i][0],
                        3 + 6 * snakeCoords[i][1], 4);
                screen_VerticalLine(5 + 6 * snakeCoords[i][0],
                        2 + 6 * snakeCoords[i][1], 6);
                screen_VerticalLine(6 + 6 * snakeCoords[i][0],
                        2 + 6 * snakeCoords[i][1], 6);
                screen_VerticalLine(7 + 6 * snakeCoords[i][0],
                        2 + 6 * snakeCoords[i][1], 6);
                screen_VerticalLine(8 + 6 * snakeCoords[i][0],
                        2 + 6 * snakeCoords[i][1], 6);
                screen_VerticalLine(9 + 6 * snakeCoords[i][0],
                        3 + 6 * snakeCoords[i][1], 4);
            }
        }
        uint32_t timeout = timer_SetTimeout(200);
        do {
            if (hal_getButton() & HAL_BUTTON_ESC) {
                while (hal_getButton())
                    ;
                j = 16;
                break;
            }
        } while (!timer_TimeoutElapsed(timeout));
    }
    // calculate points
    uint8_t points;
    if (head > tail) {
        points = head - tail - 3; // start length is 3
    } else {
        points = (uint16_t) head + 200 - tail - 3;
    }
    screen_Clear();
    screen_FastString12x16("GAME OVER", 10, 0);
    screen_FastString12x16("Points:", 4, 2);
    buf[4];
    string_fromUint(points, buf, 3, 0);
    screen_FastString12x16(buf, 88, 2);
    if (points > highscore) {
        highscore = points;
        screen_FastString12x16("HIGHSCORE", 10, 5);
    }
    while (!(hal_getButton() & HAL_BUTTON_ESC))
        ;
    while (hal_getButton())
        ;
}
#endif
