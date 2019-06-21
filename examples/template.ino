#include "config.h"

#if CLIENT_SECURE == 1 && defined(HTTP_CLIENT_SECURE) || defined(MQTT_CLIENT_SECURE)
#include "secure-credentials.h"
#endif

#include <AloesDevice.h>

// ADD YOUR LOGIC HERE

//  CALLED on errors
void onError(modules module, char* error) {
  if (module == TRANSPORT) {
    aSerial.vv().p(F("[TRANSPORT] error : ")).pln(error);
  }
}

// CALLED on incoming http/mqtt/serial message
// MQTT pattern = "prefixedDevEui/+method/+objectId/+sensorId/+resourceId",
// HTTP pattern = "apiRoot/+collection/+path/#param"
void onMessage(transportLayer transportType, Message *message) {
  if (transportType == MQTT) {
    char* payload = message->get(PAYLOAD);
    const char* method = message->get(METHOD);
    const char* objectId = message->get(OBJECT_ID);
    const char* resourceId = message->get(RESOURCE_ID);
    const char* sensorId = message->get(SENSOR_ID);
    if ( strcmp(objectId, "3306") == 0 && strcmp(resourceId, "5850") == 0) {
      if ( strcmp(method, "1") == 0 ) {
        if ( ( strcmp(payload, "true") == 0 || strcmp(payload, "1") == 0 )) {
          digitalWrite(RELAY_SWITCH, HIGH);
          digitalWrite(STATE_LED, LOW);
        } else if (( strcmp(payload, "false") == 0 ||  strcmp(payload, "0") == 0 )) {
          digitalWrite(RELAY_SWITCH, LOW);
          digitalWrite(STATE_LED, HIGH);
        }
      } else if ( strcmp(method, "2") == 0 ) {
        int val = digitalRead(RELAY_SWITCH);
        char newPayload[10];
        itoa(val, newPayload, 10);
        message->set(METHOD, "1").set(PAYLOAD, newPayload);
        aloes.sendMessage(MQTT);
      }
    }
    return;
  } else if (transportType == HTTP) {
    const char* method = message->get(METHOD);
    const char* collection = message->get(COLLECTION);
    const char* path = message->get(PATH);
    //  const char* body = message->get(PAYLOAD);

    //  aSerial.vvv().p(F("onMessage : HTTP : ")).p(collection).p(" ").pln(path);
    return;
  }
}


void setup() {
  if (!initDevice()) {
    return;
  }
  // your device custom setup
}

void loop() {
  if (!deviceRoutine()) {
    return;
  }
  // YOUR LOOP CODE
}