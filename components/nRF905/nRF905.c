/*
 * Project: nRF905 Radio Library for Arduino
 * Author: Zak Kemble, contact@zakkemble.net
 * Copyright: (C) 2020 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: https://blog.zakkemble.net/nrf905-avrarduino-librarydriver/
 */

#include <stdio.h>
#include <inttypes.h>
#include <stdint.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "nRF905.h"
#include "nRF905_config.h"
#include "nRF905_defs.h"

#define TAG "NRF905"

// SPI Stuff
#if CONFIG_SPI2_HOST
#define HOST_ID SPI2_HOST
#elif CONFIG_SPI3_HOST
#define HOST_ID SPI3_HOST
#endif

#define ESP_INTR_FLAG_DEFAULT 0

static int __csn; // SPI Chip Select.
static int __ce; //  Enables chip for receive and transmi pin.
static int __txen; // TX_EN=”1”TX mode, TX_EN=”0”RX mode.
static int __pwr; // Power up chip pin.
static int __cd; // Without the CD pin the library will run in polling mode and poll the status register over SPI.
static int __dr; // Without the DR pin the library will run in polling mode and poll the status register over SPI.
static int __am; // Without the AM pin the library will run in polling mode and poll the status register over SPI.
static spi_device_handle_t __spi; // SPI Handle
static bool __polledMode;
//static EventGroupHandle_t gpio_event_group = NULL;

static const uint8_t config[] = {
	NRF905_CMD_W_CONFIG,
	NRF905_CHANNEL,
	NRF905_AUTO_RETRAN | NRF905_LOW_RX | NRF905_PWR | NRF905_BAND | ((NRF905_CHANNEL>>8) & 0x01),
	(NRF905_ADDR_SIZE_TX<<4) | NRF905_ADDR_SIZE_RX,
	NRF905_PAYLOAD_SIZE_RX,
	NRF905_PAYLOAD_SIZE_TX,
	(uint8_t)NRF905_ADDRESS, (uint8_t)(NRF905_ADDRESS>>8), (uint8_t)(NRF905_ADDRESS>>16), (uint8_t)(NRF905_ADDRESS>>24),
	NRF905_CRC | NRF905_CLK_FREQ | NRF905_OUTCLK
};

#if 0
inline uint8_t nRF905_cselect()
{
	spi.beginTransaction(spiSettings);
	digitalWrite(csn, LOW);
	return 1;
}
#endif

#if 0
inline uint8_t nRF905_cdeselect()
{
	digitalWrite(csn, HIGH);
	spi.endTransaction();
	return 0;
}
#endif

// Can be in any mode to write registers, but standby or power-down is recommended
#if 0
#define CHIPSELECT()	for(uint8_t _cs = cselect(); _cs; _cs = cdeselect())
#endif

bool spi_write_byte(uint8_t* Dataout, size_t DataLength )
{
	spi_transaction_t SPITransaction;

	if ( DataLength > 0 ) {
		memset( &SPITransaction, 0, sizeof( spi_transaction_t ) );
		SPITransaction.length = DataLength * 8;
		SPITransaction.tx_buffer = Dataout;
		SPITransaction.rx_buffer = NULL;
		spi_device_transmit( __spi, &SPITransaction );
	}

	return true;
}

bool spi_read_byte(uint8_t* Datain, uint8_t* Dataout, size_t DataLength )
{
	spi_transaction_t SPITransaction;

	if ( DataLength > 0 ) {
		memset( &SPITransaction, 0, sizeof( spi_transaction_t ) );
		SPITransaction.length = DataLength * 8;
		SPITransaction.tx_buffer = Dataout;
		SPITransaction.rx_buffer = Datain;
		spi_device_transmit( __spi, &SPITransaction );
	}

	return true;
}

uint8_t spi_transfer(uint8_t value)
{
	uint8_t datain[1];
	uint8_t dataout[1];
	dataout[0] = value;
	spi_read_byte(datain, dataout, 1 );
	return datain[0];
}


uint8_t nRF905_readConfigRegister(uint8_t reg)
{
	gpio_set_level(__csn, 0);
	spi_transfer(NRF905_CMD_R_CONFIG | reg );
	uint8_t val = spi_transfer(NRF905_CMD_NOP);
	gpio_set_level(__csn, 1);
	return val;
}

void nRF905_writeConfigRegister(uint8_t reg, uint8_t val)
{
	gpio_set_level(__csn, 0);
	spi_transfer(NRF905_CMD_W_CONFIG | reg );
	spi_transfer(val);
	gpio_set_level(__csn, 1);
}

void nRF905_setConfigReg1(uint8_t val, uint8_t mask, uint8_t reg)
{
	// TODO atomic read/write?
	nRF905_writeConfigRegister(reg, (nRF905_readConfigRegister(NRF905_REG_CONFIG1) & mask) | val);
}

void nRF905_setConfigReg2(uint8_t val, uint8_t mask, uint8_t reg)
{
	// TODO atomic read/write?
	nRF905_writeConfigRegister(reg, (nRF905_readConfigRegister(NRF905_REG_CONFIG2) & mask) | val);
}

void nRF905_printConfig(void)
{
	uint8_t config[10];
	for (int reg=0;reg<10;reg++) {
		config[reg] = nRF905_readConfigRegister(reg);
		ESP_LOGD(TAG, "%d:%02x", reg, config[reg]);
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
	printf("FRF              = %.2f MHz\n", FRF);
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
	printf("RX_ADDRESS       = 0x%"PRIx32"\n", RX_ADDRESS);
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

void nRF905_defaultConfig(void)
{
	// Should be in standby mode

	// Set control registers
	gpio_set_level(__csn, 0);
	for(uint8_t i=0;i<sizeof(config);i++)
		spi_transfer(config[i]);
	gpio_set_level(__csn, 1);

	// Default transmit address
	// TODO is this really needed?
	gpio_set_level(__csn, 0);
	spi_transfer(NRF905_CMD_W_TX_ADDRESS);
	for(uint8_t i=0;i<4;i++)
		spi_transfer(0xE7);
	gpio_set_level(__csn, 1);

	// Clear transmit payload
	// TODO is this really needed?
	gpio_set_level(__csn, 0);
	spi_transfer(NRF905_CMD_W_TX_PAYLOAD);
	for(uint8_t i=0;i<NRF905_MAX_PAYLOAD;i++)
		spi_transfer(0x00);
	gpio_set_level(__csn, 1);

	if(__pwr == NRF905_PIN_UNUSED)
	{
		// Clear DR by reading receive payload
		gpio_set_level(__csn, 0);
		spi_transfer(NRF905_CMD_R_RX_PAYLOAD);
		for(uint8_t i=0;i<NRF905_MAX_PAYLOAD;i++)
			spi_transfer(NRF905_CMD_NOP);
		gpio_set_level(__csn, 1);
	}
}

// NOTE: SPI registers can still be accessed when in power-down mode
void nRF905_powerOn(bool val)
{
	if(__pwr != NRF905_PIN_UNUSED)
		gpio_set_level(__pwr, val ? HIGH : LOW);
}

void nRF905_standbyMode(bool val)
{
	if(__ce != NRF905_PIN_UNUSED)
		gpio_set_level(__ce, val ? LOW : HIGH);
}

void nRF905_txMode(bool val)
{
	if(__txen != NRF905_PIN_UNUSED)
		gpio_set_level(__txen, val ? HIGH : LOW);
}

void nRF905_setAddress(uint32_t address, uint8_t cmd)
{
	gpio_set_level(__csn, 0);
	spi_transfer(cmd);
	//spi.transfer16(address);
	//spi.transfer16(address>>16);
	for(uint8_t i=0;i<4;i++)
		spi_transfer(address>>(8 * i));
	//spi.transfer(&address, 4);
	gpio_set_level(__csn, 1);
}

uint8_t nRF905_readStatus(void)
{
	uint8_t status = 0;
	gpio_set_level(__csn, 0);
	status = spi_transfer(NRF905_CMD_NOP);
	gpio_set_level(__csn, 1);
	return status;
}

bool nRF905_dataReady(void)
{
	if(__dr == NRF905_PIN_UNUSED)
		return (nRF905_readStatus() & (1<<NRF905_STATUS_DR));
	return gpio_get_level(__dr);
}

bool nRF905_addressMatched(void)
{
	if(__am == NRF905_PIN_UNUSED)
		return (nRF905_readStatus() & (1<<NRF905_STATUS_AM));
	return gpio_get_level(__am);
}

void nRF905_begin()
{
	esp_err_t ret;

	// nRF905 stuff
	ESP_LOGI(TAG, "CONFIG_TXEN_GPIO=%d", CONFIG_TXEN_GPIO);
	ESP_LOGI(TAG, "CONFIG_CE_GPIO=%d", CONFIG_CE_GPIO);
	ESP_LOGI(TAG, "CONFIG_PWR_GPIO=%d", CONFIG_PWR_GPIO);
	__txen = CONFIG_TXEN_GPIO;
	//__ce = NRF905_PIN_UNUSED;
	__ce = CONFIG_CE_GPIO;
	//__pwr = NRF905_PIN_UNUSED;
	__pwr =  CONFIG_PWR_GPIO;
	__cd = NRF905_PIN_UNUSED;
	__dr = NRF905_PIN_UNUSED;
	__am = NRF905_PIN_UNUSED;

	// SPI stuff
	ESP_LOGI(TAG, "CONFIG_CSN_GPIO=%d", CONFIG_CSN_GPIO);
	ESP_LOGI(TAG, "CONFIG_MISO_GPIO=%d", CONFIG_MISO_GPIO);
	ESP_LOGI(TAG, "CONFIG_MOSI_GPIO=%d", CONFIG_MOSI_GPIO);
	ESP_LOGI(TAG, "CONFIG_SCK_GPIO=%d", CONFIG_SCK_GPIO);
	__csn = CONFIG_CSN_GPIO;

	// SPI Chip Select
	gpio_reset_pin(__csn);
	gpio_set_direction(__csn, GPIO_MODE_OUTPUT);
	gpio_set_level(__csn, 1);

	// TX_EN=”1”TX mode, TX_EN=”0”RX mode
	gpio_reset_pin(__txen);
	gpio_set_direction(__txen, GPIO_MODE_OUTPUT);

	gpio_reset_pin(__ce);
	gpio_set_direction(__ce, GPIO_MODE_OUTPUT);

	gpio_reset_pin(__pwr);
	gpio_set_direction(__pwr, GPIO_MODE_OUTPUT);

	spi_bus_config_t bus = {
		.miso_io_num = CONFIG_MISO_GPIO,
		.mosi_io_num = CONFIG_MOSI_GPIO,
		.sclk_io_num = CONFIG_SCK_GPIO,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
		.max_transfer_sz = 0
	};
					 
	ret = spi_bus_initialize(HOST_ID, &bus, SPI_DMA_CH_AUTO);
	assert(ret == ESP_OK);

	spi_device_interface_config_t dev = {
		.clock_speed_hz = 9000000,
		.mode = 0,
		.queue_size = 7,
		.flags = 0,
		.pre_cb = NULL
	};
	ret = spi_bus_add_device(HOST_ID, &dev, &__spi);
	assert(ret == ESP_OK);

	nRF905_powerOn(false);
	nRF905_standbyMode(true);
	nRF905_txMode(false);
	delay(3);
	nRF905_defaultConfig();
	__polledMode = true;
}

void nRF905_setChannel(uint16_t channel)
{
	if(channel > 511)
		channel = 511;

	// TODO atomic read/write?
	uint8_t reg = (nRF905_readConfigRegister(NRF905_REG_CONFIG1) & NRF905_MASK_CHANNEL) | (channel>>8);

	gpio_set_level(__csn, 0);
	spi_transfer(NRF905_CMD_W_CONFIG | NRF905_REG_CHANNEL);
	spi_transfer(channel);
	spi_transfer(reg);
	gpio_set_level(__csn, 1);
}

void nRF905_setBand(nRF905_band_t band)
{
	// TODO atomic read/write?
	uint8_t reg = (nRF905_readConfigRegister(NRF905_REG_CONFIG1) & NRF905_MASK_BAND) | band;

	gpio_set_level(__csn, 0);
	spi_transfer(NRF905_CMD_W_CONFIG | NRF905_REG_CONFIG1);
	spi_transfer(reg);
	gpio_set_level(__csn, 1);
}

void nRF905_setAutoRetransmit(bool val)
{
	nRF905_setConfigReg1(
		val ? NRF905_AUTO_RETRAN_ENABLE : NRF905_AUTO_RETRAN_DISABLE,
		NRF905_MASK_AUTO_RETRAN,
		NRF905_REG_AUTO_RETRAN
	);
}

void nRF905_setLowRxPower(bool val)
{
	nRF905_setConfigReg1(
		val ? NRF905_LOW_RX_ENABLE : NRF905_LOW_RX_DISABLE,
		NRF905_MASK_LOW_RX,
		NRF905_REG_LOW_RX
	);
}

void nRF905_setTransmitPower(nRF905_pwr_t val)
{
	nRF905_setConfigReg1(val, NRF905_MASK_PWR, NRF905_REG_PWR);
}

void nRF905_setCRC(nRF905_crc_t val)
{
	nRF905_setConfigReg2(val, NRF905_MASK_CRC, NRF905_REG_CRC);
}

void nRF905_setClockOut(nRF905_outclk_t val)
{
	nRF905_setConfigReg2(val, NRF905_MASK_OUTCLK, NRF905_REG_OUTCLK);
}

void nRF905_setPayloadSize(uint8_t sizeTX, uint8_t sizeRX)
{
	if(sizeTX > NRF905_MAX_PAYLOAD)
		sizeTX = NRF905_MAX_PAYLOAD;

	if(sizeRX > NRF905_MAX_PAYLOAD)
		sizeRX = NRF905_MAX_PAYLOAD;

	gpio_set_level(__csn, 0);
	spi_transfer(NRF905_CMD_W_CONFIG | NRF905_REG_RX_PAYLOAD_SIZE);
	spi_transfer(sizeRX);
	spi_transfer(sizeTX);
	gpio_set_level(__csn, 1);
}

void nRF905_setAddressSize(uint8_t sizeTX, uint8_t sizeRX)
{
	if(sizeTX != 1 && sizeTX != 4)
		sizeTX = 4;

	if(sizeRX != 1 && sizeRX != 4)
		sizeRX = 4;

	gpio_set_level(__csn, 0);
	spi_transfer(NRF905_CMD_W_CONFIG | NRF905_REG_ADDR_WIDTH);
	spi_transfer((sizeTX<<4) | sizeRX);
	gpio_set_level(__csn, 1);
}

bool nRF905_receiveBusy()
{
	return nRF905_addressMatched();
}

bool nRF905_airwayBusy()
{
	if(__cd != NRF905_PIN_UNUSED)
		return gpio_get_level(__cd);
	return false;
}

void nRF905_setListenAddress(uint32_t address)
{
	nRF905_setAddress(address, NRF905_CMD_W_CONFIG | NRF905_REG_RX_ADDRESS);
}

void nRF905_write(uint32_t sendTo, void* data, uint8_t len)
{
	nRF905_setAddress(sendTo, NRF905_CMD_W_TX_ADDRESS);

	if(len > 0 && data != NULL)
	{
		if(len > NRF905_MAX_PAYLOAD)
			len = NRF905_MAX_PAYLOAD;

		gpio_set_level(__csn, 0);
		spi_transfer(NRF905_CMD_W_TX_PAYLOAD);
		//spi_transfer(data, len);
		for(uint8_t i=0;i<len;i++)
			spi_transfer(((uint8_t*)data)[i]);
		gpio_set_level(__csn, 1);
	}
}

void nRF905_read(void* data, uint8_t len)
{
	if(len > NRF905_MAX_PAYLOAD)
		len = NRF905_MAX_PAYLOAD;

	gpio_set_level(__csn, 0);
	spi_transfer(NRF905_CMD_R_RX_PAYLOAD);

	// Get received payload
	//spi_transfer(data, len);
	for(uint8_t i=0;i<len;i++)
		((uint8_t*)data)[i] = spi_transfer(NRF905_CMD_NOP);

	// Must make sure all of the payload has been read, otherwise DR never goes low
	//uint8_t remaining = NRF905_MAX_PAYLOAD - len;
	//while(remaining--)
	//	spi_transfer(NRF905_CMD_NOP);
	gpio_set_level(__csn, 1);
}

/*
uint32_t nRF905_readUInt32() // TODO
{
	uint32_t data;
	CHIPSELECT()
	{
		spi.transfer(NRF905_CMD_R_RX_PAYLOAD);

		// Get received payload
		for(uint8_t i=0;i<4;i++)
			((uint8_t*)&data)[i] = spi.transfer(NRF905_CMD_NOP);
	}
	return data;
}

uint8_t nRF905_readUInt8() // TODO
{
	uint8_t data;
	CHIPSELECT()
	{
		spi.transfer(NRF905_CMD_R_RX_PAYLOAD);

		// Get received payload
		data = spi.transfer(NRF905_CMD_NOP);
	}
	return data;
}

size_t nRF905_write(uint8_t data) // TODO
{
	return 1;
}

size_t nRF905_write(const uint8_t *data, size_t quantity) // TODO
{
	write(0xE7E7E7E7, data, quantity);
	return quantity;
}

int nRF905_available() // TODO
{
	return 1;
}

int nRF905_read() // TODO
{
	return 1;
}

int nRF905_peek() // TODO
{
	return 1;
}

void nRF905_flush() // TODO
{
	// XXX: to be implemented.
}
*/

bool nRF905_TX(nRF905_nextmode_t nextMode, bool collisionAvoid)
{
	// TODO check DR is low?
	// check AM for incoming packet?
	// what if already in TX mode? (auto-retransmit or carrier wave)

	nRF905_mode_t currentMode = nRF905_mode();
	if(currentMode == NRF905_MODE_POWERDOWN)
	{
		currentMode = NRF905_MODE_STANDBY;
		nRF905_standbyMode(true);
		nRF905_powerOn(true);
		if(nextMode != NRF905_NEXTMODE_TX)
			delay(3); // Delay is needed to the radio has time to power-up and see the standby/TX pins pulse
	}
	else if(collisionAvoid && nRF905_airwayBusy())
		return false;

	// Put into transmit mode
	nRF905_txMode(true); //PORTB |= _BV(PORTB1);

	// Pulse standby pin to start transmission
	if(currentMode == NRF905_MODE_STANDBY)
		nRF905_standbyMode(false); //PORTD |= _BV(PORTD7);
	
	// NOTE: If nextMode is RX or STANDBY and a long running interrupt happens during the delays below then
	// we may end up transmitting a blank carrier wave until the interrupt ends.
	// If nextMode is RX then an unexpected onTxComplete event will also fire and RX mode won't be entered until the interrupt finishes.

	if(nextMode == NRF905_NEXTMODE_RX)
	{
		// 1.	The datasheets says that the radio can switch straight to RX mode after
		//	a transmission is complete by clearing TX_EN while transmitting, but if the radio was
		//	in standby mode and TX_EN is cleared within ~700us then the transmission seems to get corrupt.
		// 2.	Going straight to RX also stops DR from pulsing after transmission complete which means the onTxComplete event doesn't work
		if(currentMode == NRF905_MODE_STANDBY)
		{
			// Use micros() timing here instead of delayMicroseconds() to get better accuracy incase other interrupts happen (which would cause delayMicroseconds() to pause)
			//unsigned int start = micros();
			//while((unsigned int)(micros() - start) < 700);
			delayMicroseconds(700);
		}
		else
			delayMicroseconds(14);
		// finish transmit
		nRF905_txMode(false); //PORTB &= ~_BV(PORTB1);
	}
	else if(nextMode == NRF905_NEXTMODE_STANDBY)
	{
		delayMicroseconds(14);
		nRF905_standbyMode(true);
		//txMode(false);
	}
	else { //NRF905_NEXTMODE_TX
		delayMicroseconds(14);
		// finish transmit
		nRF905_txMode(false); //PORTB &= ~_BV(PORTB1);
	}

	return true;
}

void nRF905_RX(void)
{
	nRF905_txMode(false);
	nRF905_standbyMode(false);
	nRF905_powerOn(true);
}

void nRF905_powerDown(void)
{
	nRF905_powerOn(false);
}

void nRF905_standby(void)
{
	nRF905_standbyMode(true);
	nRF905_powerOn(true);
}

nRF905_mode_t nRF905_mode()
{
	if(__pwr != NRF905_PIN_UNUSED)
	{
		if(!gpio_get_level(__pwr))
			return NRF905_MODE_POWERDOWN;
	}
	
	if(__ce != NRF905_PIN_UNUSED)
	{
		if(!gpio_get_level(__ce))
			return NRF905_MODE_STANDBY;
	}

	if(__txen != NRF905_PIN_UNUSED)
	{
		if(gpio_get_level(__txen))
			return NRF905_MODE_TX;
		return NRF905_MODE_RX;
	}

	return NRF905_MODE_ACTIVE;
}

bool nRF905_inStandbyMode(void)
{
	if(__ce != NRF905_PIN_UNUSED)
		return !gpio_get_level(__ce);
	return false;
}

bool nRF905_poweredUp(void)
{
	if(__pwr != NRF905_PIN_UNUSED)
		return gpio_get_level(__pwr);
	return true;
}

void nRF905_getConfigRegisters(void* regs)
{
	gpio_set_level(__csn, 0);
	spi_transfer(NRF905_CMD_R_CONFIG);
	for(uint8_t i=0;i<NRF905_REGISTER_COUNT;i++)
		((uint8_t*)regs)[i] = spi_transfer(NRF905_CMD_NOP);
	gpio_set_level(__csn, 1);
}

#if 0
// No interrupts are used in this library.
void nRF905_interrupt_dr()
{
	// If DR && AM = RX new packet
	// If DR && !AM = TX finished
	
	if(nRF905_addressMatched())
	{
		__validPacket = 1;
		if(onRxComplete != NULL)
			onRxComplete(this);
	}
	else
	{
		if(onTxComplete != NULL)
			onTxComplete(this);
	}

}
#endif

#if 0
// No interrupts are used in this library.
void nRF905_interrupt_am()
{
	// If AM goes HIGH then LOW without DR going HIGH then we got a bad packet


	if(nRF905_addressMatched())
	{
		if(onAddrMatch != NULL)
			onAddrMatch(this);
	}
	else if(!__validPacket)
	{
		if(onRxInvalid != NULL)
			onRxInvalid(this);
	}
	__validPacket = 0;

}
#endif

uint8_t nRF905_poll()
{
	if(!__polledMode)
		return NRF905_NONE;

	static uint8_t lastState;
	static uint8_t addrMatch;

// TODO read pins if am / dr defined

	uint8_t state = nRF905_readStatus() & ((1<<NRF905_STATUS_DR)|(1<<NRF905_STATUS_AM));
	ESP_LOGD(TAG, "state=%x lastState=%x", state, lastState);

	if(state != lastState)
	{
		if(state == ((1<<NRF905_STATUS_DR)|(1<<NRF905_STATUS_AM)))
		{
			addrMatch = 0;
			return NRF905_RX_COMPLETE;
#if 0
			if(onRxComplete != NULL)
				onRxComplete(this);
#endif
		}
#if 0
		This event does not occur.
		else if(state == (1<<NRF905_STATUS_DR))
		{
			addrMatch = 0;
			return NRF905_TX_COMPLETE;
			if(onTxComplete != NULL)
				onTxComplete(this);
		}
#endif
		else if(state == (1<<NRF905_STATUS_AM))
		{
			addrMatch = 1;
			return NRF905_ADDR_MATCH;
#if 0
			if(onAddrMatch != NULL)
				onAddrMatch(this);
#endif
		}
		else if(state == 0 && addrMatch)
		{
			addrMatch = 0;
			return NRF905_RX_INVALID;
#if 0
			if(onRxInvalid != NULL)
				onRxInvalid(this);
#endif
		}
		
		lastState = state;
	}
	return NRF905_NONE;
}


