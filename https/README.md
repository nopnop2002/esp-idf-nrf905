# HTTPS Example   
This is nRF905 and HTTPS gateway application.   
Receive from nRF905 and send to HTTPS Server.   
ESP32 acts as HTTPS Client.   

```
            +-----------+           +-----------+            +------------+
            |           |           |           |            |            |
==(Radio)==>|  nRF905   |--(SPI)--->|   ESP32   |--(HTTPS)-->|HTTPS Server|
            |           |           |           |            |            |
            +-----------+           +-----------+            +------------+
```

# Installation
```
git clone https://github.com/nopnop2002/esp-idf-nrf905
cd esp-idf-nrf905/https/
chmod 777 mkkey.sh
./mkkey.sh
idf.py menuconfig
idf.py flash
```

mkkey.sh creates a server certificate file.   
The server certificate file is associated with the HTTPS server's IP address.   
mkkey.sh automatically retrieves the IP address of the server on which the script is executed and treats that address as an HTTPS server.   
To manually configure the HTTPS server's IP address, modify the script as follows:   
```
IP="192.168.0.123"
openssl req -x509 -new -nodes -key server.key -subj "/CN=${IP}" -days 10000 -out server.crt
```


# Configuration
<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/7541ebac-ba46-4ee5-aa81-157af2ead508" />
<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/fcb320c7-4921-4a76-bef3-c34318a5a2a1" />

## WiFi Setting
Set the information of your access point.   
<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/198be139-48bf-4643-b85c-316add820297" />

## Server Setting
Set the information of your HTTPS server.   
<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/1ee2a0de-a7ad-480f-8028-ae165b14f145" />

# Start the HTTPS server
```
python3 https-server.py
```
<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/48e78bdf-5b53-4729-8d5a-1a4a6d0a47f1" />
