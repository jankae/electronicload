#include "spi.h"

void spi_Init(void) {
    // set MISO as output
    DDRB |= (1 << PB4);
    // setup SPI slave, mode 0, MSB first
    SPCR |= (1 << SPIE) | (1 << SPE);
}

ISR(SPI_STC_vect) {
    uint8_t byte = SPDR;
    if (byte & 0x80) {
        // request ADC/read ADC
        if (byte & 0x40) {
            // latch requested ADC channel
            uint16_t result = adc.channels[byte & 0x0F];
            // transmit MSB of ADC result
            SPDR = result >> 8;
            spi.ADCbuffer = result & 0xff;
        } else {
            // transmit LSB of latched ADC result
            SPDR = spi.ADCbuffer;
        }
    } else {
        // received 'set ports' command
        if (byte & 0x01) {
            PORT_SHUNT_EN2 |= (1 << BIT_SHUNT_EN2);
        } else {
            PORT_SHUNT_EN2 &= ~(1 << BIT_SHUNT_EN2);
        }

        if (byte & 0x02) {
            PORT_SHUNT_EN1 |= (1 << BIT_SHUNT_EN1);
        } else {
            PORT_SHUNT_EN1 &= ~(1 << BIT_SHUNT_EN1);
        }

        if (byte & 0x04) {
            PORT_SHUNT_SEL |= (1 << BIT_SHUNT_SEL);
        } else {
            PORT_SHUNT_SEL &= ~(1 << BIT_SHUNT_SEL);
        }

        if (byte & 0x08) {
            PORT_GPIO3 |= (1 << BIT_GPIO3);
        } else {
            PORT_GPIO3 &= ~(1 << BIT_GPIO3);
        }

        if (byte & 0x10) {
            PORT_MODE_A |= (1 << BIT_MODE_A);
        } else {
            PORT_MODE_A &= ~(1 << BIT_MODE_A);
        }

        if (byte & 0x20) {
            PORT_MODE_B |= (1 << BIT_MODE_B);
        } else {
            PORT_MODE_B &= ~(1 << BIT_MODE_B);
        }

        if (byte & 0x40) {
            PORT_ANALOG_MUX |= (1 << BIT_ANALOG_MUX);
        } else {
            PORT_ANALOG_MUX &= ~(1 << BIT_ANALOG_MUX);
        }

    }
}
