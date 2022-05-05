/*
 * Project: nRF905 Radio Library for Arduino
 * Author: Zak Kemble, contact@zakkemble.net
 * Copyright: (C) 2020 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: https://blog.zakkemble.net/nrf905-avrarduino-librarydriver/
 */

#ifndef NRF905_H_
#define NRF905_H_

#include <stdint.h>
#include "nRF905_config.h"

/**
* @brief Available modes after transmission complete.
*/
typedef enum
{
// TODO use nRF905_mode_t instead of nRF905_nextmode_t?

	NRF905_NEXTMODE_STANDBY, ///< Standby mode
	NRF905_NEXTMODE_RX, ///< Receive mode
	NRF905_NEXTMODE_TX ///< Transmit mode (will auto-retransmit if enabled, otherwise will transmit a carrier wave with no data)
} nRF905_nextmode_t;

/**
* @brief Current radio mode
*/
typedef enum
{
	NRF905_MODE_POWERDOWN, ///< Power-down mode
	NRF905_MODE_STANDBY, ///< Standby mode
	NRF905_MODE_RX, ///< Receive mode
	NRF905_MODE_TX, ///< Transmit mode
	NRF905_MODE_ACTIVE ///< Receive or transmit mode (when unable to tell due to the tx pin being unused, hardwired to VCC (TX) or GND (RX))
} nRF905_mode_t;

/**
* @brief Frequency bands.
*/
typedef enum
{
// NOTE:
// When using NRF905_BAND_868 and NRF905_BAND_915 for calculating channel (NRF905_CALC_CHANNEL(f, b)) they should be value 0x01,
// but when using them for setting registers their value should be 0x02.
// They're defined as 0x02 here so when used for calculating channel they're right shifted by 1

	NRF905_BAND_433 = 0x00,	///< 433MHz band
	NRF905_BAND_868 = 0x02,	///< 868/915MHz band
	NRF905_BAND_915 = 0x02	///< 868/915MHz band
} nRF905_band_t;

/**
* @brief Output power (n means negative, n10 = -10).
*/
typedef enum
{
	NRF905_PWR_n10 = 0x00,	///< -10dBm = 100uW
	NRF905_PWR_n2 = 0x04,	///< -2dBm = 631uW
	NRF905_PWR_6 = 0x08,	///< 6dBm = 4mW
	NRF905_PWR_10 = 0x0C	///< 10dBm = 10mW
} nRF905_pwr_t;

/**
* @brief Output a clock signal on pin 3 of IC.
*/
typedef enum
{
	NRF905_OUTCLK_DISABLE = 0x00,	///< Disable output clock
	NRF905_OUTCLK_4MHZ = 0x04,		///< 4MHz clock
	NRF905_OUTCLK_2MHZ = 0x05,		///< 2MHz clock
	NRF905_OUTCLK_1MHZ = 0x06,		///< 1MHz clock
	NRF905_OUTCLK_500KHZ = 0x07,	///< 500KHz clock (default)
} nRF905_outclk_t;

/**
* @brief CRC Checksum.
*
* The CRC is calculated across the address (SYNC word) and payload
*/
typedef enum
{
	NRF905_CRC_DISABLE = 0x00,	///< Disable CRC
	NRF905_CRC_8 = 0x40,		///< 8bit CRC (Don't know what algorithm is used for this one)
	NRF905_CRC_16 = 0xC0,		///< 16bit CRC (CRC16-CCITT-FALSE (0xFFFF))
} nRF905_crc_t;

/**
* @brief poll event.
*
*/
typedef enum
{
	NRF905_NONE,
	NRF905_RX_COMPLETE,
	NRF905_TX_COMPLETE,
	NRF905_ADDR_MATCH,
	NRF905_RX_INVALID,
} nRF905_poll_t;

#define NRF905_MAX_PAYLOAD		32 ///< Maximum payload size
#define NRF905_REGISTER_COUNT	10 ///< Configuration register count
#define NRF905_DEFAULT_RXADDR	0xE7E7E7E7 ///< Default receive address
#define NRF905_DEFAULT_TXADDR	0xE7E7E7E7 ///< Default transmit/destination address
#define NRF905_PIN_UNUSED		255 ///< Mark a pin as not used or not connected

#define NRF905_CALC_CHANNEL(f, b)	((((f) / (1 + (b>>1))) - 422400000UL) / 100000UL) ///< Workout channel from frequency & band

#define LOW  0
#define HIGH 1

#define delayMicroseconds(microsec) esp_rom_delay_us(microsec)
#define delay(millsec) esp_rom_delay_us(millsec*1000)
//#define delay(millsec) vTaskDelay(millsec/portTICK_RATE_MS)
#define millis() xTaskGetTickCount()*portTICK_PERIOD_MS

bool spi_write_byte(uint8_t* Dataout, size_t DataLength );
bool spi_read_byte(uint8_t* Datain, uint8_t* Dataout, size_t DataLength );
uint8_t spi_transfer(uint8_t value);
uint8_t nRF905_readConfigRegister(uint8_t reg);
void nRF905_writeConfigRegister(uint8_t reg, uint8_t val);
void nRF905_setConfigReg1(uint8_t val, uint8_t mask, uint8_t reg);
void nRF905_setConfigReg2(uint8_t val, uint8_t mask, uint8_t reg);
void nRF905_printConfig(void);
void nRF905_defaultConfig(void);
void nRF905_powerOn(bool val);
void nRF905_standbyMode(bool val);
void nRF905_txMode(bool val);
void nRF905_setAddress(uint32_t address, uint8_t cmd);
uint8_t nRF905_readStatus(void);
bool nRF905_dataReady(void);
bool nRF905_addressMatched(void);
void nRF905_gpioInterrupt(int pin);
void nRF905_begin(void);
void nRF905_setChannel(uint16_t channel);
void nRF905_setBand(nRF905_band_t band);
void nRF905_setAutoRetransmit(bool val);
void nRF905_setLowRxPower(bool val);
void nRF905_setTransmitPower(nRF905_pwr_t val);
void nRF905_setCRC(nRF905_crc_t val);
void nRF905_setClockOut(nRF905_outclk_t val);
void nRF905_setPayloadSize(uint8_t sizeTX, uint8_t sizeRX);
void nRF905_setAddressSize(uint8_t sizeTX, uint8_t sizeRX);
bool nRF905_receiveBusy(void);
bool nRF905_airwayBusy(void);
void nRF905_setListenAddress(uint32_t address);
void nRF905_write(uint32_t sendTo, void* data, uint8_t len);
void nRF905_read(void* data, uint8_t len);
bool nRF905_TX(nRF905_nextmode_t nextMode, bool collisionAvoid);
void nRF905_RX(void);
void nRF905_powerDown(void);
void nRF905_standby(void);
nRF905_mode_t nRF905_mode(void);
bool nRF905_inStandbyMode(void);
bool nRF905_poweredUp(void);
void nRF905_getConfigRegisters(void* regs);
//void nRF905_interrupt_dr();
//void nRF905_interrupt_am();
uint8_t nRF905_poll();

#endif /* NRF905_H_ */

