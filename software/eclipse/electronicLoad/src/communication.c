#include "communication.h"

const char com_commands[COM_CMD_NUM][10] =
        { "HELP", "LDOFF", "LDON", "CC", "CV", "CP", "CR", "SETI", "SETU",
                "SETP", "SETR", "GETU", "GETI", "GETP" };

void com_Init(void) {
    timer_SetupPeriodicFunction(4, MS_TO_TICKS(10), com_Update, 10);
}

void com_Update(void) {
    // retrieve uart command if available
    uint8_t length = uart_dataAvailable();
    if (length) {
        uint8_t cmd[length];
        uart_retrieveData(cmd);
        uint8_t i;
        for (i = 0; i < length; i++) {
            cmd[i] = toupper(cmd[i]);
        }
        // evaluate command
        uint8_t cmdnum;
        for (cmdnum = 0; cmdnum < COM_CMD_NUM; cmdnum++) {
            if (!string_compare(cmd, com_commands[cmdnum]))
                break;
        }
        if (cmdnum < COM_CMD_NUM) {
            // received valid command
            uart_writeString("OK:");
            uart_writeString(com_commands[cmdnum]);
            uart_writeByte('\n');
            char answer[10];
            switch (cmdnum) {
            case COM_CMD_HELP:
                uart_writeString("Available commands:\n");
                for (i = 0; i < COM_CMD_NUM; i++) {
                    uart_writeString(com_commands[i]);
                    uart_writeByte('\n');
                }
                break;
            case COM_CMD_LOAD_OFF:
                load.powerOn = 0;
                break;
            case COM_CMD_LOAD_ON:
                load.powerOn = 1;
                break;
            case COM_CMD_CC:
                load.mode = FUNCTION_CC;
                break;
            case COM_CMD_CV:
                load.mode = FUNCTION_CV;
                break;
            case COM_CMD_CP:
                load.mode = FUNCTION_CP;
                break;
            case COM_CMD_CR:
                load.mode = FUNCTION_CR;
                break;
            case COM_CMD_SET_CURRENT:
                load.current = strtol(&cmd[4], NULL, 0);
                break;
            case COM_CMD_SET_VOLTAGE:
                load.voltage = strtol(&cmd[4], NULL, 0);
                break;
            case COM_CMD_SET_POWER:
                load.power = strtol(&cmd[4], NULL, 0);
                break;
            case COM_CMD_SET_RESISTANCE:
                load.resistance = strtol(&cmd[4], NULL, 0);
                break;
            case COM_CMD_GET_VOLTAGE:
                string_fromUint(load.state.voltage, answer, 6, 0);
                answer[6] = 'm';
                answer[7] = 'V';
                answer[8] = '\n';
                answer[9] = 0;
                uart_writeString(answer);
                break;
            case COM_CMD_GET_CURRENT:
                string_fromUint(load.state.current, answer, 5, 0);
                answer[5] = 'm';
                answer[6] = 'A';
                answer[7] = '\n';
                answer[8] = 0;
                uart_writeString(answer);
                break;
            case COM_CMD_GET_POWER:
                string_fromUint(load.state.power, answer, 6, 0);
                answer[6] = 'm';
                answer[7] = 'W';
                answer[8] = '\n';
                answer[9] = 0;
                uart_writeString(answer);
                break;
            }
        } else {
            // unknown command
            uart_writeString("ERROR\n");
        }
    }
}
