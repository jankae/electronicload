#ifndef LOADFUNCTIONS_H_
#define LOADFUNCTIONS_H_

#include <stdint.h>
#include "currentSink.h"
#include "calibration.h"

#define LOAD_MAXVOLTAGE			50000
#define LOAD_MAXCURRENT			20000
#define LOAD_MAXRESISTANCE		1000000
#define LOAD_MAXPOWER			100000

typedef enum {
	FUNCTION_CC, FUNCTION_CV, FUNCTION_CR, FUNCTION_CP
} loadType_t;

struct {
	loadType_t type;
	// values for the different modes
	// constant current in mA
	uint32_t current;
	// constant voltage in mV
	uint32_t voltage;
	// constant resistance in mOhm
	uint32_t resistance;
	// constant power in mW
	uint32_t power;
} loadFunctions;

void load_Init(void);

void load_set_CC(uint32_t c);

void load_set_CV(uint32_t v);

void load_set_CR(uint32_t r);

void load_set_CP(uint32_t p);

void load_update(void);

#endif
