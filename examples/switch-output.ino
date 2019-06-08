#include "config.h"
// const char* sensors[][60] = {
//   { "3306", "1", "5850", "digital_input"},
//   { "3306", "2", "5850", "digital_input"},
//   { "3306", "3", "5850", "digital_input"},
// };

#include <AloesDevice.h>

#define INPUT_PIN_1  D5
#define INPUT_PIN_2  D6
#define INPUT_PIN_3  D7

void setupPins() {
  pinMode(INPUT_PIN_1, OUTPUT);
  digitalWrite(INPUT_PIN_1, 1);
  pinMode(INPUT_PIN_2, OUTPUT);
  digitalWrite(INPUT_PIN_2, 1);
  pinMode(INPUT_PIN_3, OUTPUT);
  digitalWrite(INPUT_PIN_3, 1);
}

//  CALLED on incoming mqtt/serial message
void Aloes::onMessage(Message &message) {
  if ( strcmp(message.omaObjectId, "3306") == 0 && strcmp(message.omaResourceId, "5850") == 0) {
    if ( strcmp(message.method, "1") == 0 ) {
      if ( ( strcmp(message.payload, "true") == 0 || strcmp(message.payload, "1") == 0 )) {
        int pin;
        int state = 1;
        if (strcmp(message.sensorId, "1") == 0 ) {
          // pin = atoi(message.sensorId);
          pin = INPUT_PIN_1;
        } else if (strcmp(message.sensorId, "2") == 0 ) {
          pin = INPUT_PIN_2;
        } else if (strcmp(message.sensorId, "3") == 0 ) {
          pin = INPUT_PIN_3;
        }
        //  aSerial.vvv().p(F("set pin : ")).p(pin).p(F(" State : ")).pln(state);;
        if (!pin) return;
        digitalWrite(pin, state);
      } else if (( strcmp(message.payload, "false") == 0 ||  strcmp(message.payload, "0") == 0 )) {
        int pin;
        int state = 0;
        if (strcmp(message.sensorId, "1") == 0 ) {
          pin = INPUT_PIN_1;
        } else  if (strcmp(message.sensorId, "2") == 0 ) {
          pin = INPUT_PIN_2;
        } else  if (strcmp(message.sensorId, "3") == 0 ) {
          pin = INPUT_PIN_3;
        }
        //  aSerial.vvv().p(F("set pin : ")).p(pin).p(F(" State : ")).pln(state);;
        if (!pin) return;
        digitalWrite(pin, state);
      }
    } else if ( strcmp(message.method, "2") == 0 ) {
      int pin;
      if (strcmp(message.sensorId, "1") == 0 ) {
        pin = INPUT_PIN_1;
      } else if ( strcmp(message.sensorId, "2") == 0 ) {
        pin = INPUT_PIN_2;
      } else if ( strcmp(message.sensorId, "3") == 0 ) {
        pin = INPUT_PIN_3;
      }
      if (!pin) return;
      int val = digitalRead(pin);
      char payload[10];
      itoa(val, payload, 10);
      //  dtostrf(val, 10, 0, payload);
      char method[5] = "1";
      aloes.setMessage(message, method, message.omaObjectId, message.sensorId, message.omaResourceId, payload);
      return aloes.sendMessage(config, message);
    }
  }
}


void setup() {
  initDevice();
  setupPins();
}

void loop() {
  deviceRoutine();
}


