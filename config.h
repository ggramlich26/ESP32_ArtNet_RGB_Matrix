/*
 * config.h
 *
 *  Created on: Feb 9, 2020
 *      Author: tsugua
 */

#ifndef CONFIG_H_
#define CONFIG_H_


#define ws2812_pin 	4    // Number of the data out pin
#define	PIXEL_DRIVER_MOSI	13
#define PIXEL_DRIVER_MISO	12
#define PIXEL_DRIVER_SCK	14
#define	PIXEL_DRIVER_CS_1	27
#define	PIXEL_DRIVER_CS_2	26
#define	PIXEL_DRIVER_CS_3	25
#define	PIXEL_DRIVER_CS_4	33
#define	PIXEL_DRIVER_CS_5	32
#define PIXEL_DRIVER_CS		{PIXEL_DRIVER_CS_1, PIXEL_DRIVER_CS_2, PIXEL_DRIVER_CS_3, PIXEL_DRIVER_CS_4, PIXEL_DRIVER_CS_5}
#define NUMBER_LED_OUTPUTS	5
//#define WS2812B_PINS	[4,5,6,7]

//WIFI settings
#define DEFAULT_SSID		"Zeus Lighting"
#define DEFAULT_WIFI_PW		"BeautyOfLight"
#define DEFAULT_HOST_NAME	"Light Tube"

#define MAX_LEDS_PER_OUTPUT	240

#define MAX_ARTNET_SHORT_NAME_LEN	18
#define MAX_ARTNET_LONG_NAME_LEN	64

//#define	ON_LIGHT_TUBE_PCB

#endif /* CONFIG_H_ */
