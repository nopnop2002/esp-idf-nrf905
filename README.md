# esp-idf-nrf905
nRF905 Single chip 433/868/915MHz Transceiver Driver for esp-idf.   

I ported from [here](https://github.com/ZakKemble/nRF905-arduino).

![nRF905-PHY](https://user-images.githubusercontent.com/6020549/154829949-f2bd4d74-f287-42ae-9f1c-29da57b3d580.JPG)


# Software requirements
esp-idf v4.4 or later.   
This is because this version supports ESP32-C3.   


# Installation
```Shell
git clone https://github.com/nopnop2002/esp-idf-nrf905
cd esp-idf-nrf905/basic/
idf.py set-target {esp32/esp32s2/esp32c3}
idf.py menuconfig
idf.py flash
```

__Note for ESP32C3__   
For some reason, there are development boards that cannot use GPIO06, GPIO08, GPIO09, GPIO19 for SPI clock pins.   
According to the ESP32C3 specifications, these pins can also be used as SPI clocks.   
I used a raw ESP-C3-13 to verify that these pins could be used as SPI clocks.   

# Configuration for Transceiver   

![config-nrf905-1](https://user-images.githubusercontent.com/6020549/154828765-9a272831-a2fa-433f-8774-3ce265eecbc5.jpg)
![config-nrf905-2](https://user-images.githubusercontent.com/6020549/166896561-5fe9f860-f901-41b2-bbf7-01ff3acc0f0e.jpg)

# Wirering

|nRF905||ESP32|ESP32-S2|ESP32-C3|
|:-:|:-:|:-:|:-:|:-:|
|MISO|--|GPIO19|GPIO37|GPIO18|
|SCK|--|GPIO18|GPIO36|GPIO10|
|MOSI|--|GPIO23|GPIO35|GPIO19|
|CSN|--|GPIO5|GPIO34|GPIO9|
|TXEN|--|GPIO15|GPIO40|GPIO0|
|PWR|--|GPIO16|GPIO41|GPIO1|
|CE|--|GPIO17|GPIO42|GPIO2|
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
