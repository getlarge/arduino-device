#ifndef AloesDevice_h
#define AloesDevice_h

//	#include <Arduino.h>

#include <FS.h>
#include <advancedSerial.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266mDNS.h>
//  #include <DNSServer.h>
#include <TimeLib.h>
#include <Wire.h>


#if CLIENT_SECURE == 1
#include <WiFiClientSecure.h>
#endif

#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>

#include "lib/Constants.h"

ESP8266WiFiMulti WiFiMulti;

#if CLIENT_SECURE == 0
WiFiClient wifiClient;
#elif CLIENT_SECURE == 1
WiFiClientSecure wifiClient;
#endif

Config config;
Message message;

// todo :remove from global but need to change attachInterrupt callback signature ?
WiFiManager wifiManager;


#include "lib/Helpers.cpp"
Helpers helpers; 

#include "lib/Transport.cpp"
#if MQTT_CLIENT == 1
Transport transport(wifiClient); 
#elif WEB_SERVER == 1
ESP8266WebServer server(80);
WiFiClient client = server.client();
Transport transport(client); 
#endif

#include "lib/Network.cpp"
Network network(config); 

#include "lib/Manager.cpp"
Manager manager(config); 

#include "lib/Aloes.cpp"
extern Aloes aloes;
Aloes aloes;

#include "lib/Exec.cpp"

#endif
