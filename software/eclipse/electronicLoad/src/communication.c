#include "communication.h"

void com_Init(void) {
    timer_SetupPeriodicFunction(4, MS_TO_TICKS(10), com_Update, 10);
}

void com_Update(void) {
    // retrieve uart command if available
    uint8_t length = uart_dataAvailable();
    if(length){
        uint8_t cmd[length];
        uart_retrieveData(cmd);
        // evaluate command
        if(!strncmp(cmd, "LDOFF", 5)){
            load.powerOn = 0;
            uart_writeString("OK\n");
        } else if(!strncmp(cmd, "LDON", 4)){
            load.powerOn = 1;
            uart_writeString("OK\n");
        } else {
            uart_writeString("ERROR\n");
        }
    }
}
