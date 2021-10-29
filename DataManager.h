/*
 * DataManager.h
 *
 *  Created on: Aug 20, 2020
 *      Author: tsugua
 */

#ifndef DATAMANAGER_H_
#define DATAMANAGER_H_

#include "Arduino.h"
#include "ledOutput.h"

//enum internetMode{wifiDHCP, ethernetDHCP, wifiStatic, ethernetStatic, accesspoint};
enum internetMode{wifiDHCP, accesspoint};

#define WIFI_CONNECT_INTERVAL					10000	//interval in which the device tries to connect to wifi
#define DEFAULT_INTERT_MODE						accesspoint
#define DEFAULT_ARTNET_UNIVERSE					0
#define DEFAULT_DMX_ADDR						1
#define DEFAULT_NUMBER_LEDS						120
#define DEFAULT_SHORT_NAME						"Pixel controller"
#define DEFAULT_LONG_NAME						"Pixel controller by Georg"


class DataManager {
public:

	static internetMode getInetMode();
	static void setInetMode(internetMode mode);
	static void setScheduleRestart(bool restart);
	static String setArtnetUniverse(int output, int universe);
	static String setDMXAddress(int output, int address);
	static String setNumberLeds(int output, int number);
	static String setShortName(int output, String name);
	static String setLongName(int output, String name);

	static bool getWifiConnected();
	static String getIPAddress();
	static bool getScheduleRestart();
	static ledConfig_t* getLedConfig(int output);

	static String setWIFICredentials(const char* newSSID, const char* newPassword, const char* newHostName);

	static void init(ledOutput_t *leds);
	static void update();
private:
	DataManager(){}
	virtual ~DataManager(){}

	static internetMode inetMode;

	static uint32_t calculateWIFIChecksum();
	static void eepromWrite(uint8_t *src, int addr, int len, bool commit);
	static void eepromRead(uint8_t *dst, int addr, int len);
	static void WIFISetupMode();

	static bool scheduleRestart;
	static ledOutput_t *ledOutputs;

	static unsigned long lastWifiConnectTryTime;
};
#endif /* DATAMANAGER_H_ */

