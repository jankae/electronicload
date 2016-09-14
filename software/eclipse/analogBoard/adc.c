#include "adc.h"

void adc_Init(void) {
    // select AVCC as reference
    ADMUX = (1 << REFS0);
    // ADC enable, auto-trigger, interrupt enable, prescaler=64 -> 125kHz
    ADCSRA |= (1 << ADEN) | (1 << ADATE) | (1 << ADIE) | (1 << ADPS2)
            | (1 << ADPS1);
    // free running mode
    ADCSRB = 0;
    adc.currentChannel = 7;
    // start conversions
    ADCSRA |= (1 << ADSC);
}

ISR(ADC_vect) {
    // store measured value
    adc.channels[adc.currentChannel] = ADC;
    ADMUX &= ~0x0F;
    adc.currentChannel = (adc.currentChannel + 1) % 9;
    if (adc.currentChannel != 7) {
        // select next channel
        ADMUX |= (adc.currentChannel + 1) % 9;
    } else {
        // select 1.1V bandgap as eighth channel
        ADMUX |= 0x0E;
    }
}
