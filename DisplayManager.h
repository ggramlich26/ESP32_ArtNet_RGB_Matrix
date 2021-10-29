/*
 * DisplayManager.h
 *
 *  Created on: 28.10.2021
 *      Author: ag4716
 */

#ifndef DISPLAYMANAGER_H_
#define DISPLAYMANAGER_H_
#include <Wire.h>
#include "SSD1306Wire.h"
#include "DataManager.h"
#include "Buttons.h"
#include "config.h"

#define		MAX_UPDATE_INTERVAL	500
#define		DISP_FONT	ArialMT_Plain_16
#define		LINE_0_ORIG			0
#define		LINE_1_ORIG			16
#define		LINE_2_ORIG			32
#define		LINE_3_ORIG			48
#define		MENU_ITEM_ORIG_X	10

class DisplayManager {
public:
	static DisplayManager *instance(){
		if(!_instance)
			_instance = new DisplayManager();
		return _instance;
	}
	void init(int sda, int scl);
	void update();
private:
	static DisplayManager *_instance;
	DisplayManager();
	DisplayManager (const DisplayManager& );
	virtual ~DisplayManager();
	enum menuItems {output=0, dmx=1, artnet=2, leds=3, mode=4};
	SSD1306Wire *display;
	void displayIP();
	void displayTriangle(int line);
	void displayMenu();
	void onButtonDown(btn_action action, int pin);
	void onButtonUp(btn_action action, int pin);
	void onButtonLeft(btn_action action, int pin);
	void onButtonRight(btn_action action, int pin);

	String menuItemStrings[4] = {"DMX", "ArtNet", "#LEDs", "Mode"};
	bool updateDisplay = false;
	int dispStartItem = dmx;
	int activeItem = output;
	int currentOutput = 0;
	internetMode nextInternetMode;
	unsigned long lastUpdateTime = 0;
	Buttons *buttons;
};

#endif /* DISPLAYMANAGER_H_ */
