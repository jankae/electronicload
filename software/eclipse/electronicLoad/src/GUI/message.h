#ifndef GUI_MESSAGE_H_
#define GUI_MESSAGE_H_

#include "screen.h"

typedef enum {
    MESSAGE_RESULT_NONE, MESSAGE_RES_OK, MESSAGE_RES_ABORT, MESSAGE_RES_NO, MESSAGE_RES_YES
} GUIMessageResult_t;
typedef enum {
    MESSAGE_OK, MESSAGE_OK_ABORT, MESSAGE_YES_NO
} GUIMessageType_t;

GUIMessageResult_t message_Box(const char *text, uint8_t lines, uint8_t columns,
        font_t font, GUIMessageType_t msg, widget_t *callee);

#endif
