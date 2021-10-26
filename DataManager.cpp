/*
 * DataManager.cpp
 *
 *  Created on: Aug 21, 2020
 *      Author: tsugua
 */

#include "DataManager.h"
#include "Webserver.h"
#include "config.h"
#include "ledOutput.h"

#include <WiFi.h>
#include <WiFiClient.h>

#include "Arduino.h"
#include "EEPROM.h"
#include <stdint.h>


unsigned long DataManager::lastWifiConnectTryTime;
bool DataManager::scheduleRestart;
internetMode DataManager::inetMode;
ledOutput_t* DataManager::ledOutputs;

//	Flash address parameters
#define SSID_MAX_LEN				30
#define SSID_ADDR					0
#define WIFI_PW_MAX_LEN				30
#define WIFI_PW_ADDR				31
#define	HOST_NAME_MAX_LEN			30
#define	HOST_NAME_ADDR				62
#define	INET_MODE_ADDR				93
#define INET_MODE_LEN				sizeof(internetMode)
#define CHECKSUM_ADDR				97
#define	CHECKSUM_LEN				4

#define LED_CONFIG_ADDR				150
#define LED_CONFIG_LEN				4*sizeof(ledConfig_t)

#define	EEPROM_SIZE					150+4*sizeof(ledConfig_t)
char ssid[SSID_MAX_LEN+1];
char password[WIFI_PW_MAX_LEN+1];
char hostName[HOST_NAME_MAX_LEN+1];

void DataManager::init(ledOutput_t *leds){
	lastWifiConnectTryTime = 0;
	scheduleRestart = false;
	ledOutputs = leds;

	//read from flash/blynk (update each other) or use default values
	EEPROM.begin(EEPROM_SIZE);

	//read SSID and password from flash
	for(uint8_t i = 0; i < SSID_MAX_LEN+1; i++){
		ssid[i] = EEPROM.read(SSID_ADDR+i);
	}
	for(uint8_t i = 0; i < WIFI_PW_MAX_LEN+1; i++){
		password[i] = EEPROM.read(WIFI_PW_ADDR+i);
	}
	for(uint8_t i = 0; i < HOST_NAME_MAX_LEN+1; i++){
		hostName[i] = EEPROM.read(HOST_NAME_ADDR+i);
	}
	uint32_t checksum = 0;
	for(uint8_t i = 0; i < CHECKSUM_LEN; i++){
		checksum |= ((uint32_t)EEPROM.read(CHECKSUM_ADDR+i))<<(8*i);
	}

	eepromRead((uint8_t*)&inetMode, INET_MODE_ADDR, INET_MODE_LEN);
	for(int i = 0; i < NUMBER_LED_OUTPUTS; i++){
		eepromRead((uint8_t*)&((ledOutputs+i)->config), LED_CONFIG_ADDR+i*sizeof(ledConfig_t), sizeof(ledConfig_t));
	}

	bool notInitialized = false;
	//check if WIFI values are initialized correctly
	if(checksum != calculateWIFIChecksum()){
		notInitialized = true;
	}
	//check if all other variables are initialized correctly
	if(isnan((int)inetMode)){
		notInitialized = true;
	}
	for(int i = 0; i < NUMBER_LED_OUTPUTS; i++){
		if(isnan((ledOutputs+i)->config.startUniverse) || (ledOutputs+i)->config.startUniverse < 0 ||
				(ledOutputs+i)->config.startDmxAddress < 1 || isnan((ledOutputs+i)->config.startDmxAddress) ||
				isnan((ledOutputs+i)->config.numberLEDs) || (ledOutputs+i)->config.numberLEDs < 0 ||
				(ledOutputs+i)->config.numberLEDs > MAX_LEDS_PER_OUTPUT)
			notInitialized = true;
	}
	//if EEPROM not initialized yet, write default values
	if(notInitialized){
		Serial.println("Writing default values");
		// set default values for WIFI
		//copy default SSID to EEPROM and to the ssid variable
		const char* default_ssid = DEFAULT_SSID;
		for(uint8_t i = 0; i < SSID_MAX_LEN+1; i++){
			EEPROM.write(SSID_ADDR + i, default_ssid[i]);
			ssid[i] = default_ssid[i];
			if(default_ssid[i] == '\0'){
				break;
			}
		}
		//copy default password to EEPROM and to the password variable
		const char* default_pw = DEFAULT_WIFI_PW;
		for(uint8_t i = 0; i < WIFI_PW_MAX_LEN+1; i++){
			EEPROM.write(WIFI_PW_ADDR + i, default_pw[i]);
			password[i] = default_pw[i];
			if(default_pw[i] == '\0'){
				break;
			}
		}
		//copy default host name to EEPROM and to the hostName variable
		const char* default_host_name = DEFAULT_HOST_NAME;
		for(uint8_t i = 0; i < HOST_NAME_MAX_LEN+1; i++){
			EEPROM.write(HOST_NAME_ADDR + i, default_host_name[i]);
			hostName[i] = default_host_name[i];
			if(default_host_name[i] == '\0'){
				break;
			}
		}
		checksum = calculateWIFIChecksum();
		for(uint8_t i = 0; i < CHECKSUM_LEN; i++){
			EEPROM.write(CHECKSUM_ADDR+i, (uint8_t)(checksum>>(8*i)));
		}
		EEPROM.commit();

		//set default values for all other variables
		inetMode = DEFAULT_INTERT_MODE;
		eepromWrite((uint8_t*)&inetMode, INET_MODE_ADDR, INET_MODE_LEN, false);
		for(int i = 0; i < NUMBER_LED_OUTPUTS; i++){
			(ledOutputs+i)->config.startUniverse = DEFAULT_ARTNET_UNIVERSE;
			(ledOutputs+i)->config.startDmxAddress = DEFAULT_DMX_ADDR;
			(ledOutputs+i)->config.numberLEDs = DEFAULT_NUMBER_LEDS;
			strcpy((ledOutputs+i)->config.shortName, DEFAULT_SHORT_NAME);
			strcpy((ledOutputs+i)->config.longName, DEFAULT_LONG_NAME);
			//write to eeprom
			eepromWrite((uint8_t*)&((ledOutputs+i)->config), LED_CONFIG_ADDR+i*sizeof(ledConfig_t), sizeof(ledConfig_t), false);
		}
		EEPROM.commit();
	}

	//enter wifi setup mode
	if(DataManager::getInetMode() == accesspoint){
		WIFISetupMode();
	}
	else if(DataManager::getInetMode() == wifiDHCP){
		WiFi.setHostname(hostName);
		WiFi.begin(ssid, password);
		lastWifiConnectTryTime = millis();
		delay(500);
	}
	webserver_init();
}

/// standard update routine
void DataManager::update(){
	if(DataManager::getInetMode() == wifiDHCP && !WiFi.isConnected() &&
			millis() >= lastWifiConnectTryTime + WIFI_CONNECT_INTERVAL){
		WiFi.begin(ssid, password);
		lastWifiConnectTryTime = millis();
	}
}

internetMode DataManager::getInetMode(){
	return inetMode;
}

void DataManager::setScheduleRestart(bool restart){
	scheduleRestart = true;
}

bool DataManager::getScheduleRestart(){
	return scheduleRestart;
}

void DataManager::setInetMode(internetMode mode){
	if(inetMode == mode){
		return;
	}
	inetMode = mode;
	eepromWrite((uint8_t*)&inetMode, INET_MODE_ADDR, INET_MODE_LEN, true);
	scheduleRestart = true;
}

bool DataManager::getWifiConnected(){
	return WiFi.isConnected();
}


//////////////////////////////////////////////////////////////////
//																//
//				EEPROM functions								//
//																//
//////////////////////////////////////////////////////////////////


/// used to make sure valid data is read from EEPROM
uint32_t DataManager::calculateWIFIChecksum(){
	uint32_t res = 0;
	for(uint8_t i = 0, j = 0; i < SSID_MAX_LEN+1; i++, j = (j+1)%CHECKSUM_LEN){
		res ^= ((uint32_t)(ssid[i])<<(8*j));
		if(ssid[i] == '\0'){
			break;
		}
	}
	for(uint8_t i = 0, j = 0; i < WIFI_PW_MAX_LEN+1; i++, j = (j+1)%CHECKSUM_LEN){
		res ^= ((uint32_t)(password[i])<<(8*j));
		if(password[i] == '\0'){
			break;
		}
	}
	for(uint8_t i = 0, j = 0; i < HOST_NAME_MAX_LEN+1; i++, j = (j+1)%CHECKSUM_LEN){
		res ^= ((uint32_t)(hostName[i])<<(8*j));
		if(hostName[i] == '\0'){
			break;
		}
	}
	return res;
}

/// reads data from the EEPROM
// @param dst: memory address, where the data read is to be stored
// @param addr: EEPROM start address
// @param len: length of the data to be read in bytes
void DataManager::eepromRead(uint8_t *dst, int addr, int len){
	if(len <= 0){
		return;
	}
	for(int i = 0; i < len; i++){
		*(dst++) = EEPROM.read(addr++);
	}
}

/// writes data to the EEPROM
// @param src: memory address, where the data to be written is to be stored
// @param addr: EEPROM start address
// @param len: length of the data to be written in bytes
// @param commit: will finish the EEPROM write with a commit, if true.
//					Only performing such a commit will make the data being actually written
void DataManager::eepromWrite(uint8_t *src, int addr, int len, bool commit){
	if(len < 0){
		return;
	}
	for(int i = 0; i < len; i++){
		EEPROM.write(addr++, *(src++));
	}
	if(commit){
		EEPROM.commit();
	}
}

/**
 * Enters wifi setup mode: machine will create an access point with default SSID and password.
 * On the machine IP address (192.168.4.1) will be a webserver running to set up the new WIFI credentials
 * No regular operation will be possible in this mode
 */
void DataManager::WIFISetupMode(){
	WiFi.softAP(DEFAULT_SSID, DEFAULT_WIFI_PW);
	IPAddress IP = WiFi.softAPIP();
	Serial.print("AP IP address: ");
	Serial.println(IP);
	webserver_init();
	while(1){
		delay(500);
		if(scheduleRestart){
			scheduleRestart = false;
			delay(1000);
			ESP.restart();
		}
	}
}

String DataManager::setWIFICredentials(const char* newSSID, const char* newPassword, const char* newHostName){
	if(newSSID != NULL && strcmp(newSSID, "") != 0){
		for(uint8_t i = 0; i < SSID_MAX_LEN+1; i++){
			EEPROM.write(SSID_ADDR + i, newSSID[i]);
			ssid[i] = newSSID[i];
			if(newSSID[i] == '\0'){
				break;
			}
		}
	}
	if(newPassword != NULL && strcmp(newPassword, "") != 0){
		for(uint8_t i = 0; i < WIFI_PW_MAX_LEN+1; i++){
			EEPROM.write(WIFI_PW_ADDR + i, newPassword[i]);
			password[i] = newPassword[i];
			if(newPassword[i] == '\0'){
				break;
			}
		}
	}
	if(newHostName != NULL && strcmp(newHostName, "") != 0){
		for(uint8_t i = 0; i < HOST_NAME_MAX_LEN+1; i++){
			EEPROM.write(HOST_NAME_ADDR + i, newHostName[i]);
			hostName[i] = newHostName[i];
			if(newHostName[i] == '\0'){
				break;
			}
		}
	}
	uint32_t checksum = calculateWIFIChecksum();
	for(uint8_t i = 0; i < CHECKSUM_LEN; i++){
		EEPROM.write(CHECKSUM_ADDR+i, (uint8_t)(checksum>>(8*i)));
	}
	EEPROM.commit();
	scheduleRestart = true;
	return "Successfully set new WIFI credentials. You can change them again by restarting your machine while having both "
			"buttons pressed and the distribution switch set to manual distribution.";
}

String DataManager::setArtnetUniverse(int output, int universe){
	if(output < 0 || output >= NUMBER_LED_OUTPUTS)
		return "Not a valid LED output. Please set a value from 0 to" + String(NUMBER_LED_OUTPUTS-1);
	if(universe < 0)
		return "Not a valid universe. Please set a value larger than 0.";
	(ledOutputs + output)->config.startUniverse = universe;
	eepromWrite((uint8_t*)&((ledOutputs+output)->config), LED_CONFIG_ADDR+output*sizeof(ledConfig_t), sizeof(ledConfig_t),
			true);
	return "Successfully updated Artnet universe";
}

String DataManager::setDMXAddress(int output, int address){
	if(output < 0 || output >= NUMBER_LED_OUTPUTS)
		return "Not a valid LED output. Please set a value from 0 to" + String(NUMBER_LED_OUTPUTS-1);
	if(address < 1 || address > 512)
		return "Not a valid DMX address. Please enter a value between 1 and 512.";
	(ledOutputs + output)->config.startDmxAddress = address;
	eepromWrite((uint8_t*)&((ledOutputs+output)->config), LED_CONFIG_ADDR+output*sizeof(ledConfig_t), sizeof(ledConfig_t),
			true);
	return "Successfully updated DMX address";
}

String DataManager::setNumberLeds(int output, int number){
	if(output < 0 || output >= NUMBER_LED_OUTPUTS)
		return "Not a valid LED output. Please set a value from 0 to" + String(NUMBER_LED_OUTPUTS-1);
	if(number < 0 || number > MAX_LEDS_PER_OUTPUT)
		return "Not a valid number of LEDs. Please set a value from 0 to " + String(MAX_LEDS_PER_OUTPUT);
	(ledOutputs + output)->config.numberLEDs = number;
	eepromWrite((uint8_t*)&((ledOutputs+output)->config), LED_CONFIG_ADDR+output*sizeof(ledConfig_t), sizeof(ledConfig_t),
			true);
	return "Successfully updated number LEDs";
}

String DataManager::setShortName(int output, String name){
	if(output < 0 || output >= NUMBER_LED_OUTPUTS)
		return "Not a valid LED output. Please set a value from 0 to" + String(NUMBER_LED_OUTPUTS-1);
	if(name.length() > 17)
		return "Short name is too long. Maximum allowed length is 17 characters";
	strcpy((ledOutputs+output)->config.shortName, name.c_str());
	eepromWrite((uint8_t*)&((ledOutputs+output)->config), LED_CONFIG_ADDR+output*sizeof(ledConfig_t), sizeof(ledConfig_t),
			true);
	return "Successfully updated short Name";
}

String DataManager::setLongName(int output, String name){
	if(output < 0 || output >= NUMBER_LED_OUTPUTS)
		return "Not a valid LED output. Please set a value from 0 to" + String(NUMBER_LED_OUTPUTS-1);
	if(name.length() > 63)
		return "Long name is too long. Maximum allowed length is 63 characters";
	strcpy((ledOutputs+output)->config.longName, name.c_str());
	eepromWrite((uint8_t*)&((ledOutputs+output)->config), LED_CONFIG_ADDR+output*sizeof(ledConfig_t), sizeof(ledConfig_t),
			true);
	return "Successfully updated long Name";
}

ledConfig_t* DataManager::getLedConfig(int output){
	if(output < 0)
		output = 0;
	if(output >= NUMBER_LED_OUTPUTS)
		output = NUMBER_LED_OUTPUTS - 1;
	return &((ledOutputs+output)->config);
}


