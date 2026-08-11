# USB Serial Device Example   
This is nRF905 and USB Serial Device gateway application.   

ESP-IDF supports USB Serial Device.   
USB Serial Device can communicate with the USB Serial Host.   
Windows or Linux terminal software acts as a USB Serial Host.   
This project use [this](https://docs.tinyusb.org/en/latest/) TinyUSB component as the USB stack.   
```
            +---------------+          +---------------+          +---------------+
            |               |          |               |          |               |
            |USB Serial Host|--(USB)-->|     ESP32     |--(SPI)-->|     nRF905    |==(Radio)==>
            |               |          |               |          |               |
            +---------------+          +---------------+          +---------------+

            +---------------+          +---------------+          +---------------+
            |               |          |               |          |               |
==(Radio)==>|     nRF905    |--(SPI)-->|     ESP32     |--(USB)-->|USB Serial Host|
            |               |          |               |          |               |
            +---------------+          +---------------+          +---------------+
```

# Hardware requirements
1. ESP32-S2/S3/P4 Development board   
Because the ESP32-S2/S3/P4 does support USB OTG.   

2. USB Connector   
I used this USB Mini femail:   
![usb-connector](https://user-images.githubusercontent.com/6020549/124848149-3714ba00-dfd7-11eb-8344-8b120790c5c5.JPG)

```
ESP32-S2/S3 BOARD          USB CONNECTOR
                           +--+
                           | || VCC
    [GPIO 19]    --------> | || D-
    [GPIO 20]    --------> | || D+
    [  GND  ]    --------> | || GND
                           +--+

ESP32-P4 BOARD             USB CONNECTOR
                           +--+
                           | || VCC
    [GPIO 26]    --------> | || D-
    [GPIO 27]    --------> | || D+
    [  GND  ]    --------> | || GND
                           +--+

```


# Configuration
<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/75ec3a26-2c55-49b2-a8f6-156ebd58d6a8" />

### USB to Radio
Receive from USB and send to Radio.   
ESP32 acts as USB Serial Device for reading.   
You can use this script as USB Serial Host for writing.   
```python3 ./write.py```

```
            +---------------+          +---------------+          +---------------+
            |               |          |               |          |               |
            |USB Serial Host|--(USB)-->|     ESP32     |--(SPI)-->|     nRF905    |==(Radio)==>
            |               |          |               |          |               |
            +---------------+          +---------------+          +---------------+
```

<img width="659" height="468" alt="Image" src="https://github.com/user-attachments/assets/d0f346c5-ca95-43e3-98fc-e37707353f83" />

Communicate with Arduino Environment.   
I tested it with [this](https://github.com/nopnop2002/esp-idf-nrf905/tree/main/ArduinoCode/receiver).   

### Radio to USB
Receive from Radio and send to USB.   
ESP32 acts as USB Serial Device for writing.   
You can use this script as USB Serial Host for reading.   
```python3 ./read.py```

```
            +---------------+          +---------------+          +---------------+
            |               |          |               |          |               |
==(Radio)==>|     nRF905    |--(SPI)-->|     ESP32     |--(USB)-->|USB Serial Host|
            |               |          |               |          |               |
            +---------------+          +---------------+          +---------------+
```

<img width="659" height="468" alt="Image" src="https://github.com/user-attachments/assets/3b5e448c-605d-4e86-811c-d8576680f6a3" />

Communicate with Arduino Environment.   
I tested it with [this](https://github.com/nopnop2002/esp-idf-nrf905/tree/main/ArduinoCode/transmitter).   

# Using Windows Terminal Software
When you connect the USB cable to the USB port on your Windows machine and build the firmware, a new COM port will appear.   
Open a new COM port in the terminal software.   
When set to Radio to USB, the data received from Radio will be displayed on the terminal.   
When set to USB to Radio, input data from the keyboard is sent to Radio.   
I used TeraTerm as terminal software.   
![tusb-TeraTerm](https://github.com/user-attachments/assets/b5eea94e-5228-45b4-bcad-81cce8c52479)

# Using Linux Terminal Software
When you connect the USB cable to the USB port on your Linux machine and build the firmware, a new /dev/tty device will appear.   
Open a new tty device in the terminal software.   
Most occasions, the device is /dev/ttyACM0.   
When set to Radio to USB, the data received from Radio will be displayed on the terminal.   
When set to USB to Radio, input data from the keyboard is sent to Radio.   
I used screen as terminal software.   
![tusb-screen](https://github.com/user-attachments/assets/18a6e519-9250-4109-b05d-6bcd418bfb5b)

