# Arduino-device-mqtt

ESP8266 arduino library to interact with [Aloes backend](https://framagit.org/aloes/device-manager) MQTT API.


## Requirements

Arduino IDE - download the latest from arduino

- https://www.arduino.cc/en/Main/Software

Packages for ESP8266

- Enter `http://arduino.esp8266.com/stable/package_esp8266com_index.json` into Additional Board Manager URLs field. 


## Installation

- Via git, clone into your `~/Arduino/Libraries` folder
```
git clone https://framagit.org/aloes/arduino-device-mqtt.git
```

- Via Arduino Library Manager, look for AloesDevice


## Usage

- Open any .ino file of the folder with Arduino IDE

- Edit your preferences in `config.h`

- Enter wifi and device credentials or configure the board via the Access Point ( 192.168.244.1 )

- Configure your sensors following [Open Mobile Alliance LwM2M Registry](http://openmobilealliance.org/wp/OMNA/LwM2M/LwM2MRegistry.html) ( see examples )

- Upload the code on your ESP8266 board

- Copy the deviceId ( generated at setup on the serial interface ) to Aloes backend


