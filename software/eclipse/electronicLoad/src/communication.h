#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_

#include <string.h>
#include <ctype.h>
#include "uart.h"
#include "loadFunctions.h"

#define COM_CMD_HELP                0
#define COM_CMD_LOAD_OFF            1
#define COM_CMD_LOAD_ON             2
#define COM_CMD_CC                  3
#define COM_CMD_CV                  4
#define COM_CMD_CP                  5
#define COM_CMD_CR                  6
#define COM_CMD_SET_CURRENT         7
#define COM_CMD_SET_VOLTAGE         8
#define COM_CMD_SET_POWER           9
#define COM_CMD_SET_RESISTANCE      10
#define COM_CMD_GET_VOLTAGE         11
#define COM_CMD_GET_CURRENT         12
#define COM_CMD_GET_POWER           13
// number of commands, must always be the last define
#define COM_CMD_NUM                 14

void com_Init(void);

void com_Update(void);

#endif
