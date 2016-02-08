/**
 * \file
 * \brief   Calibration source file.
 *
 * This file contains calibration values and functions to calibrate
 * the load and store the calibration values
 */
#include "calibration.h"

/*
 * This section must fit into one flash page and thus
 * NEVER exceed 1kB
 */
__attribute__ ((section(".config")))
uint32_t calFlashValid;
__attribute__ ((section(".config")))
struct {
	int16_t currentSenseOffsetLowRange;
	float currentSenseScaleLowRange;

	int16_t currentSenseOffsetHighRange;
	float currentSenseScaleHighRange;

	int16_t voltageSenseOffsetLowRange;
	float voltageSenseScaleLowRange;

	int16_t voltageSenseOffsetHighRange;
	float voltageSenseScaleHighRange;

	int16_t currentSetOffsetLowRange;
	float currentSetScaleLowRange;

	int16_t currentSetOffsetHighRange;
	float currentSetScaleHighRange;
}calibrationFlash;

/**
 * \brief Transfers the calibration values from the end of the FLASH
 *
 * (see linker script, section '.config') into the RAM
 * \return 0 on success, 1 on failure
 */
uint8_t cal_readFromFlash(void) {
	// check whether there is any calibration data in FLASH
	if (calFlashValid == 0x01) {
		// copy memory section from FLASH into RAM
		// (depends on both sections being identically)
		uint8_t i;
		uint32_t *from = (uint32_t*) &calibrationFlash;
		uint32_t *to = (uint32_t*) &calibration;
		uint8_t bytes = sizeof(calibrationFlash);
		for (i = 0; i < bytes; i++) {
			*to = *from;
			to++;
			from++;
		}
		return 0;
	}
	return 1;
}

/**
 * \brief Writes the calibration values from the RAM into the end
 * of the FLASH.
 *
 * Use sparsely to preserve FLASH
 */
void cal_writeToFlash(void) {
	FLASH_Unlock();
	FLASH_ClearFlag(
	FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	FLASH_ErasePage(0x0807F800);
	// FLASH is ready to be written at this point
	uint8_t i;
	uint32_t *from = (uint32_t*) &calibration;
	uint32_t *to = (uint32_t*) &calibrationFlash;
	uint8_t words = (sizeof(calibrationFlash) + 3) / 4;
	for (i = 0; i < words; i++) {
		FLASH_ProgramWord((uint32_t) to, *from);
		to += 4;
		from += 4;
	}
	// set valid data indicator
	FLASH_ProgramWord((uint32_t) &calFlashValid, 0x01);
	FLASH_Lock();
}

/**
 * \brief Sets the calibration values to the default values.
 *
 * Should be used in case of missing calibration data.
 */
void cal_setDefaultCalibration(void) {
	calibration.currentSenseOffsetLowRange = CAL_DEF_CURSENS_OFFSET_LOW;
	calibration.currentSenseScaleLowRange = CAL_DEF_CURSENS_SCALE_LOW;

	calibration.currentSenseOffsetHighRange = CAL_DEF_CURSENS_OFFSET_HIGH;
	calibration.currentSenseScaleHighRange = CAL_DEF_CURSENS_SCALE_HIGH;

	calibration.voltageSenseOffsetLowRange = CAL_DEF_VOLSENS_OFFSET_LOW;
	calibration.voltageSenseScaleLowRange = CAL_DEF_VOLSENS_SCALE_LOW;

	calibration.voltageSenseOffsetHighRange = CAL_DEF_VOLSENS_OFFSET_HIGH;
	calibration.voltageSenseScaleHighRange = CAL_DEF_VOLSENS_SCALE_HIGH;

	calibration.currentSetOffsetLowRange = CAL_DEF_CURSET_OFFSET_LOW;
	calibration.currentSetScaleLowRange = CAL_DEF_CURSET_SCALE_LOW;

	calibration.currentSetOffsetHighRange = CAL_DEF_CURSET_OFFSET_HIGH;
	calibration.currentSetScaleHighRange = CAL_DEF_CURSET_SCALE_HIGH;
}

/**
 * \brief Starts and executes the calibration process.
 *
 * Stand-alone function, start from main thread, depends on interrupts
 * to update display and  to get user inputs.
 * IMPORTANT: disable all functions dealing with the DAC, this is all
 * done internally in this function
 */
void calibrationProcess(void) {
    while(hal_getButton());
	calibration.active = 1;

	uint16_t adc100mA;
	uint16_t adc5A;
	uint16_t dac100mA;
	uint16_t dac5A;

	uint16_t adc1V;
	uint16_t adc12V_lowRange;
	uint16_t adc12V_highRange;
	uint16_t adc30V;

	uint32_t button = 0;

	/*
	 * Step 1: calibrating DAC offset (done in hardware)
	 */
	hal_setGain(0);
	hal_setCurrentGain(1);
	hal_setDAC(0);

	screen_Clear();
	screen_FastString12x16("Cal", 0, 0);
    screen_FastChar12x16(33, 0, 'i');
    screen_FastString12x16("brat", 43, 0);
    screen_FastChar12x16(89, 0, 'i');
    screen_FastString12x16("on", 99, 0);
	screen_FastString6x8("Apply about 5V. Turn", 0, 2);
	screen_FastString6x8("internal trimmer", 0, 3);
	screen_FastString6x8("until load draws 0mA", 0, 4);
	screen_FastString6x8("ESC: Abort", 0, 6);
	screen_FastString6x8("Enter: Continue", 0, 7);

	while (!(button & (HAL_BUTTON_ESC | HAL_BUTTON_ENTER))) {
		button = hal_getButton();
	}
	if (button & HAL_BUTTON_ESC) {
		calibration.active = 0;
		return;
	}
    while (hal_getButton())
        ;

	/*
	 * Step 2: calibrating low current range
	 */
	int32_t defaultValue100mA = 20;
	int32_t defaultValue5A = 1000;

	screen_Clear();
    screen_FastString12x16("Cal", 0, 0);
    screen_FastChar12x16(33, 0, 'i');
    screen_FastString12x16("brat", 43, 0);
    screen_FastChar12x16(89, 0, 'i');
    screen_FastString12x16("on", 99, 0);
	screen_FastString6x8("Apply about 5V. Turn", 0, 2);
	screen_FastString6x8("knob until load draws", 0, 3);
	screen_FastString6x8("100mA", 0, 4);
	screen_FastString6x8("ESC: Abort", 0, 6);
	screen_FastString6x8("Enter: Continue", 0, 7);

	do {
		hal_setDAC(defaultValue100mA);
		defaultValue100mA += hal_getEncoderMovement();
		if (defaultValue100mA < 0)
			defaultValue100mA = 0;
		else if (defaultValue100mA > 100)
			defaultValue100mA = 100;
		button = hal_getButton();
        timer_waitms(1);
	} while (!(button & (HAL_BUTTON_ESC | HAL_BUTTON_ENTER)));
	hal_setDAC(0);
	if (button & HAL_BUTTON_ESC) {
		calibration.active = 0;
		return;
	}

	// save calibration values
	dac100mA = defaultValue100mA;
	adc100mA = hal_getADC(ADC_CURRENT_SENSE, 100);

	while (hal_getButton())
		;

	screen_Clear();
    screen_FastString12x16("Cal", 0, 0);
    screen_FastChar12x16(33, 0, 'i');
    screen_FastString12x16("brat", 43, 0);
    screen_FastChar12x16(89, 0, 'i');
    screen_FastString12x16("on", 99, 0);
	screen_FastString6x8("Apply about 5V. Turn", 0, 2);
	screen_FastString6x8("knob until load draws", 0, 3);
	screen_FastString6x8("5A", 0, 4);
	screen_FastString6x8("ESC: Abort", 0, 6);
	screen_FastString6x8("Enter: Continue", 0, 7);

	do {
		hal_setDAC(defaultValue5A);
		defaultValue5A += hal_getEncoderMovement();
		if (defaultValue5A < 900)
			defaultValue5A = 900;
		else if (defaultValue5A > 1100)
			defaultValue5A = 1100;
		button = hal_getButton();
		timer_waitms(1);
	} while (!(button & (HAL_BUTTON_ESC | HAL_BUTTON_ENTER)));
	hal_setDAC(0);
	if (button & HAL_BUTTON_ESC) {
		calibration.active = 0;
		return;
	}

	// save calibration values
	dac5A = defaultValue5A;
	adc5A = hal_getADC(ADC_CURRENT_SENSE, 100);

	while (hal_getButton())
		;

	/*
	 * Step 3: calibrating low voltage range
	 */
	hal_setVoltageGain(1);
	screen_Clear();
    screen_FastString12x16("Cal", 0, 0);
    screen_FastChar12x16(33, 0, 'i');
    screen_FastString12x16("brat", 43, 0);
    screen_FastChar12x16(89, 0, 'i');
    screen_FastString12x16("on", 99, 0);
	screen_FastString6x8("Apply exactly 1V.", 0, 2);
	screen_FastString6x8("ESC: Abort", 0, 6);
	screen_FastString6x8("Enter: Continue", 0, 7);

	do {
		button = hal_getButton();
	} while (!(button & (HAL_BUTTON_ESC | HAL_BUTTON_ENTER)));
	if (button & HAL_BUTTON_ESC) {
		calibration.active = 0;
		return;
	}

	// save calibration values
	adc1V = hal_getADC(ADC_VOLTAGE_SENSE, 100);

	while (hal_getButton())
		;

	screen_Clear();
    screen_FastString12x16("Cal", 0, 0);
    screen_FastChar12x16(33, 0, 'i');
    screen_FastString12x16("brat", 43, 0);
    screen_FastChar12x16(89, 0, 'i');
    screen_FastString12x16("on", 99, 0);
	screen_FastString6x8("Apply exactly 12V.", 0, 2);
	screen_FastString6x8("ESC: Abort", 0, 6);
	screen_FastString6x8("Enter: Continue", 0, 7);

	do {
		button = hal_getButton();
	} while (!(button & (HAL_BUTTON_ESC | HAL_BUTTON_ENTER)));
	if (button & HAL_BUTTON_ESC) {
		calibration.active = 0;
		return;
	}

	// save calibration values
	adc12V_lowRange = hal_getADC(ADC_VOLTAGE_SENSE, 100);
	hal_setVoltageGain(0);
	timer_waitms(10);
	adc12V_highRange = hal_getADC(ADC_VOLTAGE_SENSE, 100);

	while (hal_getButton())
		;

	/*
	 * Step 4: calibrating high voltage range
	 */
	hal_setVoltageGain(0);
	screen_Clear();
    screen_FastString12x16("Cal", 0, 0);
    screen_FastChar12x16(33, 0, 'i');
    screen_FastString12x16("brat", 43, 0);
    screen_FastChar12x16(89, 0, 'i');
    screen_FastString12x16("on", 99, 0);
	screen_FastString6x8("Apply exactly 30V.", 0, 2);
	screen_FastString6x8("ESC: Abort", 0, 6);
	screen_FastString6x8("Enter: Continue", 0, 7);

	do {
		button = hal_getButton();
	} while (!(button & (HAL_BUTTON_ESC | HAL_BUTTON_ENTER)));
	if (button & HAL_BUTTON_ESC) {
		calibration.active = 0;
		return;
	}

	// save calibration values
	adc30V = hal_getADC(ADC_VOLTAGE_SENSE, 100);

	while (hal_getButton())
		;

	calibration.active = 0;

	/*
	 * Step 5: all done!
	 */

	// calculate calibration values
	calibration.currentSetScaleLowRange = (float) (dac5A - dac100mA)
			/ (5000 - 100);
	calibration.currentSetOffsetLowRange = dac100mA
			- 100 * calibration.currentSetScaleLowRange;

	calibration.currentSenseScaleLowRange = (float) (5000 - 100)
			/ (adc5A - adc100mA);
	calibration.currentSenseOffsetLowRange = adc100mA
			- 100 / calibration.currentSenseScaleLowRange;

	calibration.voltageSenseScaleLowRange = (float) (12000 - 1000)
			/ (adc12V_lowRange - adc1V);
	calibration.voltageSenseOffsetLowRange = adc1V
			- 1000 / calibration.voltageSenseScaleLowRange;

	calibration.voltageSenseScaleHighRange = (float) (30000 - 12000)
			/ (adc30V - adc12V_highRange);
	calibration.voltageSenseOffsetHighRange = adc12V_highRange
			- 12000 / calibration.voltageSenseScaleHighRange;

	screen_Clear();
    screen_FastString12x16("Cal", 0, 0);
    screen_FastChar12x16(33, 0, 'i');
    screen_FastString12x16("brat", 43, 0);
    screen_FastChar12x16(89, 0, 'i');
    screen_FastString12x16("on", 99, 0);
	screen_FastString12x16("completed", 0, 2);
	screen_FastString6x8("ESC: Discard", 0, 6);
	screen_FastString6x8("Enter: Save", 0, 7);
	do {
		button = hal_getButton();
	} while (!(button & (HAL_BUTTON_ESC | HAL_BUTTON_ENTER)));
	screen_Clear();
	if (button & HAL_BUTTON_ESC) {
		calibration.active = 0;
		return;
	}

	// save calibration values in FLASH
	cal_writeToFlash();

	while (hal_getButton())
		;

	calibration.active = 0;
}

/**
 * \brief Sets the 'should be'-current
 *
 * \param mA Current the load should draw
 */
void cal_setCurrent(uint32_t mA) {
	if ((mA <= 20000 && hal.setRange == RANGE_LOW) || mA <= 15000) {
		// use low range (0-20A)
		hal_setGain(0);
		int16_t dacValue = mA * calibration.currentSetScaleLowRange
				+ calibration.currentSetOffsetLowRange;
		if (dacValue < 0)
			dacValue = 0;
		else if (dacValue > 0x0fff)
			dacValue = 0x0fff;
		hal_setDAC(dacValue);
	} else {
		// use high range (0-200A)
		hal_setGain(1);
		// TODO add calibration
		hal_setDAC(mA / 50);
	}
}

/**
 * \brief Returns the current being drawn
 *
 * \return Current in mA
 */
int32_t cal_getCurrent(void) {
	uint16_t biased = hal_getADC(ADC_CURRENT_SENSE, 1);
	int32_t ret = 0;
	if (hal.currentRange == RANGE_LOW) {
		int16_t unbiased = biased - calibration.currentSenseOffsetLowRange;
		ret = unbiased * calibration.currentSenseScaleLowRange;
	} else {
		int16_t unbiased = biased - calibration.currentSenseOffsetHighRange;
		ret = unbiased * calibration.currentSenseScaleHighRange;
	}
	if (biased >= 3500 && hal.currentRange == RANGE_LOW) {
		// low range is near limit -> switch to high range
		hal_setCurrentGain(0);
	} else if (biased <= 300 && hal.currentRange == RANGE_HIGH) {
		// high range is near limit -> switch to low range
		hal_setCurrentGain(1);
	}
	return ret;
}

/**
 * \brief Returns the voltage at the terminals
 *
 * \return Voltage in mV
 */
int32_t cal_getVoltage(void) {
	uint16_t biased = hal_getADC(ADC_VOLTAGE_SENSE, 1);
	int32_t ret = 0;
	if (hal.voltageRange == RANGE_LOW) {
		int16_t unbiased = biased - calibration.voltageSenseOffsetLowRange;
		ret = unbiased * calibration.voltageSenseScaleLowRange;
	} else {
		int16_t unbiased = biased - calibration.voltageSenseOffsetHighRange;
		ret = unbiased * calibration.voltageSenseScaleHighRange;
	}
	if (biased >= 3500 && hal.voltageRange == RANGE_LOW) {
		// low range is near limit -> switch to high range
		hal_setVoltageGain(0);
	} else if (biased <= 300 && hal.voltageRange == RANGE_HIGH) {
		// high range is near limit -> switch to low range
		hal_setVoltageGain(1);
	}
	return ret;
}

/**
 * \brief Returns the temperature at heatsink1
 *
 * \return Temperature in 0.1°C
 */
uint16_t cal_getTemp1(void) {
	return LM35_ADC_TO_TEMP(hal_getADC(ADC_TEMPERATURE1, 1));
}

/**
 * \brief Returns the temperature at heatsink2
 *
 * \return Temperature in 0.1°C
 */
uint16_t cal_getTemp2(void) {
	return LM35_ADC_TO_TEMP(hal_getADC(ADC_TEMPERATURE2, 1));
}
