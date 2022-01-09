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
#include <Ethernet.h>

String handle_internet_settings_request(AsyncWebServerRequest *request);
String handle_led_settings_request(AsyncWebServerRequest *request);
String handle_internet_settings_request(String ssid, String pw, String hostName, internetMode newMode, bool updateInternetMode);
String handle_led_settings_request(String output, String artnet, String dmx, String numberLEDs, String channelsPerUniverse, String shortDesc, String longDesc);
String create_config_table_only();
String ethernet_server_handle_form(String getLine);

AsyncWebServer server(80);
EthernetServer ethernetServer(80);

const char* SSID_INPUT = "input_ssid";
const char* PASSWORD_INPUT = "input_password";
const char* HOST_NAME_INPUT = "input_host_name";
const char* INTERNET_MODE_INPUT = "input_internet_mode";
const char* LED_OUTPUT_INPUT = "input_led_output";
const char* ARTNET_UNIVERSE_INPUT = "input_artnet_universe";
const char* DMX_ADDR_INPUT = "input_dmx_addr";
const char* NUMBER_LEDS_INPUT = "input_number_leds";
const char* CHANNELS_PER_UNIVERSE_INPUT = "input_channels_per_universe";
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
		"<p style=\"font-size:35px\"> <label for=\"input_channels_per_universe\">Channels per universe: &nbsp </label> "
		"<input style=\"font-size:35px\" name=\"input_channels_per_universe\" min=\"1\" max=\"512\" type=\"number\"> </p> "
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


void webserver_init_for_wifi(){
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

void webserver_init_for_ethernet(){
	ethernetServer.begin();
}

//this function has to be called regularily when running on ethernet
void webserver_update_for_ethernet(){
	// listen for incoming clients
	EthernetClient client = ethernetServer.available();
	if (client) {
		Serial.println("Got a client");
		String getLine;
		while (client.connected()) {
			if (client.available()) {
				//read one line
				char buffer[200];
				char c = 'a';
				int i = 0;
				while(c != '\n'){
					if(client.available()){
						c = client.read();
						buffer[i] = c;
						i++;
					}
				}
				buffer[i-2] = '\0';
				String line = String(buffer);
				Serial.println(line);
				//save GET line
				if(line.substring(0, 4).equals("GET ")){
					getLine = line;
				}
				//if request finished: an http request ends with a blank line
				if(line.equals("")){
					Serial.println("getLine: " + getLine);
					if(getLine.substring(0,6).equals("GET / ")){
						client.println("HTTP/1.1 200 OK");
						client.println("Content-Type: text/html");
						client.println();
						client.println(StringIndexHtmlStart + create_config_table_only() + StringIndexHtmlEnd);
					}
					else if(getLine.substring(0,9).equals("GET /get?")){
						String reply = ethernet_server_handle_form(getLine);
						client.println("HTTP/1.1 200 OK");
						client.println("Content-Type: text/html");
						client.println();
						client.print("<p style=\"font-size:25\">");
						client.print(reply);
						client.println("</p><br><a href=\"/\">Return</a>");
					}
					else{
						//not found
						client.println("HTTP/1.1 404 OK");
						client.println("Content-Type: text/plain");
						client.println();
						client.println("Not found");
					}
					break;
				}
			}
		}
		// give the web browser time to receive the data
		delay(1);
		// close the connection:
		client.stop();
	}
}

String handle_internet_settings_request(AsyncWebServerRequest *request){
	String ssid;
	String pw;
	String hostName;
	internetMode newMode;
	bool updateInternetMode = false;
	// GET newnew  SSID value on <ESP_IP>/get?input_ssid=<inputMessage>
	if (request->hasParam(SSID_INPUT)) {
		ssid = request->getParam(SSID_INPUT)->value();
	}
	// GET new password value on <ESP_IP>/get?input_password=<inputMessage>
	if (request->hasParam(PASSWORD_INPUT)) {
		pw = request->getParam(PASSWORD_INPUT)->value();
	}
	// GET new host name value on <ESP_IP>/get?input_host_name=<inputMessage>
	if (request->hasParam(HOST_NAME_INPUT)) {
		hostName = request->getParam(HOST_NAME_INPUT)->value();
	}
	if(request->hasParam("input_internet_mode_wifi_dhcp")){
		newMode = wifiDHCP;
		updateInternetMode = true;
	}
	if(request->hasParam("input_internet_mode_ethernet_dhcp")){
		newMode = ethernetDHCP;
		updateInternetMode = true;
	}
	if(request->hasParam("input_internet_mode_access_point")){
		newMode = accesspoint;
		updateInternetMode = true;
	}
	return handle_internet_settings_request(ssid, pw, hostName, newMode, updateInternetMode);

}

String handle_led_settings_request(AsyncWebServerRequest *request){
	String output = "";
	String artnet = "";
	String dmx = "";
	String numberLEDs = "";
	String channelsPerUniverse = "";
	String shortDesc = "";
	String longDesc = "";
	if(request->hasParam(LED_OUTPUT_INPUT))
		output = request->getParam(LED_OUTPUT_INPUT)->value();
	if(request->hasParam(ARTNET_UNIVERSE_INPUT)){
		artnet = request->getParam(ARTNET_UNIVERSE_INPUT)->value();
	}
	if(request->hasParam(DMX_ADDR_INPUT)){
		dmx = request->getParam(DMX_ADDR_INPUT)->value();
	}
	if(request->hasParam(NUMBER_LEDS_INPUT)){
		numberLEDs = request->getParam(NUMBER_LEDS_INPUT)->value();
	}
	if(request->hasParam(CHANNELS_PER_UNIVERSE_INPUT)){
		channelsPerUniverse = request->getParam(CHANNELS_PER_UNIVERSE_INPUT)->value();
	}
	if(request->hasParam(SHORT_DESCRIPTOR_INPUT)){
		shortDesc = request->getParam(SHORT_DESCRIPTOR_INPUT)->value();
	}
	if(request->hasParam(LONG_DESCRIPTOR_INPUT)){
		longDesc = request->getParam(LONG_DESCRIPTOR_INPUT)->value();
	}
	return handle_led_settings_request(output, artnet, dmx, numberLEDs, channelsPerUniverse, shortDesc, longDesc);
}

String create_config_table_only(){
	String html =
			"<h2>Current configuration</h2> "
			"<table> "
			"<tr> <th>Output</th> <th>ArtNet universe</th> <th>DMX address</th> "
			"<th>Number LEDs</th> <th>Channels per Universe</th> "
			"<th>Short descriptor</th> <th>Long descriptor</th> </tr> ";
	for(int i = 0; i < NUMBER_LED_OUTPUTS; i++){
		html += "<tr> "
				"<td> " + String(i) + "</td> " +
				"<td> " + String(DataManager::getLedConfig(i)->startUniverse) + "</td> "
				"<td> " + DataManager::getLedConfig(i)->startDmxAddress + "</td> "
				"<td> " + DataManager::getLedConfig(i)->numberLEDs + "</td> "
				"<td> " + DataManager::getLedConfig(i)->channelsPerUniverse + "</td> "
				"<td> " + DataManager::getLedConfig(i)->shortName + "</td> "
				"<td> " + DataManager::getLedConfig(i)->longName + "</td> "
				"</tr> ";
	}
	html += "</table> ";
	return html;
}

String ethernet_server_handle_form(String getLine){
	String ssid = "";
	String pw = "";
	String hostName = "";
	String inetMode = "";
	String output = "";
	String artnet = "";
	String dmx = "";
	String leds = "";
	String channels = "";
	String shortdesc = "";
	String longdesc = "";
	//replace space after last parameter with a & sign for easier processing later on
	int index = getLine.lastIndexOf('=');
	if(index != -1){
		index = getLine.indexOf(' ', index);
		getLine[index] = '&';
	}
	index = getLine.indexOf(SSID_INPUT);
	if(index != -1){
		int startIndex = index + String(SSID_INPUT).length()+1;
		int endIndex = getLine.indexOf("&",startIndex);
		ssid = getLine.substring(startIndex, endIndex);
	}
	index = getLine.indexOf(PASSWORD_INPUT);
	if(index != -1){
		int startIndex = index + String(PASSWORD_INPUT).length()+1;
		int endIndex = getLine.indexOf("&",startIndex);
		pw = getLine.substring(startIndex, endIndex);
	}
	index = getLine.indexOf(HOST_NAME_INPUT);
	if(index != -1){
		int startIndex = index + String(HOST_NAME_INPUT).length()+1;
		int endIndex = getLine.indexOf("&",startIndex);
		hostName = getLine.substring(startIndex, endIndex);
	}
	bool updateInternetMode = false;
	internetMode newMode = wifiDHCP;
	if(getLine.indexOf("input_internet_mode_wifi_dhcp") != -1){
		updateInternetMode = true;
		newMode = wifiDHCP;
	}
	if(getLine.indexOf("input_internet_mode_ethernet_dhcp") != -1){
		newMode = ethernetDHCP;
		updateInternetMode = true;
	}
	if(getLine.indexOf("input_internet_mode_access_point") != -1){
		newMode = accesspoint;
		updateInternetMode = true;
	}
	String reply = handle_internet_settings_request(ssid, pw, hostName, newMode, updateInternetMode);
	if(!reply.equals("")){
		return reply;
	}

	//handle led settings
	index = getLine.indexOf(LED_OUTPUT_INPUT);
	if(index != -1){
		int startIndex = index + String(LED_OUTPUT_INPUT).length()+1;
		int endIndex = getLine.indexOf("&",startIndex);
		output = getLine.substring(startIndex, endIndex);
	}
	index = getLine.indexOf(ARTNET_UNIVERSE_INPUT);
	if(index != -1){
		int startIndex = index + String(ARTNET_UNIVERSE_INPUT).length()+1;
		int endIndex = getLine.indexOf("&",startIndex);
		artnet = getLine.substring(startIndex, endIndex);
	}
	index = getLine.indexOf(DMX_ADDR_INPUT);
	if(index != -1){
		int startIndex = index + String(DMX_ADDR_INPUT).length()+1;
		int endIndex = getLine.indexOf("&",startIndex);
		dmx = getLine.substring(startIndex, endIndex);
	}
	index = getLine.indexOf(NUMBER_LEDS_INPUT);
	if(index != -1){
		int startIndex = index + String(NUMBER_LEDS_INPUT).length()+1;
		int endIndex = getLine.indexOf("&",startIndex);
		leds = getLine.substring(startIndex, endIndex);
	}
	index = getLine.indexOf(CHANNELS_PER_UNIVERSE_INPUT);
	if(index != -1){
		int startIndex = index + String(CHANNELS_PER_UNIVERSE_INPUT).length()+1;
		int endIndex = getLine.indexOf("&",startIndex);
		channels = getLine.substring(startIndex, endIndex);
	}
	index = getLine.indexOf(SHORT_DESCRIPTOR_INPUT);
	if(index != -1){
		int startIndex = index + String(SHORT_DESCRIPTOR_INPUT).length()+1;
		int endIndex = getLine.indexOf("&",startIndex);
		shortdesc = getLine.substring(startIndex, endIndex);
	}
	index = getLine.indexOf(LONG_DESCRIPTOR_INPUT);
	if(index != -1){
		int startIndex = index + String(LONG_DESCRIPTOR_INPUT).length()+1;
		int endIndex = getLine.indexOf("&",startIndex);
		longdesc = getLine.substring(startIndex, endIndex);
	}
	return handle_led_settings_request(output, artnet, dmx, leds, channels, shortdesc, longdesc);
}

String handle_internet_settings_request(String ssid, String pw, String hostName, internetMode newMode, bool updateInternetMode){
	bool update = false;
	if(ssid != NULL && !ssid.equals(""))
		update = true;
	if(pw != NULL && !pw.equals(""))
		update = true;
	if(hostName != NULL && !hostName.equals(""))
		update = true;
	String reply = "";
	if(updateInternetMode){
		DataManager::setInetMode(newMode);
		reply += "Successfully set internet mode <br>";
	}
	if(update){
		reply += DataManager::setWIFICredentials((ssid != NULL?ssid.c_str():""),
				(pw != NULL?pw.c_str():""),
				(hostName != NULL?hostName.c_str():""));
	}
	return reply;
}

String handle_led_settings_request(String output, String artnet, String dmx, String numberLEDs, String channelsPerUniverse, String shortDesc, String longDesc){
	if(output == NULL || output.equals(""))
		return "Please enter an output to update from 0 to " + String(NUMBER_LED_OUTPUTS-1);
	int outputVal = atoi(output.c_str());
	String reply = "";
	if(artnet != NULL && !artnet.equals("")){
		int data = atoi(artnet.c_str());
		reply += DataManager::setArtnetUniverse(outputVal, data) + " <br> ";
	}
	if(dmx != NULL && !dmx.equals("")){
		int data = atoi(dmx.c_str());
		reply += DataManager::setDMXAddress(outputVal, data) + " <br> ";
	}
	if(numberLEDs != NULL && !numberLEDs.equals("")){
		int data = atoi(numberLEDs.c_str());
		reply += DataManager::setNumberLeds(outputVal, data) + " <br> ";
	}
	if(channelsPerUniverse != NULL && !channelsPerUniverse.equals("")){
		int data = atoi(channelsPerUniverse.c_str());
		reply += DataManager::setChannelsPerUniverse(outputVal, data) + " <br> ";
	}
	if(shortDesc != NULL && !shortDesc.equals("")){
		shortDesc.replace("+", " ");
		reply += DataManager::setShortName(outputVal, shortDesc) + " <br> ";
	}
	if(longDesc != NULL && !longDesc.equals("")){
		longDesc.replace("+", " ");
		reply += DataManager::setLongName(outputVal, longDesc) + " <br> ";
	}
	return reply;
}

