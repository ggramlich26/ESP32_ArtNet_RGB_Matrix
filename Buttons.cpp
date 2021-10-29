/*
 * Button.cpp
 *
 *  Created on: 29.10.2021
 *      Author: ag4716
 */

#include "Buttons.h"

Buttons* Buttons::_instance;

Buttons::Buttons() {

}

Buttons::~Buttons() {
}

void Buttons::update(){
	for(uint8_t i = 0; i < n_buttons; i++){
		int state = digitalRead(buttons[i].pin);
		if(buttons[i].pullup == true){
			if(buttons[i].state == high && state == 0){
				buttons[i].state = debounce_in;
				buttons[i].start_millis = millis();
			}
			else if(buttons[i].state == debounce_in){
				if(millis() - buttons[i].start_millis >= DEBOUNCE_INTERVAL){
					buttons[i].state = low;
				}
			}
			else if(buttons[i].state == low && state > 0){
				buttons[i].state = debounce_out;
				if(millis()-buttons[i].start_millis >= LONG_PRESS_INTERVAL){
					buttons[i].callback(long_press, buttons[i].pin);
				}
				else{
					buttons[i].callback(short_press, buttons[i].pin);
				}
			}
			else if(buttons[i].state == debounce_out){
				if(millis()-buttons[i].start_millis >= DEBOUNCE_INTERVAL){
					buttons[i].state = high;
				}
			}
		}
		else{
			if(buttons[i].state == low && state > 0){
				buttons[i].state = debounce_in;
				buttons[i].start_millis = millis();
			}
			else if(buttons[i].state == debounce_in){
				if(millis() - buttons[i].start_millis >= DEBOUNCE_INTERVAL){
					buttons[i].state = high;
				}
			}
			else if(buttons[i].state == high && state == 0){
				buttons[i].state = debounce_out;
				if(millis()-buttons[i].start_millis >= LONG_PRESS_INTERVAL){
					buttons[i].callback(long_press, buttons[i].pin);
				}
				else{
					buttons[i].callback(short_press, buttons[i].pin);
				}
			}
			else if(buttons[i].state == debounce_out){
				if(millis()-buttons[i].start_millis >= DEBOUNCE_INTERVAL){
					buttons[i].state = low;
				}
			}
		}
	}
}

void Buttons::trackPin(int pin, btn_callback callback, bool pullup){
	if(n_buttons == MAX_BUTTONS)
		return;
	buttons[n_buttons].pin = pin;
	buttons[n_buttons].callback = callback;
	buttons[n_buttons].pullup = pullup;
	buttons[n_buttons].start_millis = 0;
	if(pullup){
		pinMode(pin, INPUT_PULLUP);
		buttons[n_buttons].state = high;
	}
	else {
		pinMode(pin, INPUT);
		buttons[n_buttons].state = low;
	}
	n_buttons++;
}
