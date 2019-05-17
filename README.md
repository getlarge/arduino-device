# Arduino-device-mqtt

ESP8266 arduino sketch to interact with [Aloes backend](https://framagit.org/aloes/device-manager) MQTT API.


## Requirements

Arduino IDE - download the latest from arduino

- https://www.arduino.cc/en/Main/Software

Packages for ESP8266

- Enter `http://arduino.esp8266.com/stable/package_esp8266com_index.json` into Additional Board Manager URLs field. 

following libraries are required :

- ArduinoJson
- Bounce2
- FS
- Ticker
- WifiManager

## Installation

```
git clone https://framagit.org/aloes/arduino-device-mqtt.git
```

## Usage

- Open any .ino file of the folder with Arduino IDE

- Edit your preferences in `config.h`

- Upload the code on your ESP8266 board

#if using MQTT_CLIENT

- Copy the deviceId ( generated at setup on the serial interface ) to Aloes backend

- Enter wifi and device credentials in `config.h` or configure the board via the Access Point ( 192.168.244.1 )

#endif


## Reference

- Set resetConfig to true, to restore default state at reset
