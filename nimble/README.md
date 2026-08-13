# BLE Example   
This is nRF905 and BLE gateway application.   
ESP-IDF can use either the ESP-Bluedroid host stack or the ESP-NimBLE host stack.   
The differences between the two are detailed [here](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/ble/overview.html).   
This project uses the ESP-NimBLE host stack.   

```
            +------------+          +------------+          +------------+
            |            |          |            |          |            |
            | Smartphone |--(BLE)-->|   ESP32    |--(SPI)-->|   nrf905   |==(Radio)==>
            |            |          |            |          |            |
            +------------+          +------------+          +------------+

            +------------+          +------------+          +------------+
            |            |          |            |          |            |
==(Radio)==>|   nrf905   |--(SPI)-->|   ESP32    |--(BLE)-->| Smartphone |
            |            |          |            |          |            |
            +------------+          +------------+          +------------+
```



# Configuration
<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/bca0f782-e398-42c7-9f5e-4278ec30cfae" />

### BLE to Radio
Receive from BLE and send to Radio.   
```
            +------------+          +------------+          +------------+
            |            |          |            |          |            |
            | Smartphone |--(BLE)-->|   ESP32    |--(SPI)-->|   nrf905   |==(Radio)==>
            |            |          |            |          |            |
            +------------+          +------------+          +------------+
```

<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/bdf32720-f865-4066-a5a3-ca6bc870d343" />

Communicate with Arduino Environment.   
I tested it with [this](https://github.com/nopnop2002/esp-idf-nrf905/tree/main/ArduinoCode/receiver).   

### Radio to BLE
Receive from Radio and send to BLE.   
```
            +------------+          +------------+          +------------+
            |            |          |            |          |            |
==(Radio)==>|   nrf905   |--(SPI)-->|   ESP32    |--(BLE)-->| Smartphone |
            |            |          |            |          |            |
            +------------+          +------------+          +------------+
```

<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/69a5b69b-9a92-4892-9a82-9d28319a555b" />

Communicate with Arduino Environment.   
I tested it with [this](https://github.com/nopnop2002/esp-idf-nrf905/tree/main/ArduinoCode/transmitter).   

# Android Application   
I used [this](https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal) app.   

- pair with ESP_NIMBLE_SERVER   

- Launch the app and select device  
	Menu->Devices->Bluetooth LE   

- Long press the device and select the Edit menu   
	![Image](https://github.com/user-attachments/assets/2d36b757-585a-4310-919c-a57f136c7f20)

- Select Custom and specify UUID   
	The UUIDs are different for ESP-Bluedroid and ESP-NimBLE.   
	![Image](https://github.com/user-attachments/assets/9b0f23bc-86f4-4631-81e6-1df8d876f41b)

- Connect to device   
	You can send and receive Radio packets using an Android device.   
	![Image](https://github.com/user-attachments/assets/e84fa3b1-a0ee-4af3-a64c-695a5b383857)

# iOS Application   
[This](https://apps.apple.com/jp/app/bluetooth-v2-1-spp-setup/id6449416841) might work, but I don't have iOS so I don't know.   

# Concurrent connection
Unlike ESP-Bluedroid host stack, ESP-NimBLE host stack allows concurrent connections.   
The maximum number of simultaneous connections is specified here.   
![Image](https://github.com/user-attachments/assets/9d1e1182-ed41-4b9e-bc55-bb3c75dd4745)   

You can use multiple smartphones simultaneously.   
![Image](https://github.com/user-attachments/assets/4d84823a-69c4-48bf-9671-64644f048ccd)   
