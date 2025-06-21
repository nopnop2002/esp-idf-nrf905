# PingPong Example   
Send data from primary to secondary.   
In the secondary, the characters are converted and sent back.   

- ESP32 is Primary   
```
+-----------+           +-----------+             +-----------+           +-----------+
|           |           |           |             |           |           |           |
|  Primary  |===(SPI)==>|  nRF905   |---(Radio)-->|  nRF905   |===(SPI)==>| Secondary |
|   ESP32   |           |           |             |           |           |           |
|           |           |           |             |           |           |           |
|           |<==(SPI)===|           |<--(Radio)---|           |<==(SPI)===|           |
|           |           |           |             |           |           |           |
+-----------+           +-----------+             +-----------+           +-----------+
```

- ESP32 is Secondary   

```
+-----------+           +-----------+             +-----------+           +-----------+
|           |           |           |             |           |           |           |
|  Primary  |===(SPI)==>|  nRF905   |---(Radio)-->|  nRF905   |===(SPI)==>| Secondary |
|           |           |           |             |           |           |   ESP32   |
|           |           |           |             |           |           |           |
|           |<==(SPI)===|           |<--(Radio)---|           |<==(SPI)===|           |
|           |           |           |             |           |           |           |
+-----------+           +-----------+             +-----------+           +-----------+
```

# Configuration   
![Image](https://github.com/user-attachments/assets/701cec16-cbb9-4e85-967e-5c7041245266)
![Image](https://github.com/user-attachments/assets/d743b21c-75c5-4050-a1dd-4b2beea087ad)

# Communication with the Arduino environment   
- ESP32 is the primary   
I tested it with [this](https://github.com/nopnop2002/esp-idf-nrf905/tree/main/ArduinoCode/secondary).   

- ESP32 is the secondary   
I tested it with [this](https://github.com/nopnop2002/esp-idf-nrf905/tree/main/ArduinoCode/primary).   

