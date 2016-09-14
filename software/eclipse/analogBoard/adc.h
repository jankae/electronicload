#ifndef ADC_H_
#define ADC_H_

#include <avr/io.h>
#include <avr/interrupt.h>

struct {
    uint16_t channels[9];
    uint8_t currentChannel;
} adc;

/**
 * \brief Initializes ADC, sets up free running mode
 */
void adc_Init(void);

/**
 * \brief Stores the measured channel into the ADC struct.
 *
 * Also incrementes the channel thus the ADC is cycling through
 * all eight channels.
 */
ISR(ADC_vect);

#endif /* ADC_H_ */
