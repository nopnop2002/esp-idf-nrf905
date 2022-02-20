# Basic Example   
Simple send or receive.   

# Configuration   
![config-basic-1](https://user-images.githubusercontent.com/6020549/154828786-642e75d8-2042-4362-93f4-98363ebac9c1.jpg)
![config-basic-2](https://user-images.githubusercontent.com/6020549/154828787-1848834c-ea82-499a-9516-c1cdf00f487a.jpg)
![config-basic-3](https://user-images.githubusercontent.com/6020549/154828788-5032cfea-e316-4664-800d-04317b85c031.jpg)


# Communication with arduino environment
I use [this](https://github.com/ZakKemble/nRF905-arduino).   
Example is in the arduino folder.   

## Wirering using polling

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

## Wirering using DR and AM Interrupt

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
