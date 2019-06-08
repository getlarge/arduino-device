#include "config.h"
// const char* sensors[][60] = {
//   { "3306", "1", "5850", "digital_input"},
//   { "3342", "2", "5850", "digital_output"},
// };

#include <AloesDevice.h>

#define BUTTON_PIN_1  D5

unsigned long releaseInterval = 1000;
int buttonState1;
unsigned long buttonTimeStamp1;
char sensorId[4] = "2";

Bounce debouncer1 = Bounce();

void setupPins(Bounce *instance, int pin);
void readButton(Bounce *instance, int btnState, unsigned long timestamp);

void setupPins(Bounce *instance, int pin) {
  instance->attach(pin, INPUT_PULLUP);
  //  debouncer1.attach(BUTTON_PIN_1, INPUT_PULLUP);
  debouncer1.interval(releaseInterval);
}

void readButton(Bounce *instance, int btnState, unsigned long timestamp) {
  //  boolean changed = debouncer1.update();
  boolean changed = instance->update();

  if ( changed ) {
    int value = debouncer1.read();
    if ( value == HIGH && buttonState1 == 0 ) {
      buttonState1 = 0;
    } else if ( value == HIGH && buttonState1 == 1 ) {
      buttonState1 = 0;
      aSerial.vvv().pln(F("Button released"));
      char payload[5] = "0";
      setMessage(message, (char*)"1", (char*)"3342", sensorId, (char*)"5500", payload );
      return sendMessage(config, message);
    } else {
      buttonState1 = 1;
      aSerial.vvv().p(F("Button pushed : ")).pln(buttonState1);
      buttonTimeStamp1 = millis();
    }
  }
  if  ( buttonState1 == 1 ) {
    if ( millis() - buttonTimeStamp1 >= releaseInterval ) {
      buttonTimeStamp1 = millis();
      char payload[10];
      itoa(buttonState1, payload, 10);
      aSerial.vvv().p(F("Retriggering button : ")).p(payload).p("  ").pln(sensorId);
      setMessage(message, (char*)"1", (char*)"3342", sensorId, (char*)"5500", payload );
      return sendMessage(config, message);
    }
  }
}

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
  setupPins(debouncer1, BUTTON_PIN_1);
}

void loop() {
  deviceRoutine();
  readButton(debouncer1, buttonState1, buttonTimeStamp1);
}
