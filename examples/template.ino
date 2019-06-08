#include "config.h"
#include <AloesDevice.h>

// ADD YOUR LOGIC HERE

//  CALLED on incoming mqtt/serial message
void Aloes::onMessage(Message &message) {
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
      char payload[10];
      itoa(val, payload, 10);
      aloes.setMessage(message, (char*)"1", message.omaObjectId, message.sensorId, message.omaResourceId, payload);
      return aloes.sendMessage(config, message);
    }
  }
}


void setup() {
  initDevice();
}

void loop() {
  deviceRoutine();
}
