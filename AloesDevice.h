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

#if CLIENT_SECURE == 1
#include <WiFiClientSecure.h>
//	#include <WiFiClientSecureBearSSL.h>
#endif

#if defined(ESP8266)
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#elif defined(ESP32)
#include <HTTPClient.h>
#include <Update.h>
#endif

#include <PubSubClient.h>

#include "lib/Globals.h"

#include "lib/AsyncWait.h"

#include "lib/Aloes.cpp"
extern Aloes aloes;
Aloes aloes;

#include "lib/Exec.cpp"

#endif
