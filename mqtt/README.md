# MQTT Example   
This is nRF905 and MQTT gateway application.   
```
            +----------+           +----------+           +----------+           +----------+
            |          |           |          |           |          |           |          |
            |Publisher |--(MQTT)-->|  Broker  |--(MQTT)-->|  ESP32   |--(SPI)--->|  nRF905  |==(Radio)==>
            |          |           |          |           |          |           |          |
            +----------+           +----------+           +----------+           +----------+

            +----------+           +----------+           +----------+           +----------+
            |          |           |          |           |          |           |          |
==(Radio)==>|  nRF905  |--(SPI)--->|  ESP32   |--(MQTT)-->|  Broker  |--(MQTT)-->|Subscriber|
            |          |           |          |           |          |           |          |
            +----------+           +----------+           +----------+           +----------+
```



# Configuration
![config-top](https://github.com/nopnop2002/esp-idf-nrf905/assets/6020549/c38dbfb0-3886-49d1-bf89-4d06074a843d)
![config-app](https://github.com/nopnop2002/esp-idf-nrf905/assets/6020549/8f43212b-8508-4d13-8392-e538a57f2f45)


## WiFi Setting
Set the information of your access point.   
![confog-wifi](https://github.com/nopnop2002/esp-idf-nrf905/assets/6020549/acad2f2d-1f1b-4d6f-860e-b32b8c010320)

## Radioi Setting
Set the wireless communication direction.   

### MQTT to Radio
Subscribe with MQTT and send to Radio.   
You can use mosquitto_pub as Publisher.   
```sh ./mqtt_pub.sh```

```
            +----------+           +----------+           +----------+           +----------+
            |          |           |          |           |          |           |          |
            |Publisher |--(MQTT)-->|  Broker  |--(MQTT)-->|  ESP32   |--(SPI)--->|  nRF905  |==(Radio)==>
            |          |           |          |           |          |           |          |
            +----------+           +----------+           +----------+           +----------+
```

![config-radio-1](https://github.com/nopnop2002/esp-idf-nrf905/assets/6020549/76201693-63a4-490b-9afd-9f7e2c16d456)



### Radio to MQTT
Receive from Radio and publish as MQTT.   
You can use mosquitto_sub as Subscriber.   
```sh ./mqtt_sub.sh```

```
            +----------+           +----------+           +----------+           +----------+
            |          |           |          |           |          |           |          |
==(Radio)==>|  nRF905  |--(SPI)--->|  ESP32   |--(MQTT)-->|  Broker  |--(MQTT)-->|Subscriber|
            |          |           |          |           |          |           |          |
            +----------+           +----------+           +----------+           +----------+
```

![config-radio-2](https://github.com/nopnop2002/esp-idf-nrf905/assets/6020549/024ea328-57e7-43e4-9217-75dd43c4733d)


### Specifying an MQTT Broker   
You can specify your MQTT broker in one of the following ways:   
- IP address   
 ```192.168.10.20```   
- mDNS host name   
 ```mqtt-broker.local```   
- Fully Qualified Domain Name   
 ```broker.emqx.io```

You can use this as broker.   
https://github.com/nopnop2002/esp-idf-mqtt-broker


# Communication with arduino environment
I use [this](https://github.com/ZakKemble/nRF905-arduino).   
Example is in the arduino folder.   


