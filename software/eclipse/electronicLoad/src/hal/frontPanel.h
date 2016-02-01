#ifndef FRONTPANEL_H_
#define FRONTPANEL_H_

#include <stdint.h>

typedef enum {
	BUTTON_0,
	BUTTON_1,
	BUTTON_2,
	BUTTON_3,
	BUTTON_4,
	BUTTON_5,
	BUTTON_6,
	BUTTON_7,
	BUTTON_8,
	BUTTON_9,
	BUTTON_ESC,
	BUTTON_DOT,
	BUTTON_CC,
	BUTTON_CV,
	BUTTON_CR,
	BUTTON_CP,
	BUTTON_MENU,
	BUTTON_ENTER,
	BUTTON_ENCODER,
	BUTTON_NONE
} Button_t;

void hal_frontPanelInit(void);

Button_t hal_getButton(void);

int32_t hal_getEncoderMovement(void);

#endif
