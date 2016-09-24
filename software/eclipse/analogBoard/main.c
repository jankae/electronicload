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

    sei();

    uint8_t ports = 0;
    uint8_t resultCounter = 0;
    uint16_t resultSum = 0;
    while (1) {
        if (!(ADCSRA & (1 << ADSC))) {
            // ADC has finished a conversion
            if (resultCounter >= 17) {
                resultSum += ADC;
                // save result and switch to next channel
                adc.channels[adc.currentChannel] = resultSum / 16;
                // increase current channel
                adc.currentChannel = (adc.currentChannel + 1) % 9;
                // set next conversion channel
                if (adc.currentChannel != 8) {
                    // select next channel
                    ADMUX = adc.currentChannel | (1 << REFS0);
                } else {
                    // select 1.1V bandgap as eighth channel
                    ADMUX = 0x0E | (1 << REFS0);
                }
                resultCounter = 0;
            } else {
                if (resultCounter == 0) {
                    resultSum = 0;
                } else {
                    resultSum += ADC;
                }
                resultCounter++;
            }
            // start next conversion
            ADCSRA |= (1 << ADSC);
        }
    }
}
