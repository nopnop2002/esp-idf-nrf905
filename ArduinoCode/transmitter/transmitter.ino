/*
 * Project: nRF905 Radio Library for Arduino (Trasnmitter example)
 * Author: Zak Kemble, contact@zakkemble.net
 * Copyright: (C) 2020 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: https://blog.zakkemble.net/nrf905-avrarduino-librarydriver/
 */

#include <nRF905.h> // https://github.com/ZakKemble/nRF905-arduino
#include <SPI.h>
#include <printf.h>

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

void printConfig() {
  uint8_t config[10];
  transceiver.getConfigRegisters(config);
  for (int i=0;i<10;i++) {
    Serial.print("Register[");
    Serial.print(i);
    Serial.print("]=0x");
    Serial.println(config[i], HEX);
  }
  uint16_t CH_NO = (config[1] & 0x01) << 8 | config[0];
  uint8_t  AUTO_RETRAN = (config[1] & 0x20) >> 4;
  uint8_t  RX_RED_PWR = (config[1] & 0x10) >> 3;
  uint8_t  PA_PWR = (config[1] & 0x0C) >> 2;
  uint8_t  HFREQ_PLL = (config[1] & 0x02) >> 1;
  uint8_t  TX_AFW = (config[2] & 0x70) >> 4;
  uint8_t  RX_AFW = (config[2] & 0x07) >> 0;
  uint8_t  RX_PW = (config[3] & 0x3F) >> 0;
  uint8_t  TX_PW = (config[4] & 0x3F) >> 0;
  uint32_t RX_ADDRESS = (config[5] << 24) | (config[6] << 16) | (config[7] << 8) | (config[8] << 0);
  uint8_t  CRC_MODE = (config[9] & 0x80) >> 7;
  uint8_t  CRC_EN = (config[9] & 0x40) >> 6;
  uint8_t  XOF = (config[9] & 0x38) >> 3;
  uint8_t  UP_CLK_EN = (config[9] & 0x04) >> 2;
  uint8_t  UP_CLK_FREQ = (config[9] & 0x03) >> 0;
  float    FRF = ( 422.4 + CH_NO /10)*(1+HFREQ_PLL);

  printf("================ NRF Configuration ================\n");
  printf("CH_NO            = %d\n", CH_NO);
  Serial.print("FRF              = ");
  Serial.print(FRF);
  Serial.print(" MHz");
  Serial.println();
  //printf("FRF              = %.2f MHz\n", FRF);
  if (AUTO_RETRAN == 0) {
  printf("AUTO_RETRAN      = No retransmission\n");
  } else {
  printf("AUTO_RETRAN      = Retransmission of data packet\n");
  }
  if (RX_RED_PWR == 0) {
  printf("RX_RED_PWR       = Normal operation\n");
  } else {
  printf("RX_RED_PWR       = Reduced power\n");
  }
  if (PA_PWR == 0) {
  printf("PA_PWR           = -10dBm\n");
  } else if (PA_PWR == 1) {
  printf("PA_PWR           = -2dBm\n");
  } else if (PA_PWR == 2) {
  printf("PA_PWR           = +6dBm\n");
  } else {
  printf("PA_PWR           = +10dBm\n");
  }
  if (HFREQ_PLL == 0) {
  printf("HFREQ_PLL        = Chip operating in 433MHz band\n");
  } else {
  printf("HFREQ_PLL        = Chip operating in 868 or 915 MHz band\n");
  }
  printf("TX_AFW           = %d byte TX address field width\n", TX_AFW);
  printf("RX_AFW           = %d byte RX address field width\n", RX_AFW);
  printf("TX_PW            = %d byte TX payload field width\n", TX_PW);
  printf("RX_PW            = %d byte RX payload field width\n", RX_PW);
  //printf("RX_ADDRESS       = 0x%"PRIx32"\n", RX_ADDRESS);
  printf("RX_ADDRESS       = 0x%x%x%x%x\n", config[5], config[6], config[7], config[8]);
  if (CRC_MODE == 0) {
  printf("CRC_MODE         = 8 CRC check bit\n");
  } else {
  printf("CRC_MODE         = 16 CRC check bit\n");
  }
  if (CRC_EN == 0) {
  printf("CRC_EN           = Disable\n");
  } else {
  printf("CRC_EN           = Enable\n");
  }
  if (XOF == 0) {
  printf("XOF              = 4MHz\n");
  } else if (XOF == 1) {
  printf("XOF              = 8MHz\n");
  } else if (XOF == 2) {
  printf("XOF              = 12MHz\n");
  } else if (XOF == 3) {
  printf("XOF              = 16MHz\n");
  } else {
  printf("XOF              = 20MHz\n");
  }
  if (UP_CLK_EN == 0) {
  printf("UP_CLK_EN        = No external clock signal available\n");
  } else {
  printf("UP_CLK_EN        = External clock signal enabled\n");
  }
  if (UP_CLK_FREQ == 0) {
  printf("UP_CLK_FREQ      = 4MHz\n");
  } else if (UP_CLK_FREQ == 1) {
  printf("UP_CLK_FREQ      = 2MHz\n");
  } else if (UP_CLK_FREQ == 2) {
  printf("UP_CLK_FREQ      = 1MHz\n");
  } else {
  printf("UP_CLK_FREQ      = 500kHz\n");
  }
}

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

	// Print current settings
	printf_begin();
	printConfig();
}

void loop()
{
	// Make data
	uint8_t buffer[PAYLOAD_SIZE];
	memset(buffer, 0, PAYLOAD_SIZE);
	sprintf(buffer,"Hello World %ld",micros());

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
