#include <Arduino.h>
#include <NativeEthernet.h>
#include <ArtNode.h>
#include <OctoWS2811.h>
#include <TaskManager.h>

#define VERSION_HI 0
#define VERSION_LO 1
/*
ArtConfig config = {
{0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}, // MAC
{10, 0, 0, 52},                         // IP
{255, 255, 255, 0},                       // Subnet mask
0x1936                               // UDP port
};
*/
ArtConfig config = {
	{0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED},
	//{0x00, 0x00, 0x00, 0x1F, 0x25, 0x98},
	{10, 0, 0, 50},		// IP
	{255, 255, 255, 0}, // Subnet mask
	0x1936,				// UDP port
	false,				// DHCP
	0,
	0,																														  // Net (0-127) and subnet (0-15)
	"ArtNode",																												  // Short name
	"ArtNode",																												  // Long name
	4,																														  // Number of ports
	{PortTypeDmx | PortTypeOutput, PortTypeDmx | PortTypeOutput, PortTypeDmx | PortTypeOutput, PortTypeDmx | PortTypeOutput}, // Port types
	{0, 1, 2, 3},																											  // Port input universes (0-15)
	{0, 1, 2, 3},																											  // Port output universes (0-15)
	VERSION_HI,
	VERSION_LO};

IPAddress gateway(config.ip[0], config.ip[1], config.ip[2], 1);
EthernetUDP udp;
byte buffer[1024];
ArtNode node = ArtNode(config, sizeof(buffer), buffer);

// OctoWS2811 settings
const int ledsPerStrip = 680; // change for your setup
const byte numStrips = 8;	  // change for your setup
DMAMEM int displayMemory[ledsPerStrip * 6];
int drawingMemory[ledsPerStrip * 6];
const int octoConfig = WS2811_GRB | WS2811_800kHz;
// OctoWS2811 leds(ledsPerStrip, displayMemory, drawingMemory, octoConfig);

const int startUniverse = 0;							   // CHANGE FOR YOUR SETUP most software this is 1, some software send out artnet first universe as zero.
const int numberOfChannels = ledsPerStrip * numStrips * 3; // Total number of channels you want to receive (1 led = 3 channels)
byte channelBuffer[numberOfChannels];					   // Combined universes into a single array

// Check if we got all universes
const int maxUniverses = 8;
bool universesReceived[maxUniverses];
bool sendFrame;

const uint8_t dipSwPin[8] = {33, 34, 41, 36, 37, 38, 39, 40}; //{23,22,19,18,0,1,3,4};
const int numPins = 8;
uint8_t pinList[numPins] = {1, 8, 14, 17, 20, 24, 29, 35};

OctoWS2811 leds(ledsPerStrip, displayMemory, drawingMemory, octoConfig, numPins, pinList);

constexpr uint8_t
	ledPin = 13,
	linkLedPin = 19;
uint8_t deviceId = 0;

void updateDipSw()
{
	uint8_t newId = 0;
	for (uint8_t i = 0; i < 8; i++)
	{
		newId |= (!digitalRead(dipSwPin[i])) << i;
	}
	if ( newId != deviceId)
	{
		deviceId = newId;
		Serial.printf("New ID:%d\n", deviceId);
	}

}
// template
void initTest();

void setup()
{
	Serial.begin(9600);
	pinMode(linkLedPin, OUTPUT);
	pinMode(ledPin, OUTPUT);

	leds.begin();
	initTest();
	digitalWrite(ledPin, HIGH);


	for (uint8_t i = 0; i < 8; i++)
	{
		pinMode(dipSwPin[i], INPUT_PULLUP);
		// delayMicroseconds(100);
		deviceId |= (!digitalRead(dipSwPin[i])) << i;
	}
	config.mac[5] = 50 + deviceId;
	config.ip[3] = 50 + deviceId;
	// myIp[3] = deviceId + 110;
	// mac[5] = deviceId;

	// Ethernet.begin(config.mac, config.ip,  gateway, gateway, config.subnet);
	Ethernet.begin(config.mac, config.ip);
	udp.begin(config.udpPort);

	Tasks.add("DipSwPoll",[] {updateDipSw();})->startFps(10);

}

void loop()
{
	Tasks.update();
	sendFrame = true;
	while (udp.parsePacket())
	{
		int n = udp.read(buffer, min(udp.available(), sizeof(buffer)));
		IPAddress remote = udp.remoteIP();
		SerialUSB.print("From ");
		for (int i = 0; i < 4; i++)
		{
			SerialUSB.print(remote[i], DEC);
			if (i < 3)
			{
				SerialUSB.print(".");
			}
		}
		// SerialUSB.print(millis());
		SerialUSB.print(", length:");
		SerialUSB.println(n);

		if (n >= sizeof(ArtHeader) && node.isPacketValid())
		{

			// Package Op-Code determines type of packet
			switch (node.getOpCode())
			{

			// Poll packet. Send poll reply.
			case OpPoll:
			{
				SerialUSB.println("op poll !!!");

				// ArtPoll* poll = (ArtPoll*)buffer;
				node.createPollReply();
				udp.beginPacket(remote, config.udpPort);
				udp.write(buffer, sizeof(ArtPollReply));
				udp.endPacket();
			}
			break;

			// DMX packet
			case OpDmx:
			{
				ArtDmx *dmx = (ArtDmx *)buffer;
				int port = node.getPort(dmx->SubUni, dmx->Net);
				int len = dmx->getLength();
				int universe = dmx->getUni();
				byte *data = dmx->Data;

				SerialUSB.print(millis());
				SerialUSB.print(", port:");
				SerialUSB.print(port);
				SerialUSB.print(", length:");
				SerialUSB.print(len);
				SerialUSB.print(", universe:");
				SerialUSB.println(dmx->getUni());

				universesReceived[universe] = 1;

				for (int i = 0; i < ledsPerStrip * 3; i++)
				{
					int bufferIndex = i + ((universe - startUniverse) * ledsPerStrip * 3);
					if (bufferIndex < numberOfChannels) // to verify
						channelBuffer[bufferIndex] = byte(data[i]);
				}
			}
			break;

			default:
				break;
			}
		}
		else
		{
			SerialUSB.println("isPacketValid !!!");
		}
	}

	for (uint8_t i = 0; i < numStrips; i++)
	{
		if (0 == universesReceived[i])
		{
			sendFrame = false;
			break;
		}
	}

	if (sendFrame)
	{
		// send to leds
		for (int i = 0; i < ledsPerStrip * numStrips; i++)
		{
			leds.setPixel(i, channelBuffer[(i)*3], channelBuffer[(i * 3) + 1], channelBuffer[(i * 3) + 2]);
		}
		leds.show();
		// Reset universeReceived to 0
		memset(universesReceived, 0, maxUniverses);
	}
}
void initTest()
{
	for (int i = 0; i < ledsPerStrip * numStrips; i++)
		leds.setPixel(i, 127, 0, 0);
	leds.show();
	delay(500);
	for (int i = 0; i < ledsPerStrip * numStrips; i++)
		leds.setPixel(i, 0, 127, 0);
	leds.show();
	delay(500);
	for (int i = 0; i < ledsPerStrip * numStrips; i++)
		leds.setPixel(i, 0, 0, 127);
	leds.show();
	delay(500);
	for (int i = 0; i < ledsPerStrip * numStrips; i++)
		leds.setPixel(i, 0, 0, 0);
	leds.show();
}