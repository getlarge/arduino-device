#include <FS.h>
#include <advancedSerial.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>
//  #include <DNSServer.h>
#include <TimeLib.h>
#include <Ticker.h>
#include <Bounce2.h>
#include <Wire.h>

/// NETWORK PARAMS
#define ID_TYPE 0 // 0 : prefix+espCHipId - 1 : prefix+MACadress - 2 : prefix+EUI-64 address
#define CLIENT_SECURE 1
#define MQTT_CLIENT 1
#define WEB_SERVER 0
#define NTP_SERVER 0

#if WEB_SERVER == 1
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#endif
#if NTP_SERVER == 1
#include <WiFiUdp.h>
#endif
#if CLIENT_SECURE == 1
#include <WiFiClientSecure.h>
#endif
#if MQTT_CLIENT == 1
#include <PubSubClient.h>
#endif
