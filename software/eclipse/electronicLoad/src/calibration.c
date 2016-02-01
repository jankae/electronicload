#include "calibration.h"

/*
 * This section must fit into one flash page and thus
 * NEVER exceed 1kB
 */
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

/*
 * Transfers the calibration values from the end of the FLASH
 * (see linker script, section '.config') into the RAM
 */
void cal_readFromFlash(void) {
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
}

/*
 * Writes the calibration values from the RAM into the end
 * of the FLASH. Use sparsely to preserve FLASH
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
	FLASH_Lock();
}

/*
 * Starts and executes the calibration process. Stand-alone function,
 * start from main thread, depends on interrupts to update display and
 * to get user inputs.
 * IMPORTANT: disable all functions dealing with the DAC, this is all
 * done internally in this function
 */
void calibrationProcess(void) {
	calibration.active = 1;

	uint16_t adc100mA;
	uint16_t adc5A;
	uint16_t dac100mA;
	uint16_t dac5A;

	uint16_t adc1V;
	uint16_t adc12V_lowRange;
	uint16_t adc12V_highRange;
	uint16_t adc30V;

	Button_t button = BUTTON_NONE;

	/*
	 * Step 1: calibrating DAC offset (done in hardware)
	 */
	hal_setGain(0);
	hal_setCurrentGain(1);
	hal_setDAC(0);

	display_Clear();
	display_FastString12x16("Calibration", 0, 0);
	display_FastString6x8("Apply about 5V. Turn", 0, 2);
	display_FastString6x8("internal trimmer", 0, 2);
	display_FastString6x8("until load draws 0mA", 0, 3);
	display_FastString6x8("ESC: Abort", 0, 6);
	display_FastString6x8("Enter: Continue", 0, 7);

	while (button != BUTTON_ENTER && button != BUTTON_ESC) {
		button = hal_getButton();
	}
	if (button == BUTTON_ESC)
		return;
	while (hal_getButton() != BUTTON_NONE)
		;

	/*
	 * Step 2: calibrating low current range
	 */
	int32_t defaultValue100mA = 20;
	int32_t defaultValue5A = 1000;

	display_Clear();
	display_FastString12x16("Calibration", 0, 0);
	display_FastString6x8("Apply about 5V. Turn", 0, 2);
	display_FastString6x8("knob until load draws", 0, 3);
	display_FastString6x8("100mA", 0, 4);
	display_FastString6x8("ESC: Abort", 0, 6);
	display_FastString6x8("Enter: Continue", 0, 7);

	do {
		hal_setDAC(defaultValue100mA);
		defaultValue100mA += hal_getEncoderMovement();
		if (defaultValue100mA < 0)
			defaultValue100mA = 0;
		else if (defaultValue100mA > 100)
			defaultValue100mA = 100;
		button = hal_getButton();
	} while (button != BUTTON_ENTER && button != BUTTON_ESC);
	hal_setDAC(0);
	if (button == BUTTON_ESC)
		return;

	// save calibration values
	dac100mA = defaultValue100mA;
	adc100mA = hal_getADC(ADC_CURRENT_SENSE, 100);

	while (hal_getButton() != BUTTON_NONE)
		;

	display_Clear();
	display_FastString12x16("Calibration", 0, 0);
	display_FastString6x8("Apply about 5V. Turn", 0, 2);
	display_FastString6x8("knob until load draws", 0, 3);
	display_FastString6x8("5A", 0, 4);
	display_FastString6x8("ESC: Abort", 0, 6);
	display_FastString6x8("Enter: Continue", 0, 7);

	do {
		hal_setDAC(defaultValue5A);
		defaultValue5A += hal_getEncoderMovement();
		if (defaultValue5A < 900)
			defaultValue5A = 900;
		else if (defaultValue5A > 1100)
			defaultValue5A = 1100;
		button = hal_getButton();
	} while (button != BUTTON_ENTER && button != BUTTON_ESC);
	hal_setDAC(0);
	if (button == BUTTON_ESC)
		return;

	// save calibration values
	dac5A = defaultValue5A;
	adc5A = hal_getADC(ADC_CURRENT_SENSE, 100);

	while (hal_getButton() != BUTTON_NONE)
		;

	/*
	 * Step 3: calibrating low voltage range
	 */
	hal_setVoltageGain(1);
	display_Clear();
	display_FastString12x16("Calibration", 0, 0);
	display_FastString6x8("Apply exactly 1V.", 0, 2);
	display_FastString6x8("ESC: Abort", 0, 6);
	display_FastString6x8("Enter: Continue", 0, 7);

	do {
		button = hal_getButton();
	} while (button != BUTTON_ENTER && button != BUTTON_ESC);
	if (button == BUTTON_ESC)
		return;

	// save calibration values
	adc1V = hal_getADC(ADC_VOLTAGE_SENSE, 100);

	while (hal_getButton() != BUTTON_NONE)
		;

	display_Clear();
	display_FastString12x16("Calibration", 0, 0);
	display_FastString6x8("Apply exactly 12V.", 0, 2);
	display_FastString6x8("ESC: Abort", 0, 6);
	display_FastString6x8("Enter: Continue", 0, 7);

	do {
		button = hal_getButton();
	} while (button != BUTTON_ENTER && button != BUTTON_ESC);
	if (button == BUTTON_ESC)
		return;

	// save calibration values
	adc12V_lowRange = hal_getADC(ADC_VOLTAGE_SENSE, 100);
	hal_setVoltageGain(0);
	// TODO wait until value has settled
	adc12V_highRange = hal_getADC(ADC_VOLTAGE_SENSE, 100);

	while (hal_getButton() != BUTTON_NONE)
		;

	/*
	 * Step 4: calibrating high voltage range
	 */
	hal_setVoltageGain(0);
	display_Clear();
	display_FastString12x16("Calibration", 0, 0);
	display_FastString6x8("Apply exactly 30V.", 0, 2);
	display_FastString6x8("ESC: Abort", 0, 6);
	display_FastString6x8("Enter: Continue", 0, 7);

	do {
		button = hal_getButton();
	} while (button != BUTTON_ENTER && button != BUTTON_ESC);
	if (button == BUTTON_ESC)
		return;

	// save calibration values
	adc30V = hal_getADC(ADC_VOLTAGE_SENSE, 100);

	while (hal_getButton() != BUTTON_NONE)
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

	display_Clear();
	display_FastString12x16("Calibration", 0, 0);
	display_FastString12x16("completed", 0, 2);
	display_FastString6x8("Enter: Continue", 0, 7);
	while (hal_getButton() != BUTTON_ENTER)
		;
	display_Clear();
	while (hal_getButton() != BUTTON_NONE)
		;

	calibration.active = 0;
}

/*
 * sets the 'should be'-current
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

/*
 * returns the actual sinked current in mA
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

/*
 * returns the measured voltage at the terminals in mV
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

/*
 * returns the temperature of heatsink1 in 0.1°C
 */
uint16_t cal_getTemp1(void) {
	return LM35_ADC_TO_TEMP(hal_getADC(ADC_TEMPERATURE1, 1));
}

/*
 * returns the temperature of heatsink2 in 0.1°C
 */
uint16_t cal_getTemp2(void) {
	return LM35_ADC_TO_TEMP(hal_getADC(ADC_TEMPERATURE2, 1));
}
