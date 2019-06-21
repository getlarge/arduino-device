/*
 * Exec.h
 *
 * Author:  Getlarge
 * Created: 2019-06-10
 */

#ifndef Exec_h
#define Exec_h

#include "Globals.h"
#include "Manager.h"
#include "Helpers.h"
#include "Aloes.h"
#include "Transport.h"

void ICACHE_RAM_ATTR onMQTTMessage(char* topic, uint8_t* payload, unsigned int length);
void ICACHE_RAM_ATTR onHTTPMessage(char* url, uint8_t* body, unsigned int length);
void ICACHE_RAM_ATTR onMessage(transportLayer transportType, Message *message);
void ICACHE_RAM_ATTR onDeviceUpdate();

void onErrorCallback(modules module, char* error);
void onError(modules module, char* error);

bool asyncConnectMQTT(AsyncWait *async, MilliSec startTime, unsigned long interval);

bool initDevice();
bool deviceRoutine();

bool resetConfig = false;
bool wifiResetConfig = false; // set to true to reset FS and/or Wifimanager, don't forget to set this to false after
bool executeOnce = false;

#endif
