/*
 * Project: nRF905 Radio Library for Arduino (Ping server example)
 * Author: Zak Kemble, contact@zakkemble.net
 * Copyright: (C) 2020 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: https://blog.zakkemble.net/nrf905-avrarduino-librarydriver/
 */

/*
 * Listen for packets and send them back
 */

#include <nRF905.h>
#include <SPI.h>

#define RXADDR 0xE7E7E7E7 // Address of this device
#define TXADDR 0xE7E7E7E7 // Address of device to send to

#define PACKET_NONE    0
#define PACKET_RX_DONE 1
#define PACKET_INVALID 2
#define PACKET_TX_DONE 3
#define PACKET_ADDR_MATCH 4

#define PAYLOAD_SIZE  NRF905_MAX_PAYLOAD

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

	Serial.println(F("Receiver starting..."));

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

	// All wires (Using DR and AM Interrupt)
	transceiver.begin(
		SPI, // SPI bus to use (SPI, SPI1, SPI2 etc)
		10000000, // SPI Clock speed (10MHz)
		6, // SPI SS
		NRF905_PIN_UNUSED, // CE (standby) pin must be connected to VCC (3.3V) - Will always be in RX or TX mode
		9, // TRX (RX/TX mode)
		NRF905_PIN_UNUSED, // PWR pin must be connected to VCC (3.3V) - Will always be powered up
		NRF905_PIN_UNUSED, // Without the CD pin Carrier Detect will be disabled
		3, // DR (data ready)
		2, // AM (address match)
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

	// Put into receive mode
	transceiver.RX();
}

void loop()
{
	static uint32_t pings;
	static uint32_t invalids;

#if defined(POLLING)
	transceiver.poll();
#endif

	if(packetStatus == PACKET_INVALID)
	{
		invalids++;
		packetStatus = PACKET_NONE;
		Serial.println(F("Invalid packet!"));
		transceiver.RX();
	}
	else if(packetStatus == PACKET_RX_DONE)
	{
		pings++;
		packetStatus = PACKET_NONE;
		Serial.println(F("Got pascket!"));

		// Make buffer for data
		uint8_t buffer[PAYLOAD_SIZE];

		// Read payload
		transceiver.read(buffer, sizeof(buffer));

		// Show received data
		Serial.print(F("Data from client:"));
		for(uint8_t i=0;i<PAYLOAD_SIZE;i++)
		{
			Serial.print(F(" "));
			Serial.print(buffer[i], DEC);
		}
		Serial.println();

		Serial.println(F("Totals:"));
		Serial.print(F(" Pings   "));
		Serial.println(pings);
		Serial.print(F(" Invalid "));
		Serial.println(invalids);
		Serial.println(F("------"));
	}

}
