/*
 * webserver.c
 *
 *  Created on: Feb 9, 2020
 *      Author: tsugua
 */

#include "Webserver.h"
#include "DataManager.h"
#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#else
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

String handle_internet_settings_request(AsyncWebServerRequest *request);
String handle_led_settings_request(AsyncWebServerRequest *request);
String create_config_table_only();

AsyncWebServer server(80);

const char* SSID_INPUT = "input_ssid";
const char* PASSWORD_INPUT = "input_password";
const char* HOST_NAME_INPUT = "input_host_name";
const char* INTERNET_MODE_INPUT = "input_internet_mode";
const char* LED_OUTPUT_INPUT = "input_led_output";
const char* ARTNET_UNIVERSE_INPUT = "input_artnet_universe";
const char* DMX_ADDR_INPUT = "input_dmx_addr";
const char* NUMBER_LEDS_INPUT = "input_number_leds";
const char* SHORT_DESCRIPTOR_INPUT = "input_short_descriptor";
const char* LONG_DESCRIPTOR_INPUT = "input_long_descriptor";


String StringIndexHtmlStart ="<!DOCTYPE HTML> <html> <head> <title>LED controller settings</title> "
		"<style> form { display: table; } p { display: table-row; } label { display: table-cell; } "
		"input { display: table-cell; } "

			"table { font-family: arial, sans-serif; border-collapse: collapse; width: 100%; } "
			"td, th { border: 1px solid #dddddd; text-align: left; padding: 8px; } tr:nth-child(even) "
			"{ background-color: #dddddd; } "

		"</style> </head> "
		"<body>  ";

String StringIndexHtmlEnd =
		"<br>"
		"<h2>Edit configuration</h2> "
		"<form action=\"/get\" autocomplete=\"off\"> "
		"<p style=\"font-size:35px\"> "
		"<label for=\"input_led_output\">LED output to update: &nbsp </label> "
		"<input style=\"font-size:35px\" name=\"input_led_output\" min=0 type=\"number\"> </p> "
		"<p style=\"font-size:35px\"> "
		"<label for=\"input_artnet_universe\">ArtNet universe: &nbsp </label> "
		"<input style=\"font-size:35px\" name=\"input_artnet_universe\" min=0 type=\"number\"> </p> "
		"<p style=\"font-size:35px\"> <label for=\"input_dmx_addr\">DMX address: &nbsp </label> "
		"<input style=\"font-size:35px\" name=\"input_dmx_addr\" min=\"1\" max=\"512\" type=\"number\"> </p> "
		"<p style=\"font-size:35px\"> <label for=\"input_number_leds\">Number LEDs: &nbsp </label> "
		"<input style=\"font-size:35px\" name=\"input_number_leds\" min=\"0\" max=\"300\" type=\"number\"> </p> "
		"<p style=\"font-size:35px\"> <label for=\"input_short_descriptor\">Short descriptor: &nbsp </label> "
		"<input style=\"font-size:35px\" name=\"input_short_descriptor\" maxlength=18 type=\"text\"> </p> "
		"<p style=\"font-size:35px\"> <label for=\"input_long_descriptor\">Long descriptor: &nbsp </label> "
		"<input style=\"font-size:35px\" name=\"input_long_descriptor\" maxlength=64 type=\"text\"> </p> <br> "
		"<p> <input style=\"font-size:35px\" type=\"submit\" value=\"Set LED settings\"> </p> "
		"</form> "
		"<br> <br> "
		"<form action=\"/get\" autocomplete=\"off\"> <p style=\"font-size:35px\"> "
		"<label for=\"input_ssid\">New SSID: </label> "
		"<input style=\"font-size:35px\" name=\"input_ssid\" maxlength=30 type=\"text\"> </p> "
		"<p style=\"font-size:35px\"> <label for=\"input_password\">New password: &nbsp </label> "
		"<input style=\"font-size:35px\" name=\"input_password\" maxlength=30 type=\"text\"> </p> "
		"<p style=\"font-size:35px\"> <label for=\"input_host_name\">New host name: &nbsp </label> "
		"<input style=\"font-size:35px\" name=\"input_host_name\" maxlength=30 type=\"text\"> </p> "
		"<p style=\"font-size:35px\"> Internet mode: </p> <p style=\"font-size:35px\"> "
		"<input style=\"font-size:35px\" name=\"input_internet_mode_wifi_dhcp\" type=\"radio\"> "
		"<label for=\"input_internet_mode_wifi_dhcp\">WiFi DHCP &nbsp </label> "
		"<input style=\"font-size:35px\" name=\"input_internet_mode_ethernet_dhcp\" type=\"radio\"> "
		"<label for=\"input_internet_mode_ethernet_dhcp\">Ethernet DHCP &nbsp </label> "
		"<input style=\"font-size:35px\" name=\"input_internet_mode_access_point\" type=\"radio\"> "
		"<label for=\"input_internet_mode_access_point\">Access point &nbsp </label> </p> "
		"<p> <input style=\"font-size:35px\" type=\"submit\" value=\"Set Internet settings\"> </p> "
		"</form> "
		"</body> "
		"</html>";

void notFound(AsyncWebServerRequest *request) {
	request->send(404, "text/plain", "Not found");
}


void webserver_init(){
	// Send web page with input fields to client
	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send(200, "text/html", StringIndexHtmlStart + create_config_table_only() + StringIndexHtmlEnd);
	});

	server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
		String reply = handle_internet_settings_request(request);
		if(reply.equals("")){
			reply = handle_led_settings_request(request);
		}
		request->send(200, "text/html", "<p style=\"font-size:25\">" + reply + "</p>"
				"<br><a href=\"/\">Return</a>");
	});
	server.onNotFound(notFound);
	server.begin();
}

String handle_internet_settings_request(AsyncWebServerRequest *request){
	String new_ssid;
	String new_password;
	String new_host_name;
	internetMode newMode;
	bool update = false;
	bool updateMode = false;
	// GET newnew  SSID value on <ESP_IP>/get?input_ssid=<inputMessage>
	if (request->hasParam(SSID_INPUT)) {
		new_ssid = request->getParam(SSID_INPUT)->value();
		update = true;
	}
	// GET new password value on <ESP_IP>/get?input_password=<inputMessage>
	if (request->hasParam(PASSWORD_INPUT)) {
		new_password = request->getParam(PASSWORD_INPUT)->value();
		update = true;
	}
	// GET new host name value on <ESP_IP>/get?input_host_name=<inputMessage>
	if (request->hasParam(HOST_NAME_INPUT)) {
		new_host_name = request->getParam(HOST_NAME_INPUT)->value();
		update = true;
	}
	if(request->hasParam("input_internet_mode_wifi_dhcp")){
		newMode = wifiDHCP;
		updateMode = true;
	}
	if(request->hasParam("input_internet_mode_ethernet_dhcp")){
		newMode = ethernetDHCP;
		updateMode = true;
	}
	if(request->hasParam("input_internet_mode_access_point")){
		newMode = accesspoint;
		updateMode = true;
	}
	if(updateMode){
		DataManager::setInetMode(newMode);
	}
	if(update){
		return DataManager::setWIFICredentials((new_ssid != NULL?new_ssid.c_str():""),
				(new_password != NULL?new_password.c_str():""),
				(new_host_name != NULL?new_host_name.c_str():""));
	}
	return "";

}

String handle_led_settings_request(AsyncWebServerRequest *request){
	if(!request->hasParam(LED_OUTPUT_INPUT) || request->getParam(LED_OUTPUT_INPUT)->value().equals(""))
		return "Please enter an output to update from 0 to " + String(NUMBER_LED_OUTPUTS-1);
	int output = atoi(request->getParam(LED_OUTPUT_INPUT)->value().c_str());
	String reply = "";
	if(request->hasParam(ARTNET_UNIVERSE_INPUT)){
		String input = request->getParam(ARTNET_UNIVERSE_INPUT)->value();
		if(!input.equals("")){
			int data = atoi(input.c_str());
			reply += DataManager::setArtnetUniverse(output, data) + " <br> ";
		}
	}
	if(request->hasParam(DMX_ADDR_INPUT)){
		String input = request->getParam(DMX_ADDR_INPUT)->value();
		if(!input.equals("")){
			int data = atoi(input.c_str());
			reply += DataManager::setDMXAddress(output, data) + " <br> ";
		}
	}
	if(request->hasParam(NUMBER_LEDS_INPUT)){
		String input = request->getParam(NUMBER_LEDS_INPUT)->value();
		if(!input.equals("")){
			int data = atoi(input.c_str());
			reply += DataManager::setNumberLeds(output, data) + " <br> ";
		}
	}
	if(request->hasParam(SHORT_DESCRIPTOR_INPUT)){
		String input = request->getParam(SHORT_DESCRIPTOR_INPUT)->value();
		if(!input.equals("")){
			reply += DataManager::setShortName(output, input) + " <br> ";
		}
	}
	if(request->hasParam(LONG_DESCRIPTOR_INPUT)){
		String input = request->getParam(LONG_DESCRIPTOR_INPUT)->value();
		if(!input.equals("")){
			reply += DataManager::setLongName(output, input) + " <br> ";
		}
	}
	return reply;
}

String create_config_table_only(){
	String html =
			"<h2>Current configuration</h2> "
			"<table> "
			"<tr> <th>Output</th> <th>ArtNet universe</th> <th>DMX address</th> "
			"<th>Number LEDs</th> <th>Short descriptor</th> <th>Long descriptor</th> </tr> ";
	for(int i = 0; i < NUMBER_LED_OUTPUTS; i++){
		html += "<tr> "
				"<td> " + String(i) + "</td> " +
				"<td> " + String(DataManager::getLedConfig(i)->startUniverse) + "</td> "
				"<td> " + DataManager::getLedConfig(i)->startDmxAddress + "</td> "
				"<td> " + DataManager::getLedConfig(i)->numberLEDs + "</td> "
				"<td> " + DataManager::getLedConfig(i)->shortName + "</td> "
				"<td> " + DataManager::getLedConfig(i)->longName + "</td> "
				"</tr> ";
	}
	html += "</table> ";
	return html;
}

