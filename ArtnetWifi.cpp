/*The MIT License (MIT)

Copyright (c) 2014 Nathanaël Lécaudé
https://github.com/natcl/Artnet, http://forum.pjrc.com/threads/24688-Artnet-to-OctoWS2811

Copyright (c) 2016,2019 Stephan Ruloff
https://github.com/rstephan

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
 */

#include "ArtnetWifi.h"


const char ArtnetWifi::artnetId[] = ART_NET_ID;

ArtnetWifi::ArtnetWifi() {}

void ArtnetWifi::begin(String hostname)
{
	Udp.begin(ART_NET_PORT);
	host = hostname;
	sequence = 1;
	physical = 0;
}

void ArtnetWifi::end(){
	Udp.stop();
}

uint16_t ArtnetWifi::read(void)
{
	packetSize = Udp.parsePacket();

	if (packetSize <= MAX_BUFFER_ARTNET && packetSize > 0)
	{
		Udp.read(artnetPacket, MAX_BUFFER_ARTNET);

		// Check that packetID is "Art-Net" else ignore
		if (memcmp(artnetPacket, artnetId, sizeof(artnetId)) != 0) {
			return 0;
		}

		opcode = artnetPacket[8] | artnetPacket[9] << 8;

		if (opcode == ART_DMX)
		{
			sequence = artnetPacket[12];
			incomingUniverse = artnetPacket[14] | artnetPacket[15] << 8;
			dmxDataLength = artnetPacket[17] | artnetPacket[16] << 8;

			if (artDmxCallback) (*artDmxCallback)(incomingUniverse, dmxDataLength, sequence, artnetPacket + ART_DMX_START);
			if (artDmxFunc) {
				artDmxFunc(incomingUniverse, dmxDataLength, sequence, artnetPacket + ART_DMX_START);
			}
			return ART_DMX;
		}
		if (opcode == ART_POLL)
		{
			artPollHost = IPAddress(Udp.remoteIP()).toString();
			return ART_POLL;
		}
	}

	return 0;
}

uint16_t ArtnetWifi::makePacket(void)
{
	uint16_t len;
	uint16_t version;

	memcpy(artnetPacket, artnetId, sizeof(artnetId));
	opcode = ART_DMX;
	artnetPacket[8] = opcode;
	artnetPacket[9] = opcode >> 8;
	version = 14;
	artnetPacket[11] = version;
	artnetPacket[10] = version >> 8;
	artnetPacket[12] = sequence;
	sequence++;
	if (!sequence) {
		sequence = 1;
	}
	artnetPacket[13] = physical;
	artnetPacket[14] = outgoingUniverse;
	artnetPacket[15] = outgoingUniverse >> 8;
	len = dmxDataLength + (dmxDataLength % 2); // make a even number
	artnetPacket[17] = len;
	artnetPacket[16] = len >> 8;

	return len;
}

uint16_t ArtnetWifi::makeArtPollReplyPacket(void){

	uint32_t ipAddress = WiFi.localIP();
	uint16_t portNumber = 0x1936;
	uint16_t versInfo = 1;
	uint16_t netSwitch = 0;	//todo
	uint16_t oem = 0x6362;
	uint8_t ubeaVersion = 0;
	uint8_t status1 = 0b00010000;
	uint16_t estaMan = 0x00;
	char shortName[18] = "Light Tube";//todo right pad with zeros
	char longName[64]; //todo
	char nodeReport[64]; //todo
	uint16_t numPorts = 0;
	uint32_t portTypes = 0x00000000;
	uint32_t goodInput = 0x08080808;
	uint32_t goodOutput = 0x00000000;
	uint32_t swIn = 0x00000000; //todo: possibly adapt
	uint32_t swOut = 0x00000000; //todo: possibly adapt
	uint8_t swVideo = 0;
	uint8_t swMacro = 0;
	uint8_t swRemote = 0;
	uint8_t style = 0;
	uint8_t mac[6];
	uint32_t bindIp = ipAddress;
	uint8_t bindIndex = 0;
	uint8_t status2 = 0b00000111;

	memcpy(artnetPacket, artnetId, sizeof(artnetId));
	opcode = ART_POLL_REPLY;
	artnetPacket[8] = opcode;
	artnetPacket[9] = opcode >> 8;

	artnetPacket[10] = ipAddress >> 24;
	artnetPacket[11] = ipAddress >> 16;
	artnetPacket[12] = ipAddress >> 8;
	artnetPacket[13] = ipAddress;
	artnetPacket[14] = portNumber >> 8;
	artnetPacket[15] = portNumber;
	artnetPacket[16] = versInfo >> 8;
	artnetPacket[17] = versInfo;
	artnetPacket[18] = netSwitch >> 8;
	artnetPacket[19] = netSwitch;
	artnetPacket[20] = oem >> 8;
	artnetPacket[21] = oem;
	artnetPacket[22] = ubeaVersion;
	artnetPacket[23] = status1;
	artnetPacket[24] = estaMan;
	artnetPacket[25] = estaMan >> 8;
	memcpy(artnetPacket+26, shortName, 18);
	memcpy(artnetPacket+44, longName, 64);
	memcpy(artnetPacket+108, nodeReport, 64);
	artnetPacket[172] = numPorts >> 8;
	artnetPacket[173] = numPorts;
	artnetPacket[174] = portTypes >> 24;
	artnetPacket[175] = portTypes >> 16;
	artnetPacket[176] = portTypes >> 8;
	artnetPacket[177] = portTypes;
	artnetPacket[178] = goodInput >> 24;
	artnetPacket[179] = goodInput >> 16;
	artnetPacket[180] = goodInput >> 8;
	artnetPacket[181] = goodInput;
	artnetPacket[182] = goodOutput >> 24;
	artnetPacket[183] = goodOutput >> 16;
	artnetPacket[184] = goodOutput >> 8;
	artnetPacket[185] = goodOutput;
	artnetPacket[186] = swIn >> 24;
	artnetPacket[187] = swIn >> 16;
	artnetPacket[188] = swIn >> 8;
	artnetPacket[189] = swIn;
	artnetPacket[190] = swOut >> 24;
	artnetPacket[191] = swOut >> 16;
	artnetPacket[192] = swOut >> 8;
	artnetPacket[193] = swOut;
	artnetPacket[194] = swVideo;
	artnetPacket[195] = swMacro;
	artnetPacket[196] = swRemote;
	artnetPacket[197] = 0x00;
	artnetPacket[198] = 0x00;
	artnetPacket[199] = 0x00;
	artnetPacket[200] = style;
	for(uint8_t i = 0; i < 6; i++){
		artnetPacket[201+i] = mac[i];
	}
	artnetPacket[207] = bindIp >> 24;
	artnetPacket[208] = bindIp >> 16;
	artnetPacket[209] = bindIp >> 8;
	artnetPacket[210] = bindIp;
	artnetPacket[211] = bindIndex;
	artnetPacket[212] = status2;
	for(uint8_t i = 0; i < 26; i++){
		artnetPacket[213+i] = 0;
	}
	return 239;
}

int ArtnetWifi::sendArtPollReply(){
	uint16_t len;

	len = makeArtPollReplyPacket();
	Udp.beginPacket(artPollHost.c_str(), ART_NET_PORT);
	Udp.write(artnetPacket, ART_DMX_START + len);

	return Udp.endPacket();
}

int ArtnetWifi::write(void)
{
	uint16_t len;

	len = makePacket();
	Udp.beginPacket(host.c_str(), ART_NET_PORT);
	Udp.write(artnetPacket, ART_DMX_START + len);

	return Udp.endPacket();
}

int ArtnetWifi::write(IPAddress ip)
{
	uint16_t len;

	len = makePacket();
	Udp.beginPacket(ip, ART_NET_PORT);
	Udp.write(artnetPacket, ART_DMX_START + len);

	return Udp.endPacket();
}

void ArtnetWifi::setByte(uint16_t pos, uint8_t value)
{
	if (pos > 512) {
		return;
	}
	artnetPacket[ART_DMX_START + pos] = value;
}

void ArtnetWifi::printPacketHeader(void)
{
	Serial.print("packet size = ");
	Serial.print(packetSize);
	Serial.print("\topcode = ");
	Serial.print(opcode, HEX);
	Serial.print("\tuniverse number = ");
	Serial.print(incomingUniverse);
	Serial.print("\tdata length = ");
	Serial.print(dmxDataLength);
	Serial.print("\tsequence n0. = ");
	Serial.println(sequence);
}

void ArtnetWifi::printPacketContent(void)
{
	for (uint16_t i = ART_DMX_START ; i < dmxDataLength ; i++){
		Serial.print(artnetPacket[i], DEC);
		Serial.print("  ");
	}
	Serial.println('\n');
}
