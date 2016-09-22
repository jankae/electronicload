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


#endif /* ADC_H_ */
