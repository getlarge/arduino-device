# Aloes Device - Arduino

ESP8266 | ESP32 Arduino library to interact with [Aloes backend](https://framagit.org/aloes/device-manager) MQTT and HTTP API.


## Requirements

### Arduino tools

Arduino IDE - download the latest from arduino

- https://www.arduino.cc/en/Main/Software

Or Arduino CLI - download the latest from arduino

- https://github.com/arduino/arduino-cli


### For ESP8266 

Packages for ESP8266 on Arduino

- Enter `http://arduino.esp8266.com/stable/package_esp8266com_index.json` into Additional Board Manager URLs field.

Packages to upload data on ESP8266 FileSystem

- Follow instructions on `https://github.com/esp8266/arduino-esp8266fs-plugin` to install the plugin in your Arduino IDE.


### For ESP32 

Packages for ESP32 on Arduino

- Enter `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json` into Additional Board Manager URLs field.

Packages to upload data on ESP32 FileSystem

- Follow instructions on `https://github.com/me-no-dev/arduino-esp32fs-plugin` to install the plugin in your Arduino IDE.


### Dependencies

Requires the next libraries :  
- <Arduino.h>
- <FS.h>
- <advancedSerial.h>
- <ArduinoJson.h>
- <PubSubClient.h>
- <WiFiClientSecure.h>
- on ESP8266 :
	- <ESP8266WiFi.h>
	- <ESP8266HTTPClient.h>
	- <ESP8266httpUpdate.h>
- on ESP32 :
	- <WiFi.h>
	- <HTTPClient.h>
	- <Update.h>
	- <SPIFFS.h>

Starting from Arduino IDE 1.8.10, dependencies can be automatically installed.

Starting from arduino-cli 0.7.0, dependencies can be automatically installed with `arduino-cli lib install`

## Installation

- Via git, clone into your `~/Arduino/Libraries` folder
```bash 
git clone https://framagit.org/aloes/arduino-device.git 
```

- Via Arduino Library Manager, look for AloesDevice


## Library usage

- Create the directory `data` at the root of your Arduino project

- Create the file `./data/sensors.json` following [Open Mobile Alliance LwM2M Registry](http://openmobilealliance.org/wp/OMNA/LwM2M/LwM2MRegistry.html) and [Aloes](https://aloes.io/about?tab=docs) ( see examples )

- Open any .ino file of the project folder with Arduino IDE

- Upload `sensors.json` on your ESP** board

- Edit your preferences in `config.h`, at the root of your Arduino project

- Enter wifi and device credentials or configure the board via the Access Point ( 192.168.244.1 )

- Upload the code on your ESP** board

- If the device is not registerd on your Aloes account, copy the `DevEUI` ( generated at setup on the serial interface ) to create one on Aloes backend

- Configure Aloes credentials and servers configuration in  `config.h` to connect to Aloes backend

- You can use examples as a base for your project