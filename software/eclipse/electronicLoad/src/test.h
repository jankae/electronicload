#ifndef TEST_H_
#define TEST_H_

#include "hal/frontPanel.h"
#include "hal/display.h"
#include "menu.h"

#define TEST_INCLUDE_EASTER

#ifdef TEST_INCLUDE_EASTER
#define TEST_NUM_TESTS          5
#else
#define TEST_NUM_TESTS          4
#endif

void test_Menu(void);

void test_Font(void);

void test_Buttons(void);

void test_AVRGPIO(void);

void test_AVRADC(void);

#ifdef TEST_INCLUDE_EASTER
#define SNAKE_LEFT              0
#define SNAKE_UP                1
#define SNAKE_RIGHT             2
#define SNAKE_DOWN              3
void test_Snake(void);
#endif

#endif
