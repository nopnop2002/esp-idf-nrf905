# HTTP Example   
This is nRF905 and HTTP gateway application.   
```
            +-----------+           +-----------+           +-----------+
            |           |           |           |           |           |
            |HTTP Client|--(HTTP)-->|   ESP32   |--(SPI)--->|  nRF905   |==(Radio)==>
            |           |           |           |           |           |
            +-----------+           +-----------+           +-----------+

            +-----------+           +-----------+           +-----------+
            |           |           |           |           |           |
==(Radio)==>|  nRF905   |--(SPI)--->|   ESP32   |--(HTTP)-->|HTTP Server|
            |           |           |           |           |           |
            +-----------+           +-----------+           +-----------+
```



# Configuration
![config-top](https://github.com/nopnop2002/esp-idf-nrf905/assets/6020549/8ff295e5-7a9f-49b6-b34f-df038d767d35)
![config-app](https://github.com/nopnop2002/esp-idf-nrf905/assets/6020549/e87d059b-7e90-4235-9e45-96541cf0582f)

## WiFi Setting
Set the information of your access point.   
![config-wifi](https://github.com/nopnop2002/esp-idf-nrf905/assets/6020549/630c0aa8-463a-4b68-9a83-34025236585c)

## Radioi Setting
Set the wireless communication direction.   

### HTTP to Radio
Subscribe with HTTP and send to Radio.   
ESP32 acts as HTTP Server.   
You can use curl as HTTP Client.   
```sh ./http-client.sh```

```
            +-----------+           +-----------+           +-----------+
            |           |           |           |           |           |
            |HTTP Client|--(HTTP)-->|   ESP32   |--(SPI)--->|  nRF905   |==(Radio)==>
            |           |           |           |           |           |
            +-----------+           +-----------+           +-----------+
```

![config-radio-1](https://github.com/nopnop2002/esp-idf-nrf905/assets/6020549/50c4f2b0-2931-47f1-b0fa-653388972d3c)

Communicate with Arduino Environment.   
I tested it with [this](https://github.com/nopnop2002/esp-idf-nrf905/tree/main/ArduinoCode/receiver).   

### Radio to HTTP
Receive from Radio and publish as HTTP.   
ESP32 acts as HTTP Client.   
You can use nc(netcat) as HTTP Server.   
```sh ./http-server.sh```

```
            +-----------+           +-----------+           +-----------+
            |           |           |           |           |           |
==(Radio)==>|  nRF905   |--(SPI)--->|   ESP32   |--(HTTP)-->|HTTP Server|
            |           |           |           |           |           |
            +-----------+           +-----------+           +-----------+
```

![config-radio-2](https://github.com/nopnop2002/esp-idf-nrf905/assets/6020549/ac04ed51-4c17-42fb-a2a1-2771eaaeef76)

Communicate with Arduino Environment.   
I tested it with [this](https://github.com/nopnop2002/esp-idf-nrf905/tree/main/ArduinoCode/transmitter).   

### Specifying an HTTP Server   
You can specify your HTTP Server in one of the following ways:   
- IP address   
 ```192.168.10.20```   
- mDNS host name   
 ```http-server.local```   
- Fully Qualified Domain Name   
 ```http-server.public.io```


# Communication with arduino environment
I use [this](https://github.com/ZakKemble/nRF905-arduino).   
Example is in the arduino folder.   
