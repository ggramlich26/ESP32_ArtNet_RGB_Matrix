/*
 * config.h
 *
 *  Created on: Feb 9, 2020
 *      Author: tsugua
 */

#ifndef CONFIG_H_
#define CONFIG_H_


#define ws2812_pin 	4    // Number of the data out pin
#define WS2812B_PINS	[4,5,6,7]

//WIFI settings
#define DEFAULT_SSID		"Hulapalu"
#define DEFAULT_WIFI_PW		"thebestAK13top"
#define DEFAULT_HOST_NAME	"Light Tube"

#define MAX_LEDS_PER_OUTPUT	240
#define NUMBER_LED_OUTPUTS	1

#define MAX_ARTNET_SHORT_NAME_LEN	18
#define MAX_ARTNET_LONG_NAME_LEN	64

//#define	ON_LIGHT_TUBE_PCB

#endif /* CONFIG_H_ */
