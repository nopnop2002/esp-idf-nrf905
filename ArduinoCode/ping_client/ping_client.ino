/*
 * Project: nRF905 Radio Library for Arduino (Ping client example)
 * Author: Zak Kemble, contact@zakkemble.net
 * Copyright: (C) 2020 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: https://blog.zakkemble.net/nrf905-avrarduino-librarydriver/
 */

/*
 * Time how long it takes to send some data and get a reply.
 * Should be around 14-17ms with default settings.
 */

#include <nRF905.h>
#include <SPI.h>

#define RXADDR 0xE7E7E7E7 // Address of this device
#define TXADDR 0xE7E7E7E7 // Address of device to send to

#define TIMEOUT 500 // 500ms ping timeout

#define PACKET_NONE    0
#define PACKET_RX_DONE 1
#define PACKET_INVALID 2
#define PACKET_TX_DONE 3
#define PACKET_ADDR_MATCH 4

#define PAYLOAD_SIZE  NRF905_MAX_PAYLOAD // 32

// Comment when using DR and AM Interrupt.
#define POLLING

nRF905 transceiver = nRF905();

static volatile uint8_t packetStatus;

#if !defined(POLLING)
// Don't modify these 2 functions. They just pass the DR/AM interrupt to the correct nRF905 instance.
void nRF905_int_dr(){transceiver.interrupt_dr();}
void nRF905_int_am(){transceiver.interrupt_am();}
#endif

// Event function for RX complete
void nRF905_onRxComplete(nRF905* device)
{
	Serial.println("nRF905_onRxComplete");
	packetStatus = PACKET_RX_DONE;
	//transceiver.standby();
}

// Event function for RX invalid
void nRF905_onRxInvalid(nRF905* device)
{
	Serial.println("nRF905_onRxInvalid");
	packetStatus = PACKET_INVALID;
	//transceiver.standby();
}

// Event function for TX complete
void nRF905_onTxComplete(nRF905* device)
{
	Serial.println("nRF905_onTxComplete");
	packetStatus = PACKET_TX_DONE;
	//transceiver.standby();
}

// Event function for Addr Match
void nRF905_onAddrMatch(nRF905* device)
{
	Serial.println("nRF905_onAddrMatch");
	packetStatus = PACKET_ADDR_MATCH;
	//transceiver.standby();
}

void setup()
{
	Serial.begin(115200);

	Serial.println(F("Client starting..."));

	// This must be called first
	SPI.begin();

#if defined(POLLING)

	// Minimal wires (polling status register)
	// Up to 5 wires can be disconnected, however this will reduce functionality and will put the library into polling mode instead of interrupt mode
	// In polling mode the .poll() method must be called as often as possible. If .poll() is not called often enough then events may be missed. (Search for .poll() in the loop() function below)
	transceiver.begin(
		SPI,
		10000000,
		6, // SPI SS (OUT)
		NRF905_PIN_UNUSED, // CE (standby) pin must be connected to VCC (3.3V) - Will always be in RX or TX mode
		9, // TRX (RX/TX mode)
		NRF905_PIN_UNUSED, // PWR pin must be connected to VCC (3.3V) - Will always be powered up
		NRF905_PIN_UNUSED, // Without the CD pin Carrier Detect will be disabled
		NRF905_PIN_UNUSED, // Without the DR pin the library will run in polling mode and poll the status register over SPI. This also means the nRF905 can not wake the MCU up from sleep mode
		NRF905_PIN_UNUSED, // Without the AM pin the library will run in polling mode and poll the status register over SPI.
		NULL, // No interrupt function
		NULL // No interrupt function
	);

#else

	// All wires, maximum functionality
	transceiver.begin(
		SPI, // SPI bus to use (SPI, SPI1, SPI2 etc)
		10000000, // SPI Clock speed (10MHz)
		6, // SPI SS (OUT)
		7, // CE (standby) (OUT)
		9, // TRX (RX/TX mode) (OUT)
		8, // PWR (power down) (OUT)
		4, // CD (collision avoid) (IN)
		3, // DR (data ready) (IN)
		2, // AM (address match) (IN)
		nRF905_int_dr, // Interrupt function for DR
		nRF905_int_am // Interrupt function for AM
	);

#endif


	// Register event functions
	transceiver.events(
		nRF905_onRxComplete,
		nRF905_onRxInvalid,
		nRF905_onTxComplete,
		NULL
	);

	// Set address of this device
	transceiver.setListenAddress(RXADDR);

	Serial.println(F("Client started"));
}

void loop()
{
	static uint8_t counter;
	static uint32_t sent;
	static uint32_t replies;
	static uint32_t timeouts;
	static uint32_t invalids;
	static uint32_t badData;

	// Make data
	uint8_t buffer[PAYLOAD_SIZE];
	memset(buffer, counter, PAYLOAD_SIZE);
	counter++;

	packetStatus = PACKET_NONE;

	// Show data
	Serial.print(F("Sending data: "));
	for(uint8_t i=0;i<PAYLOAD_SIZE;i++)
	{
		Serial.print(F(" "));
		Serial.print(buffer[i], DEC);
	}
	Serial.println();

	// Write reply data and destination address to radio IC
	transceiver.write(TXADDR, buffer, sizeof(buffer));

	uint32_t startTime = millis();

	// Send the data (send fails if other transmissions are going on, keep trying until success) and enter RX mode on completion
	while(!transceiver.TX(NRF905_NEXTMODE_RX, true));
	sent++;

	Serial.println(F("Waiting for reply..."));

	uint8_t success;

	// Wait for reply with timeout
	uint32_t sendStartTime = millis();
	while(1)
	{
#if defined(POLLING)
		transceiver.poll();
#endif

		success = packetStatus;
		if(success != PACKET_NONE)
			break;
		else if(millis() - sendStartTime > TIMEOUT)
			break;
	}

	if(success == PACKET_NONE)
	{
		Serial.println(F("Ping timed out"));
		timeouts++;
	}
	else if(success == PACKET_INVALID)
	{
		Serial.println(F("Invalid packet!"));
		invalids++;
	}
	else if(success == PACKET_RX_DONE)
	{
		uint16_t totalTime = millis() - startTime;
		replies++;

		Serial.print(F("Ping time: "));
		Serial.print(totalTime);
		Serial.println(F("ms"));

		// Get the reply data
		uint8_t replyBuffer[PAYLOAD_SIZE];
		transceiver.read(replyBuffer, sizeof(replyBuffer));

		// Validate data
		for(uint8_t i=0;i<PAYLOAD_SIZE;i++)
		{
			if(replyBuffer[i] != counter)
			{
				badData++;
				Serial.println(F("Bad data!"));
				break;
			}
		}

		// Print out ping contents
		Serial.print(F("Data from server:"));
		for(uint8_t i=0;i<PAYLOAD_SIZE;i++)
		{
			Serial.print(F(" "));
			Serial.print(replyBuffer[i], DEC);
		}
		Serial.println();
	}

	Serial.println(F("Totals:"));
	Serial.print(F(" Sent     "));
	Serial.println(sent);
	Serial.print(F(" Replies  "));
	Serial.println(replies);
	Serial.print(F(" Timeouts "));
	Serial.println(timeouts);
	Serial.print(F(" Invalid  "));
	Serial.println(invalids);
	Serial.print(F(" Bad      "));
	Serial.println(badData);
	Serial.println(F("------"));

	delay(1000);
}
