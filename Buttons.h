/*
 * Button.h
 *
 *  Created on: 29.10.2021
 *      Author: ag4716
 */

#ifndef BUTTONS_H_
#define BUTTONS_H_

#include "Arduino.h"

#define DEBOUNCE_INTERVAL	100 //time in ms in which the button can bounce
#define LONG_PRESS_INTERVAL	500	//time in ms after which a button will be considered to be long pressed
#define MAX_BUTTONS	4 //max number of buttons that can be tracked

enum btn_action{short_press, long_press};
typedef void (*btn_callback)(btn_action action, int pin);


class Buttons {



void btn_main();
public:
	static Buttons *instance(){
		if(!_instance)
			_instance = new Buttons();
		return _instance;
	}

	void update();
	//register a button to be tracked
	void trackPin(int pin, btn_callback callback, bool pullup);

private:
	static Buttons *_instance;
	Buttons();
	Buttons (const Buttons& );
	virtual ~Buttons();


	enum btn_state{low, debounce_in, high, debounce_out};
	typedef struct button_s{
	int pin;
	long start_millis;
	btn_callback callback;
	btn_state state;
	bool pullup;
	} button_t;

	int n_buttons = 0;
	button_t buttons[MAX_BUTTONS];
};

#endif /* BUTTONS_H_ */
