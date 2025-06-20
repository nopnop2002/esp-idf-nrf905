/*
 * Project: nRF905 Radio Library for Arduino (Trasnmitter example)
 * Author: Zak Kemble, contact@zakkemble.net
 * Copyright: (C) 2020 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: https://blog.zakkemble.net/nrf905-avrarduino-librarydriver/
 */

#include <nRF905.h>
#include <SPI.h>

#define RXADDR 0xE7E7E7E7 // Address of this device
#define TXADDR 0xE7E7E7E7 // Address of device to send to
#define PAYLOAD_SIZE	NRF905_MAX_PAYLOAD // 32
#define CHANNEL 10

#define PACKET_NONE    0
#define PACKET_RX_DONE 1
#define PACKET_INVALID 2
#define PACKET_TX_DONE 3
#define PACKET_ADDR_MATCH 4

nRF905 transceiver = nRF905();

#if 0
// These are not used in the transmitter.
void nRF905_int_dr(){transceiver.interrupt_dr();}
void nRF905_int_am(){transceiver.interrupt_am();}
#endif

void setup()
{
	Serial.begin(115200);
	
	Serial.println(F("Transmitter started"));
	
	// This must be called first
	SPI.begin();

	// Minimal wires (polling)
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

  // Set channel to use
  transceiver.setChannel(CHANNEL);
  
	// Register event functions
	transceiver.events(
		NULL,
		NULL,
		NULL,
		NULL
	);

}

void loop()
{
	// Make data
	uint8_t buffer[PAYLOAD_SIZE];
	memset(buffer, 0, PAYLOAD_SIZE);
	sprintf(buffer,"now is %lu",micros());

	// Show data
	Serial.print(F("Sending data: ["));
	Serial.print((char *)buffer);
	Serial.println("]");

	// Write data
	transceiver.write(TXADDR, buffer, sizeof(buffer));

	// Send the data (send fails if other transmissions are going on, keep trying until success) and enter RX mode on completion
	while(!transceiver.TX(NRF905_NEXTMODE_RX, true));
   
	delay(1000);
}
