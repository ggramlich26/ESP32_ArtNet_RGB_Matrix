/*
 * DataManager.h
 *
 *  Created on: Aug 20, 2020
 *      Author: tsugua
 */

#ifndef DATAMANAGER_H_
#define DATAMANAGER_H_

#include "Arduino.h"

enum internetMode{wifiDHCP, ethernetDHCP, wifiStatic, ethernetStatic, accesspoint};

#define WIFI_CONNECT_INTERVAL					10000	//interval in which the device tries to connect to wifi
#define DEFAULT_INTERT_MODE						wifiDHCP


class DataManager {
public:

	static internetMode getInetMode();
	static void setInetMode(internetMode mode);
	static void setScheduleRestart(bool restart);

	static bool getWifiConnected();
	static bool getScheduleRestart();

	static String setWIFICredentials(const char* newSSID, const char* newPassword, const char* newHostName);

	static void init();
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

	static unsigned long lastWifiConnectTryTime;
};
#endif /* DATAMANAGER_H_ */

