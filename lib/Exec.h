#ifndef Exec_h
#define Exec_h

#include "Manager.h"
#include "Helpers.h"
#include "Aloes.h"
#include "Transport.h"


void initDevice();
void onReceive(char* topic, byte* payload, unsigned int length);
void deviceRoutine();

bool executeOnce = false;
bool resetConfig = false;
bool wifiResetConfig = false; // set to true to reset FS and/or Wifimanager, don't forget to set this to false after

#endif
