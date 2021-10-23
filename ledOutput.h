/*
 * ledOutput.h
 *
 *  Created on: Sep 25, 2021
 *      Author: tsugua
 */

#ifndef LEDOUTPUT_H_
#define LEDOUTPUT_H_

#include "Arduino.h"
#include "config.h"

typedef struct ledConfig_s{
	uint16_t numberLEDs;
	int startUniverse;
	int startDmxAddress;
	char shortName[MAX_ARTNET_SHORT_NAME_LEN];
	char longName[MAX_ARTNET_LONG_NAME_LEN];
} ledConfig_t;

typedef struct ledOutput_s{
	ledConfig_t config;
	uint8_t data[MAX_LEDS_PER_OUTPUT*3];
} ledOutput_t;

//uint16_t ledOutput_getNumberLEDs(ledOutput_t *out);
//uint8_t ledOutput_getPin(ledOutput_t *out);

#endif /* LEDOUTPUT_H_ */
