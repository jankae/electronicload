/**
 * \file
 * \brief   SPI control
 *
 * This file contains all SPI functions and implements a basic SPI protocol.
 *
 * The SPI is set up in the following way:
 * SPI slave, mode 0 which means:
 * - clock idle is low
 * - data is sampled at the rising clock edge
 * - data is set-up at the falling clock edge
 *
 * The protocol allows to transmit up to 7 GPIO data (only 6 are used)
 * and to read the ADC results from all eight channels (only 4 are used).
 *
 * Setting GPIOs:
 * Transmit one byte:
 *          +-------+-------+-------+-------+-------+-------+-------+-------+
 * send:    |   0   | GPIO6 | GPIO5 | GPIO4 | GPIO3 | GPIO2 | GPIO1 | GPIO0 |
 *          +-------+-------+-------+-------+-------+-------+-------+-------+
 * receive: |   X   |   X   |   X   |   X   |   X   |   X   |   X   |   X   |
 *          +-------+-------+-------+-------+-------+-------+-------+-------+
 *
 * GPIO functions:
 * GPIO0 - shunt enable 2 (PB1)
 * GPIO1 - shunt enable 1 (PB0)
 * GPIO2 - shunt selector (PD7)
 * GPIO3 - not connected (PD5)
 * GPIO4 - mode select A (PD4)
 * GPIO5 - mode select B (PD3)
 * GPIO6 - analog mux (PD2)
 *
 * Reading ADC channel:
 * Transmit three bytes:
 *          +-------+-------+-------+-------+-------+-------+-------+-------+
 * send1:   |   1   |   1   |   0   |   0   |   0   |   requested channel   |
 *          +-------+-------+-------+-------+-------+-------+-------+-------+
 * receive1:|   X   |   X   |   X   |   X   |   X   |   X   |   X   |   X   |
 *          +-------+-------+-------+-------+-------+-------+-------+-------+
 *          +-------+-------+-------+-------+-------+-------+-------+-------+
 * send2:   |   1   |   0   |   X   |   X   |   X   |   X   |   X   |   X   |
 *          +-------+-------+-------+-------+-------+-------+-------+-------+
 * receive2:|   0   |   0   |   0   |   0   |   0   |   0   |ADC_MSB| ADC_8 |
 *          +-------+-------+-------+-------+-------+-------+-------+-------+
 *          +-------+-------+-------+-------+-------+-------+-------+-------+
 * send3:   |   1   |   0   |   X   |   X   |   X   |   X   |   X   |   X   |
 *          +-------+-------+-------+-------+-------+-------+-------+-------+
 * receive3:| ADC_7 | ADC_6 | ADC_5 | ADC_4 | ADC_3 | ADC_2 | ADC_1 |ADC_LSB|
 *          +-------+-------+-------+-------+-------+-------+-------+-------+
 */

#ifndef SPI_H_
#define SPI_H_

#include <avr/io.h>
#include <avr/interrupt.h>

#include "adc.h"

struct {
    uint8_t ADCbuffer;
    volatile uint8_t ports;
} spi;

/**
 * \brief Sets up SPI slave mode
 */
void spi_Init(void);

/**
 * \brief Handles SPI communications
 */
ISR(SPI_STC_vect);

#endif /* SPI_H_ */
