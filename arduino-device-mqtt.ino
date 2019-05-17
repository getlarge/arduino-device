#include "dependencies.h"
// add your dependencies under

#include "config.h"
#include "globals.h"

// ADD YOUR LOGIC HERE 

//  CALLED on incoming mqtt/serial message
void onMessage(Message &message, char sensorId[4]) {
  if ( strcmp(message.omaObjectId, "3306") == 0 && strcmp(message.omaResourceId, "5850") == 0) {
    if ( strcmp(message.method, "1") == 0 ) {
      if ( ( strcmp(message.payload, "true") == 0 || strcmp(message.payload, "1") == 0 )) {
        digitalWrite(RELAY_SWITCH, HIGH);
        digitalWrite(STATE_LED, LOW);
      } else if (( strcmp(message.payload, "false") == 0 ||  strcmp(message.payload, "0") == 0 )) {
        digitalWrite(RELAY_SWITCH, LOW);
        digitalWrite(STATE_LED, HIGH);
      }
    } else if ( strcmp(message.method, "2") == 0 ) {
      int val = digitalRead(RELAY_SWITCH);
      // publish val
    }
  }
}

void beforeSetup() {
}

void afterSetup() {
}

void setup() {
  deviceSetup();
}

void beforeLoop() {
}

void afterLoop() {
}

void loop() {
  if ( !executeOnce ) {
    executeOnce = true;
    aSerial.v().println(F("====== Loop started ======"));
  }
  deviceLoop();
}
