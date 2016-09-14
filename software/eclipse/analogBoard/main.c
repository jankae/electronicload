#include <avr/io.h>

#include "adc.h"
#include "spi.h"
#include "portDefines.h"

int main(void) {
    // set gpio as outputs
    DDR_SHUNT_EN1 |= (1 << BIT_SHUNT_EN1);
    DDR_SHUNT_EN2 |= (1 << BIT_SHUNT_EN2);
    DDR_SHUNT_SEL |= (1 << BIT_SHUNT_SEL);
    DDR_GPIO3 |= (1 << BIT_GPIO3);
    DDR_MODE_A |= (1 << BIT_MODE_A);
    DDR_MODE_B |= (1 << BIT_MODE_B);
    DDR_ANALOG_MUX |= (1 << BIT_ANALOG_MUX);

    adc_Init();
    spi_Init();

    uint8_t ports = 0;
    while (1) {
        if (ports != spi.ports) {
            // SPI gpio have changed
            // -> update ports
            ports = spi.ports;

            if (ports & 0x01) {
                PORT_SHUNT_EN2 |= (1 << BIT_SHUNT_EN2);
            } else {
                PORT_SHUNT_EN2 &= ~(1 << BIT_SHUNT_EN2);
            }

            if (ports & 0x02) {
                PORT_SHUNT_EN1 |= (1 << BIT_SHUNT_EN1);
            } else {
                PORT_SHUNT_EN1 &= ~(1 << BIT_SHUNT_EN1);
            }

            if (ports & 0x04) {
                PORT_SHUNT_SEL |= (1 << BIT_SHUNT_SEL);
            } else {
                PORT_SHUNT_SEL &= ~(1 << BIT_SHUNT_SEL);
            }

            if (ports & 0x08) {
                PORT_GPIO3 |= (1 << BIT_GPIO3);
            } else {
                PORT_GPIO3 &= ~(1 << BIT_GPIO3);
            }

            if (ports & 0x10) {
                PORT_MODE_A |= (1 << BIT_MODE_A);
            } else {
                PORT_MODE_A &= ~(1 << BIT_MODE_A);
            }

            if (ports & 0x20) {
                PORT_MODE_B |= (1 << BIT_MODE_B);
            } else {
                PORT_MODE_B &= ~(1 << BIT_MODE_B);
            }

            if (ports & 0x40) {
                PORT_ANALOG_MUX |= (1 << BIT_ANALOG_MUX);
            } else {
                PORT_ANALOG_MUX &= ~(1 << BIT_ANALOG_MUX);
            }
        }
    }
}
