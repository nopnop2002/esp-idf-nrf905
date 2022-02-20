# Example of Arduino environment   
I use [this](https://github.com/ZakKemble/nRF905-arduino).   

# Wirering using polling

|nRF905||UNO||
|:-:|:-:|:-:|:-:|
|CSN|--|D6|*1|
|MISO|--|D11||
|MOSI|--|D12|*1|
|SCK|--|D13|*1|
|TXEN|--|D9|*1|
|DR|--|N/C||
|CD|--|N/C||
|PWR|--|3.3V||
|AM|--|N/C||
|CLK|--|N/C||
|CE|--|3.3V||
|VCC|--|3.3V||
|GND|--|GND||

(*1)    
nRF905 is not 5V tolerant.   
You need level shift from 5V to 3.3V.   

# Wirering using DR and AM Interrupt

|nRF905||UNO||
|:-:|:-:|:-:|:-:|
|CSN|--|D6|*1|
|MISO|--|D11||
|MOSI|--|D12|*1|
|SCK|--|D13|*1|
|TXEN|--|D9|*1|
|DR|--|D3||
|CD|--|D2||
|PWR|--|3.3V||
|AM|--|N/C||
|CLK|--|N/C||
|CE|--|3.3V||
|VCC|--|3.3V||
|GND|--|GND||

(*1)    
nRF905 is not 5V tolerant.   
You need level shift from 5V to 3.3V.   
