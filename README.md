# esp-idf-nrf905
nRF905 Single chip 433/868/915MHz Transceiver Driver for esp-idf.   

I ported from [here](https://github.com/ZakKemble/nRF905-arduino).   

This is a 433MHz module.   
I have never seen an 868MHz or 915Mhz module.   
![nRF905-PHY](https://user-images.githubusercontent.com/6020549/154829949-f2bd4d74-f287-42ae-9f1c-29da57b3d580.JPG)

# Software requirements
ESP-IDF V4.4/V5.x.   
ESP-IDF V5.0 is required when using ESP32-C2.   
ESP-IDF V5.1 is required when using ESP32-C6.   


# Installation
```Shell
git clone https://github.com/nopnop2002/esp-idf-nrf905
cd esp-idf-nrf905/basic/
idf.py set-target {esp32/esp32s2/esp32s3/esp32c2/esp32c3/esp32c6}
idf.py menuconfig
idf.py flash
```

__Note for ESP32C3__   
For some reason, there are development boards that cannot use GPIO06, GPIO08, GPIO09, GPIO19 for SPI clock pins.   
According to the ESP32C3 specifications, these pins can also be used as SPI clocks.   
I used a raw ESP-C3-13 to verify that these pins could be used as SPI clocks.   

# Configuration for Transceiver   
![config-nrf905-1](https://user-images.githubusercontent.com/6020549/154828765-9a272831-a2fa-433f-8774-3ce265eecbc5.jpg)
![Image](https://github.com/user-attachments/assets/4c80312f-4139-41fb-8a6b-b4a4df45ea16)

# SPI BUS selection   
![Image](https://github.com/user-attachments/assets/12e2e650-a1c3-4056-b9c9-8901bf663065)

The ESP32 series has three SPI BUSs.   
SPI1_HOST is used for communication with Flash memory.   
You can use SPI2_HOST and SPI3_HOST freely.   
When you use SDSPI(SD Card via SPI), SDSPI uses SPI2_HOST BUS.   
When using this module at the same time as SDSPI or other SPI device using SPI2_HOST, it needs to be changed to SPI3_HOST.   
When you don't use SDSPI, both SPI2_HOST and SPI3_HOST will work.   
Previously it was called HSPI_HOST / VSPI_HOST, but now it is called SPI2_HOST / SPI3_HOST.   

# Channel to use
![Image](https://github.com/user-attachments/assets/985173a2-947a-4006-93f6-62f65ef8de4c)

The channel used by the sender and receiver must match.   
Channel spacing for 433MHz band is 100KHz.   

# Wirering

|nRF905||ESP32|ESP32-S2/S3|ESP32-C2/C3/C6|
|:-:|:-:|:-:|:-:|:-:|
|MISO|--|GPIO19|GPIO37|GPIO4|
|SCK|--|GPIO18|GPIO36|GPIO3|
|MOSI|--|GPIO23|GPIO35|GPIO2|
|CSN|--|GPIO5|GPIO34|GPIO1|
|TXEN|--|GPIO15|GPIO40|GPIO5|
|PWR|--|GPIO16|GPIO41|GPIO6|
|CE|--|GPIO17|GPIO42|GPIO0|
|DR|--|N/C|N/C|N/C|
|CD|--|N/C|N/C|N/C|
|AM|--|N/C|N/C|N/C|
|CLK|--|N/C|N/C|N/C|
|VCC|--|3.3V|3.3V|3.3V|
|GND|--|GND|GND|GND|

__You can change it to any pin using menuconfig.__   

# Default Settings
```
================ NRF Configuration ================
CH_NO            = 10
FRF              = 423.40 MHz
AUTO_RETRAN      = No retransmission
RX_RED_PWR       = Normal operation
PA_PWR           = +10dBm
HFREQ_PLL        = Chip operating in 433MHz band
TX_AFW           = 4 byte TX address field width
RX_AFW           = 4 byte RX address field width
TX_PW            = 32 byte TX payload field width
RX_PW            = 32 byte RX payload field width
RX_ADDRESS       = 0xe7e7e7e7
CRC_MODE         = 16 CRC check bit
CRC_EN           = Enable
XOF              = 16MHz
UP_CLK_EN        = No external clock signal available
UP_CLK_FREQ      = 4MHz
```

# Comparison of nRF905 and nRF24L01
||nRF905|nRF24L01|
|:-:|:-:|:-:|
|Manufacturer|Nordic Semiconductor|Nordic Semiconductor|
|Frequency|433/868/915MHz|2.4GHz|
|Maximum Payload|32Byte|32Byte|
|FiFo for reception|32Byte*1|32Byte*3|
|FiFo for transmission|32Byte*1|32Byte*3|
|CRC Length|8/16bits|8/16bits|
|Acknowledgement Payload|No|Yes|
|Available Modulation format|GFSK|GFSK|

nRF905 cannot tell whether the transmission was successful or unsuccessful on the transmitting side.   
nRF24L01 can tell whether the transmission was successful or unsuccessful on the transmitting side.   

# Comparison of nRF905, cc1101 and si4432
||nRF905|cc1101|si4432|
|:-:|:-:|:-:|:-:|
|Manufacturer|Nordic Semiconductor|Texas Instrument|Silicon Labs|
|Frequency|433/868/915MHz|315/433/868/915MHz|315/433/868/915MHz|
|Maximum Payload|32Byte|64Byte|64Byte|
|FiFo for reception|32Byte*1|64Byte*1|64Byte*1|
|FiFo for transmission|32Byte*1|64Byte*1|64Byte*1|
|CRC Length|8/16bits|16bits|8/16bits|
|Acknowledgement Payload|No|No|No|
|Available Modulation format|GFSK|2-FSK/4-FSK/GFSK/ASK/OOK/MSK|FSK/GFSK/OOK|

- GFSK  : Gaussian shaped Frequency Shift Keying
- 2-FSK : Binary Frequency Shift Keying
- 4-FSK : Quaternary Frequency Shift Keying
- ASK   : Amplitude Shift Keying
- OSK   : On-Off Keying
- MSI   : Minimum Shift Keying

