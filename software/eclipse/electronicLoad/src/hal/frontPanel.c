/**
  * \file	frontPanel.c
  * \author	Jan Kaeberich
  * \brief	Frontpanel hardware abstraction layer source file.
  * 		This file contains all function connected to the#
  * 		frontpanel user inputs
  */

#include "frontPanel.h"


/**
 * \brief Initialises the frontpanel hardware
 *
 * Initialises GPIOs used for the buttons and the encoder.
 * Also registers the frontPanelUpdate function with Timer4
 * in a 10ms interval
 * @see hal_frontPanelUpdate
 */
void hal_frontPanelInit(void){

}

/**
 * \brief Reads and saves the status from the user inputs
 *
 * Should be called regularly (e.g. at a 10ms interval) in a
 * low priority interrupt. Performs a user-button multiplex
 * cycle and handles the encoder counter
 */
void hal_frontPanelUpdate(void){

}

/**
 * \brief Returns the pattern of pressed buttons
 *
 * \return bitmask representing pressed buttons
 */
uint32_t hal_getButton(void){
	return frontpanel.buttonState;
}

/**
 * \brief Returns the movement of the encoder
 *
 * \return encoder steps since last call
 */
int32_t hal_getEncoderMovement(void){
	uint8_t buf = frontpanel.encoderCounter;
	frontpanel.encoderCounter = 0;
	return buf;
}
