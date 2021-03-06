/*The MIT License (MIT)
Copyright (c) 2014 Nathana?l L?caud?
https://github.com/natcl/Artnet, http://forum.pjrc.com/threads/24688-Artnet-to-OctoWS2811
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

#ifndef ARTNET_H_
#define ARTNET_H_

#include <functional>
#include <Arduino.h>
#include <Udp.h>
#include <EthernetENC.h>
#include <EthernetUdp.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include "DataManager.h"

// UDP specific
#define ART_NET_PORT 6454
// Opcodes
#define ART_POLL 0x2000
#define ART_DMX 0x5000
#define ART_POLL_REPLY 0x2100
// Buffers
#define MAX_BUFFER_ARTNET 530
// Packet
#define ART_NET_ID "Art-Net"
#define ART_DMX_START 18

#define DMX_FUNC_PARAM uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data
typedef std::function <void (DMX_FUNC_PARAM)> StdFuncDmx_t;

class ArtNet {
public:
	ArtNet();
	virtual ~ArtNet();

	void begin(String hostname = "");
	void end();
	uint16_t read(void);
	/* returns 1 for Ok, or 0 on problem */
	int write(void);
	int write(IPAddress ip);
	void setByte(uint16_t pos, uint8_t value);
	void printPacketHeader(void);
	void printPacketContent(void);
	int sendArtPollReply();
	int sendArtPollReply(uint16_t universe, char *shortName, char *longName);

	// Flush the UDP buffer
	inline void flushBuffer(void)
	{
		Udp->flush();
	}

	// Return a pointer to the start of the DMX data
	inline uint8_t* getDmxFrame(void)
	{
		return artnetPacket + ART_DMX_START;
	}

	inline uint16_t getOpcode(void)
	{
		return opcode;
	}

	inline uint8_t getSequence(void)
	{
		return sequence;
	}

	inline uint16_t getUniverse(void)
	{
		return incomingUniverse;
	}

	inline void setUniverse(uint16_t universe)
	{
		outgoingUniverse = universe;
	}

	inline void setPhysical(uint8_t port)
	{
		physical = port;
	}

	[[deprecated]]
	 inline void setPhisical(uint8_t port)
	{
		setPhysical(port);
	}

	inline uint16_t getLength(void)
	{
		return dmxDataLength;
	}

	inline void setLength(uint16_t len)
	{
		dmxDataLength = len;
	}

	inline void setArtDmxCallback(void (*fptr)(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data))
	{
		artDmxCallback = fptr;
	}

	inline void setArtDmxFunc(StdFuncDmx_t func)
	{
		artDmxFunc = func;
	}

private:
	uint16_t makePacket(void);
	uint16_t makeArtPollReplyPacket(void);
	uint16_t makeArtPollReplyPacket(uint16_t universe, char *shortname, char *longName);

	UDP *Udp;
	String host;
	String artPollHost;
	uint8_t artnetPacket[MAX_BUFFER_ARTNET];
	uint16_t packetSize;
	uint16_t opcode;
	uint8_t sequence;
	uint8_t physical;
	uint16_t incomingUniverse;
	uint16_t outgoingUniverse;
	uint16_t dmxDataLength;
	void (*artDmxCallback)(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data);
	StdFuncDmx_t artDmxFunc;
	static const char artnetId[];
};

#endif /* ARTNET_H_ */
