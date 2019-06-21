#ifndef AloesDevice_h
#define AloesDevice_h

#include <Arduino.h>

#include <FS.h>
#if defined(ESP8266) 
#elif defined(ESP32)
#include <SPIFFS.h>
#define FORMAT_SPIFFS_IF_FAILED true
#endif

#include <ArduinoJson.h>
#include <advancedSerial.h>

#if defined(ESP8266) 
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif

#include "lib/Globals.h"

#include "lib/AsyncWait.h"

#include "lib/Device.cpp"
Device device;

#include "lib/Helpers.cpp"
Helpers helpers; 

#include "lib/Network.cpp"
Network _network; 

#include "lib/Manager.cpp"
Manager manager(device); 

#include "lib/Transport.cpp"
Transport _transport; 

#include "lib/Aloes.cpp"
extern Aloes aloes;
Aloes aloes;

#include "lib/Exec.cpp"

#endif
