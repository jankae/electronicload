#ifndef TEST_H_
#define TEST_H_

#include "hal/frontPanel.h"
#include "hal/display.h"
#include "menu.h"

#define TEST_NUM_TESTS          3

#define TEST_INCLUDE_EASTER

void test_Menu(void);

void test_Font(void);

void test_Buttons(void);

#ifdef TEST_INCLUDE_EASTER
#define SNAKE_LEFT              0
#define SNAKE_UP                1
#define SNAKE_RIGHT             2
#define SNAKE_DOWN              3
void test_Snake(void);
#endif

#endif
