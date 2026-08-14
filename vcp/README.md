# VCP Example   
This is nRF905 and VCP(Virtual Com Port) gateway application.   

ESP-IDF supports VCP hosts.   
VCP hosts can communicate with VCP devices using the USB port.   
Representative VCP devices include Arduino Uno and Arduino Mega, which have a UART-USB conversion chip.   
I based it on [this](https://github.com/espressif/esp-idf/tree/master/examples/peripherals/usb/host/cdc).   

This project uses the following components.   
Other UART-USB converter chips are not supported.   
- https://components.espressif.com/components/espressif/usb_host_ch34x_vcp   
- https://components.espressif.com/components/espressif/usb_host_cp210x_vcp   
- https://components.espressif.com/components/espressif/usb_host_ftdi_vcp   


```
            +------------+          +------------+          +------------+
            | VCP device |          |            |          |            |           |
            |Arduino Uno |--(USB)-->|    ESP32   |--(SPI)-->|   nRF905   |==(Radio)==>
            |Arduino Mega|          |            |          |            |
            +------------+          +------------+          +------------+

            +------------+          +------------+          +------------+
            |            |          |            |          | VCP device |
==(Radio)==>|   nRF905   |--(SPI)-->|    ESP32   |--(USB)-->|Arduino Uno |
            |            |          |            |          |Arduino Mega|
            +------------+          +------------+          +------------+
```


# Hardtware requirements

## ESP32-S2/S3
This project only works with ESP32S2/S3.   
The ESP32S2/S3 has USB capabilities.   

## USB Type-A Femail connector
USB connectors are available from AliExpress or eBay.   
I used it by incorporating it into a Universal PCB.   
![USBConnector](https://github.com/user-attachments/assets/8d7d8f0a-d289-44b8-ae90-c693a1099ca0)

We can buy this breakout on Ebay or AliExpress.   
![usb-conector-11](https://github.com/user-attachments/assets/848998d4-fb0c-4b4f-97ae-0b3ae8b8996a)
![usb-conector-12](https://github.com/user-attachments/assets/6fc34dcf-0b13-4233-8c71-07234e8c6d06)


# Configuration
<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/ac40055d-e8b5-46de-bac3-a800ef3b8fd1" />
<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/0a874ecc-413c-4cd0-8bef-35179d54e195" />

## Radio Setting

### VCP to Radio
Receive from VCP device and send to Radio.   
ESP32 acts as the VCP host.   
```
            +------------+          +------------+          +------------+
            | VCP device |          |            |          |            |           |
            |Arduino Uno |--(USB)-->|    ESP32   |--(SPI)-->|   nRF905   |==(Radio)==>
            |Arduino Mega|          |            |          |            |
            +------------+          +------------+          +------------+
```

<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/da9fe038-7c52-4893-bdcd-88f44909dfdd" />


### Radio to VCP
Receive from Radio and send to VCP device.   
ESP32 acts as the VCP host.   
```
            +------------+          +------------+          +------------+
            |            |          |            |          | VCP device |
==(Radio)==>|   nRF905   |--(SPI)-->|    ESP32   |--(USB)-->|Arduino Uno |
            |            |          |            |          |Arduino Mega|
            +------------+          +------------+          +------------+
```

<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/74be7ef4-e534-46bf-a1f6-02e3b2c0cfa5" />


## VCP Setting

<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/7b940997-12d4-4e46-9bb2-a1aba489081b" />


# Write this sketch on Arduino Uno.   
You can use any AtMega microcontroller that has a USB port.   

```
const int LED_PIN = 13;
unsigned long lastMillis = 0;

void setup() {
  Serial.begin(115200);
  pinMode( LED_PIN, OUTPUT );
}

void loop() {
  while (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    digitalWrite( LED_PIN, !digitalRead(LED_PIN) );
  }

  if(lastMillis + 1000 <= millis()){
    Serial.print("Hello World ");
    Serial.println(millis());
    lastMillis += 1000;
  }

  delay(1);
}
```

Strings from Arduino to ESP32 are terminated with CR(0x0d)+LF(0x0a).   
This project will remove the termination character and send to Radio.   
```
I (6020) VCP: Receiving data through CdcAcmDevice
I (6020) VCP: 0x3fca1c04   48 65 6c 6c 6f 20 57 6f  72 6c 64 20 31 35 30 30  |Hello World 1500|
I (6020) VCP: 0x3fca1c14   30 0d 0a                                          |0..|
I (6030) TX: xMessageBufferReceive received=19
I (6040) TX: 0x3fc9e230   48 65 6c 6c 6f 20 57 6f  72 6c 64 20 31 35 30 30  |Hello World 1500|
I (6040) TX: 0x3fc9e240   30
```

The Arduino sketch inputs data with LF as the terminator.   
So strings from the ESP32 to the Arduino must be terminated with LF (0x0a).   
If the string output from the ESP32 to the Arduino is not terminated with LF (0x0a), the Arduino sketch will complete the input with a timeout.   
The default input timeout for Arduino sketches is 1000 milliseconds.   
This project will add a LF to the end and send to Arduino.   
The Arduino sketch will blink the on-board LED.   
```
I (3290) VCP: Sending data through CdcAcmDevice
I (72618) VCP: 0x3fc9f2e0   48 65 6c 6c 6f 20 57 6f  72 6c 64 20 36 30 31 38  |Hello World 6018|
I (72628) VCP: 0x3fc9f2f0   38 34                                             |84|
I (72628) VCP: 0x3fc9f2e0   48 65 6c 6c 6f 20 57 6f  72 6c 64 20 36 30 31 38  |Hello World 6018|
I (72638) VCP: 0x3fc9f2f0   38 34 0a
```

# Wireing   
Arduino Uno connects via USB connector.   
The USB port on the ESP32S2/S3 development board does not function as a USB-HOST.   

```
+---------+  +-------------+  +-----------+
|ESP BOARD|==|USB CONNECTOR|==|Arduino Uno|
+---------+  +-------------+  +-----------+
```

```
ESP BOARD          USB CONNECTOR (type A)
                         +--+
5V        -------------> | || VCC
[GPIO19]  -------------> | || D-
[GPIO20]  -------------> | || D+
GND       -------------> | || GND
                         +--+
```

![Image](https://github.com/user-attachments/assets/7bf405af-b1ec-4c7c-87d1-8bbe176e807b)

