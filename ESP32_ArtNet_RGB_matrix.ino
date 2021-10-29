/*
 * This sketch will control WS2812 or WS2811 LED strips either via ArtNet or in stand alone mode. It is also capable
 * of creating a WIFI access point over which other devices running the same code can be synchronized.
 */

#undef MBEDTLS_CONFIG_FILE
#include "ArtnetWifi.h"
#include "config.h"
#include "ws2812.h"
#include "ledOutput.h"
//#include "webserver.h"
#include "DataManager.h"
#include "PixelDriver.h"
#include "DisplayManager.h"


#include "Arduino.h"
#include "EEPROM.h"
#include <stdint.h>
#include <WiFi.h>

#define LED_UPDATE_INTERVAL 50
#define DMX_UPDATE_INTERVAL 100


void initArtnet();

ledOutput_t ledOutputs[NUMBER_LED_OUTPUTS];

#ifndef ON_LIGHT_TUBE_PCB
#endif


uint8_t artnet_initialized = 0;

// Artnet settings
ArtnetWifi artnet;

PixelDriver *pixelDriver = NULL;
DisplayManager *displayManager = NULL;


void setup()
{
	Serial.begin(115200);
//	ws2812_init(ws2812_pin);
	DataManager::init(ledOutputs);
	pixelDriver = PixelDriver::instance();
	displayManager = DisplayManager::instance();
	displayManager->init(DISPLAY_SDA, DISPLAY_SCL);

	Serial.println("hello, just started up");

#ifdef ON_LIGHT_TUBE_PCB
	initSwitches();
#endif

	int csPins[NUMBER_LED_OUTPUTS] = PIXEL_DRIVER_CS;
	//initialize LED data
	for(uint8_t i = 0; i < NUMBER_LED_OUTPUTS; i++){
		for(uint16_t j = 0; j < MAX_LEDS_PER_OUTPUT*3; j++){
			ledOutputs[i].data[j] = 0;
		}
		ledOutputs[i].update = 0;
		ledOutputs[i].CSPin = csPins[i];
	}
}

void loop()
{
	DataManager::update();
	displayManager->update();

	if(DataManager::getScheduleRestart()){
		DataManager::setScheduleRestart(false);
		delay(1000);
		ESP.restart();
		delay(1000);
	}

	//only recieve and display data if not in setup mode
	if(DataManager::getInetMode() == accesspoint)
		return;

	//init artnet
	if(DataManager::getWifiConnected() && !artnet_initialized){
		Serial.print("my ip: ");
		Serial.println(WiFi.localIP());
		initArtnet();
	}

	if(artnet_initialized){
		uint16_t opCode;
		opCode = artnet.read();
		if(opCode == ART_POLL){
			for(int i = 0; i < NUMBER_LED_OUTPUTS; i++){
				if(ledOutputs[i].config.numberLEDs > 0){
					uint16_t endUniverse = ledOutputs[i].config.startUniverse + (ledOutputs[i].config.startDmxAddress +
							ledOutputs[i].config.numberLEDs*3)/512;
					for(int j = ledOutputs[i].config.startUniverse; j <= endUniverse; j++){
						artnet.sendArtPollReply(j, ledOutputs[i].config.shortName, ledOutputs[i].config.longName);
					}
				}
			}
		}
	}

	//update effect and LEDs
	for(uint8_t i = 0; i < NUMBER_LED_OUTPUTS; i++){
		if(ledOutputs[i].update){
			ledOutputs[i].update = 0;
			pixelDriver->send(ledOutputs[i].CSPin, ledOutputs[i].data, ledOutputs[i].config.numberLEDs*3);
//			ws2812_setColors(ledOutputs[0].config.numberLEDs, ledOutputs[0].data);
		}
	}
}

//void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data, IPAddress remoteIP)
void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{
	for(int i = 0; i < NUMBER_LED_OUTPUTS; i++){
		uint16_t endUniverse = ledOutputs[i].config.startUniverse + (ledOutputs[i].config.startDmxAddress +
				ledOutputs[i].config.numberLEDs*3)/512;
		if(ledOutputs[i].config.startUniverse <= universe && endUniverse >= universe){
			if(universe == ledOutputs[i].config.startUniverse){
				for(uint16_t j = ledOutputs[i].config.startDmxAddress-1, k = 0;
						j < length && k < ledOutputs[i].config.numberLEDs*3; j++, k++){
					ledOutputs[i].data[k] = data[j];
				}
			}
			else{
				uint16_t startIndex = ((universe - ledOutputs[i].config.startUniverse)*512 -
						(ledOutputs[i].config.startDmxAddress - 1)) * 3;
				for(uint16_t j = 0, k = startIndex; j < length && k < ledOutputs[i].config.numberLEDs*3; j++, k++){
					ledOutputs[i].data[k] = data[j];
				}
			}
		}
		if (endUniverse == universe){
			ledOutputs[i].update = 0xFF;
		}
	}
}

void onSync(IPAddress remoteIP) {
}

void initArtnet(){
	artnet.begin();
	// this will be called for each packet received
	artnet.setArtDmxCallback(onDmxFrame);
	artnet_initialized = true;
}
