#include "config.h"

#if CLIENT_SECURE == 1
#include "secure-credentials.h"
#endif
// const char* sensors[][60] = {
//   { "3306", "1", "5850", "digital_output"},
//   { "3306", "2", "5850", "digital_output"},
//   { "3306", "3", "5850", "digital_output"},
// };
 
#include <AloesDevice.h>

#define OUTPUT_PIN_1  D5
#define OUTPUT_PIN_2  D6
#define OUTPUT_PIN_3  D7

void setupPins() {
  pinMode(OUTPUT_PIN_1, OUTPUT);
  digitalWrite(OUTPUT_PIN_1, 1);
  pinMode(OUTPUT_PIN_2, OUTPUT);
  digitalWrite(OUTPUT_PIN_2, 1);
  pinMode(OUTPUT_PIN_3, OUTPUT);
  digitalWrite(OUTPUT_PIN_3, 1);
}

//  CALLED on errors
void onError(modules module, char* error) {
//  if (module == EXEC) {
//    aSerial.vv().p(F("[EXEC] error : ")).pln(error);
//  } else if (module == TRANSPORT) {
//    aSerial.vv().p(F("[TRANSPORT] error : ")).pln(error);
//  }
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
          int pin;
          int state = 1;
          if (strcmp(sensorId, "1") == 0 ) {
            // pin = atoi(message.sensorId);
            pin = OUTPUT_PIN_1;
          } else if (strcmp(sensorId, "2") == 0 ) {
            pin = OUTPUT_PIN_2;
          } else if (strcmp(sensorId, "3") == 0 ) {
            pin = OUTPUT_PIN_3;
          }
          //  aSerial.vvv().p(F("set pin : ")).p(pin).p(F(" State : ")).pln(state);;
          if (!pin) return;
          digitalWrite(pin, state);
        } else if (( strcmp(payload, "false") == 0 ||  strcmp(payload, "0") == 0 )) {
          int pin;
          int state = 0;
          if (strcmp(sensorId, "1") == 0 ) {
            pin = OUTPUT_PIN_1;
          } else  if (strcmp(sensorId, "2") == 0 ) {
            pin = OUTPUT_PIN_2;
          } else  if (strcmp(sensorId, "3") == 0 ) {
            pin = OUTPUT_PIN_3;
          }
          //  aSerial.vvv().p(F("set pin : ")).p(pin).p(F(" State : ")).pln(state);;
          if (!pin) return;
          digitalWrite(pin, state);
        }
      } else if ( strcmp(method, "2") == 0 ) {
        int pin;
        if (strcmp(sensorId, "1") == 0 ) {
          pin = OUTPUT_PIN_1;
        } else if ( strcmp(sensorId, "2") == 0 ) {
          pin = OUTPUT_PIN_2;
        } else if ( strcmp(sensorId, "3") == 0 ) {
          pin = OUTPUT_PIN_3;
        }
        if (!pin) return;
        int val = digitalRead(pin);
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
  setupPins();
}

void loop() {
  if (!deviceRoutine()) {
    return;
  }
}