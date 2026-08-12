# Web Form Example   
Data entered through the web form is sent to Radio.   
Data received from Radio is displayed in a web form.   
```
            +-----------+              +-----------+              +-----------+
            |           |              |           |              |           |
            |  WebForm  |-(WebSocket)->|   ESP32   |----(SPI)---->|  nRF905   |==(Radio)==>
            |           |              |           |              |           |
            +-----------+              +-----------+              +-----------+

            +-----------+              +-----------+              +-----------+
            |           |              |           |              |           |
==(Radio)==>|  nRF905   |----(SPI)---->|   ESP32   |-(WebSocket)->|  WebForm  |
            |           |              |           |              |           |
            +-----------+              +-----------+              +-----------+
```

I used [this](https://github.com/Molorius/esp32-websocket) component.   
This component can communicate directly with the browser.   
It's a great job.   

# Configuration
<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/0d11b536-c267-4943-b35d-2ba5ae1680c7" />
<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/788d7a62-dde9-4c6c-8252-cf5e29d3f3d5" />

## WiFi Setting
Set the information of your access point.   
<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/f0a79f94-e9fe-4c0b-b2bd-697f08e1bce9" />

## Radio Setting
Set the wireless communication direction.   

### Web to Radio
Data entered through the web form is sent to Radio.   
```
            +-----------+              +-----------+              +-----------+
            |           |              |           |              |           |
            |  WebForm  |-(WebSocket)->|   ESP32   |----(SPI)---->|  nRF905   |==(Radio)==>
            |           |              |           |              |           |
            +-----------+              +-----------+              +-----------+
```

<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/d1a5bd5b-0054-496a-80e5-ae1bc806b915" />


### Radio to Web
Data received from Radio is displayed in a web form.   

```
            +-----------+              +-----------+              +-----------+
            |           |              |           |              |           |
==(Radio)==>|  nRF905   |----(SPI)---->|   ESP32   |-(WebSocket)->|  WebForm  |
            |           |              |           |              |           |
            +-----------+              +-----------+              +-----------+
```

<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/807a733b-6690-410e-8ae4-5c95614d2a52" />


# Launch a web browser
Enter the following in the address bar of your web browser.   
```
http:://{IP of ESP32}/
or
http://esp32-server.local/
```

<img width="819" height="878" alt="Image" src="https://github.com/user-attachments/assets/4f197141-bca8-4c98-9674-8c2fe6713dff" />
<img width="819" height="878" alt="Image" src="https://github.com/user-attachments/assets/0836364f-7d2e-4448-a0a3-6639338bc1db" />

### Web to Radio
Enter the data to send in the TextBox and press the Send button.   
<img width="819" height="878" alt="Image" src="https://github.com/user-attachments/assets/b7bc09ab-c482-4b98-aae4-6fef0995297f" />

### Radio to Web
The received data will be displayed in the TextBox.   
The Change button changes the number of lines displayed.   
The Copy button copies the received data to the clipboard.   
<img width="819" height="878" alt="Image" src="https://github.com/user-attachments/assets/f47dbe3f-c43f-47e1-8ecb-fe182ba0725e" />
<img width="819" height="878" alt="Image" src="https://github.com/user-attachments/assets/f68a5a62-0765-49be-97e1-aac4d91e563c" />

# WEB Pages
WEB Pages are stored in the html folder.   
I used [this](https://bulma.io/) open source css.   
You can change root.html as you like.   


