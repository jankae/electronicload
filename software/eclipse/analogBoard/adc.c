#include "adc.h"

void adc_Init(void) {
    // select AVCC as reference
    ADMUX = (1 << REFS0);
    // ADC enable, prescaler=64 -> 125kHz
    ADCSRA |= (1 << ADEN) | (1 << ADPS2)
            | (1 << ADPS1);
    adc.currentChannel = 0;
    // start conversions
    ADCSRA |= (1 << ADSC);
}
