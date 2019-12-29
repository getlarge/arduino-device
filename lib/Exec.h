/*
 * Exec.h
 *
 * Author:  Getlarge
 * Created: 2019-06-10
 */

#ifndef Exec_h
#define Exec_h

// #include "Globals.h"

void ICACHE_RAM_ATTR onMQTTMessage(char *topic, uint8_t *payload,
                                   unsigned int length);
void ICACHE_RAM_ATTR onHTTPMessage(char *url, uint8_t *body,
                                   unsigned int length);
void ICACHE_RAM_ATTR onMessage(transportLayer type, Message *message);
void ICACHE_RAM_ATTR onDeviceUpdate();
void ICACHE_RAM_ATTR onErrorCallback(modules moduleName, char *error);
void ICACHE_RAM_ATTR onError(modules moduleName, char *error);

bool aloesSetup();
bool aloesLoop();
void stop();

#endif
