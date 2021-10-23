/*
 * PixelDriver.h
 *
 *  Created on: 19.10.2021
 *      Author: ag4716
 */

#ifndef PIXELDRIVER_H_
#define PIXELDRIVER_H_

#include "Arduino.h"
#include <SPI.h>

#define PIXELDRIVER_SPI_CLK_SPEED		1800000		//2MHz

class PixelDriver {
public:
	static PixelDriver *instance(){
		if(!_instance)
			_instance = new PixelDriver();
		return _instance;
	}
	void send(int CSPin, uint8_t *data, int len);
private:
	static PixelDriver *_instance;
	PixelDriver();
	PixelDriver (const PixelDriver& );
	virtual ~PixelDriver();

	SPIClass *hspi;
};

#endif /* PIXELDRIVER_H_ */
