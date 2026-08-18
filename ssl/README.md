# SSL Example   
This is nRF905 and SSL gateway application.   
Receive from nRF905 and send to SSL Server.   
ESP32 acts as SSL Client.   

```
            +-----------+           +-----------+           +-----------+
            |           |           |           |           |           |
==(Radio)==>|  nRF905   |--(SPI)--->|   ESP32   |--(SSL)--->| SSL Server|
            |           |           |           |           |           |
            +-----------+           +-----------+           +-----------+
```

# Installation
```
git clone https://github.com/nopnop2002/esp-idf-nrf905
cd esp-idf-nrf905/ssl/
chmod 777 mkkey.sh
./mkkey.sh
idf.py menuconfig
idf.py flash
```

mkkey.sh creates a server certificate file.   
The server certificate file is associated with the SSL server's IP address.   
mkkey.sh automatically retrieves the IP address of the server on which the script is executed and treats that address as an SSL server.   
To manually configure the SSL server's IP address, modify the script as follows:   
```
IP="192.168.0.123"
openssl req -x509 -new -nodes -key server.key -subj "/CN=${IP}" -days 10000 -out server.crt
```


# Configuration
<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/1c40cfcf-cd0c-49a6-a4e3-6f7fc2aa8377" />
<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/9e950e4e-4fd4-4b00-99bc-a4817d92747b" />

## WiFi Setting
Set the information of your access point.   
<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/2daa7920-71f3-473e-8c74-2a036dcd096c" />

## Server Setting
Set the information of your SSL server.   
<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/5a7e6ae0-eac2-461e-b1be-41f109b3af5a" />

# Start the SSL server
- C language
	```
	cd clang-tls-communication
	make
	./server
	```
	<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/8e232db4-3833-4eec-a294-691f72c806bc" />

- python script
	```
	cd python-tls-communication
	python3 server.py
	```
	<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/67dff2d2-e803-464f-9b4b-5f5d8382157d" />

