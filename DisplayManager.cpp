/*
 * DisplayManager.cpp
 *
 *  Created on: 28.10.2021
 *      Author: ag4716
 */

#include "DisplayManager.h"
#include <cstddef>

DisplayManager* DisplayManager::_instance;

DisplayManager::DisplayManager() {
	display = NULL;
	nextInternetMode = DataManager::getInetMode();
	buttons = Buttons::instance();
}

DisplayManager::~DisplayManager() {
	if(display != NULL)
		delete(display);
}

void DisplayManager::init(int sda, int scl){
	display = new SSD1306Wire(0x3C, sda, scl);
	display->init();
	display->flipScreenVertically();
	display->clear();
	display->setFont(DISP_FONT);
	buttons->trackPin(BUTTON_DOWN, (btn_callback)([](btn_action action, int pin){return DisplayManager::instance()->onButtonDown(action, pin);}),
			true);
	buttons->trackPin(BUTTON_UP, (btn_callback)([](btn_action action, int pin){return DisplayManager::instance()->onButtonUp(action, pin);}),
			true);
	buttons->trackPin(BUTTON_LEFT, (btn_callback)([](btn_action action, int pin){return DisplayManager::instance()->onButtonLeft(action, pin);}),
			true);
	buttons->trackPin(BUTTON_RIGHT, (btn_callback)([](btn_action action, int pin){return DisplayManager::instance()->onButtonRight(action, pin);}),
			true);
}

void DisplayManager::update(){
	buttons->update();
	if(updateDisplay || millis() > lastUpdateTime + MAX_UPDATE_INTERVAL){
		lastUpdateTime = millis();
		display->clear();
		displayIP();
		displayMenu();
		display->display();
	}
}

void DisplayManager::displayTriangle(int line){
	if(line < 0 || line > 3)
		return;
	display->fillTriangle(0,line*16+5, 0,line*16+11, 6,line*16+8);
}

void DisplayManager::displayIP(){
	display->setTextAlignment(TEXT_ALIGN_RIGHT);
	if(DataManager::getInternetConnected())
		display->drawString(127, LINE_0_ORIG, DataManager::getIPAddress());
	else if(DataManager::getInetMode() == accesspoint)
		display->drawString(127, LINE_0_ORIG, "AP: 192.168.4.1");
	else
		display->drawString(127, LINE_0_ORIG, "disconnected");
}

void DisplayManager::displayMenu(){
	//draw texts
	display->setTextAlignment(TEXT_ALIGN_LEFT);
	display->drawString(MENU_ITEM_ORIG_X,LINE_1_ORIG,"Output");
	display->drawString(MENU_ITEM_ORIG_X,LINE_2_ORIG,menuItemStrings[dispStartItem-1]);
	display->drawString(MENU_ITEM_ORIG_X,LINE_3_ORIG,menuItemStrings[dispStartItem]);
	//draw values
	display->setTextAlignment(TEXT_ALIGN_RIGHT);
	display->drawString(127,LINE_1_ORIG,String(currentOutput));
	switch(dispStartItem){
	case dmx:
		display->drawString(127,LINE_2_ORIG,String(DataManager::getLedConfig(currentOutput)->startDmxAddress));
		display->drawString(127,LINE_3_ORIG,String(DataManager::getLedConfig(currentOutput)->startUniverse));
		break;
	case artnet:
		display->drawString(127,LINE_2_ORIG,String(DataManager::getLedConfig(currentOutput)->startUniverse));
		display->drawString(127,LINE_3_ORIG,String(DataManager::getLedConfig(currentOutput)->numberLEDs));
		break;
	case leds:
		display->drawString(127,LINE_2_ORIG,String(DataManager::getLedConfig(currentOutput)->numberLEDs));
		display->drawString(127,LINE_3_ORIG,(nextInternetMode==DataManager::getInetMode()?String(""):String("*")) +
				(nextInternetMode==wifiDHCP?String("WIFI"):(nextInternetMode==accesspoint?String("Setup"):String("Ethernet"))));
		break;
	}
	//draw triangle for active item
	if(activeItem == output)
		displayTriangle(1);
	else
		displayTriangle(2+activeItem-dispStartItem);
}

void DisplayManager::onButtonDown(btn_action action, int pin){
	if(activeItem==mode)
		return;
	if(activeItem-dispStartItem >= 1){
		activeItem++;
		dispStartItem++;
	}
	else{
		activeItem++;
	}
	updateDisplay = true;
	return;
}

void DisplayManager::onButtonUp(btn_action action, int pin){
	if(activeItem == output)
		return;
	if(activeItem == mode)
		DataManager::setInetMode(nextInternetMode);
	if(activeItem == dispStartItem && dispStartItem != dmx){
		dispStartItem--;
		activeItem--;
	}
	else{
		activeItem--;
	}
	return;
}

void DisplayManager::onButtonLeft(btn_action action, int pin){
	switch(activeItem){
	case output:
		if(currentOutput > 0)
			currentOutput--;
		break;
	case dmx:
		DataManager::setDMXAddress(currentOutput, DataManager::getLedConfig(currentOutput)->startDmxAddress-1);
		break;
	case artnet:
		DataManager::setArtnetUniverse(currentOutput, DataManager::getLedConfig(currentOutput)->startUniverse-1);
		break;
	case leds:
		DataManager::setNumberLeds(currentOutput, DataManager::getLedConfig(currentOutput)->numberLEDs-1);
		break;
	case mode:
		switch(nextInternetMode){
		case wifiDHCP:
			break;
		case ethernetDHCP:
			nextInternetMode = wifiDHCP;
			break;
		case accesspoint:
			nextInternetMode = ethernetDHCP;
			break;
		}
		break;
	}
	updateDisplay = true;
	return;
}

void DisplayManager::onButtonRight(btn_action action, int pin){
	switch(activeItem){
	case output:
		if(currentOutput < NUMBER_LED_OUTPUTS-1)
			currentOutput++;
		break;
	case dmx:
		DataManager::setDMXAddress(currentOutput, DataManager::getLedConfig(currentOutput)->startDmxAddress+1);
		break;
	case artnet:
		DataManager::setArtnetUniverse(currentOutput, DataManager::getLedConfig(currentOutput)->startUniverse+1);
		break;
	case leds:
		DataManager::setNumberLeds(currentOutput, DataManager::getLedConfig(currentOutput)->numberLEDs+1);
		break;
	case mode:
		switch(nextInternetMode){
		case wifiDHCP:
			nextInternetMode = ethernetDHCP;
			break;
		case ethernetDHCP:
			nextInternetMode = accesspoint;
			break;
		case accesspoint:
			break;
		}
		break;
	}
	updateDisplay = true;
	return;
}
