/*
 * PixelDriver.cpp
 *
 *  Created on: 19.10.2021
 *      Author: ag4716
 */

#include "PixelDriver.h"
#include "config.h"

PixelDriver* PixelDriver::_instance = NULL;


PixelDriver::PixelDriver() {
	hspi = new SPIClass(HSPI);
	hspi->begin(PIXEL_DRIVER_SCK, PIXEL_DRIVER_MISO, PIXEL_DRIVER_MOSI, PIXEL_DRIVER_CS_1);
	int cs[] = PIXEL_DRIVER_CS;
	for(int i = 0; i < NUMBER_LED_OUTPUTS; i++){
		pinMode(cs[i], OUTPUT);
		digitalWrite(cs[i], HIGH);
	}
}

PixelDriver::~PixelDriver() {
	delete(hspi);
}

void PixelDriver::send(int CSPin, uint8_t *data, int len){
	hspi->beginTransaction(SPISettings(PIXELDRIVER_SPI_CLK_SPEED, MSBFIRST, SPI_MODE0));
	digitalWrite(CSPin, LOW);
	delayMicroseconds(20);
	hspi->writeBytes(data, len);
	digitalWrite(CSPin, HIGH);
	delayMicroseconds(1);
	hspi->endTransaction();
}

