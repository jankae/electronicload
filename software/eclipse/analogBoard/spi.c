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
        spi.ports = byte;
    }
}
