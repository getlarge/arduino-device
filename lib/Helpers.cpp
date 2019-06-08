#include "Helpers.h"

Helpers::Helpers() {
  //  checkSerial();
}

void Helpers::tick() {
  int state = digitalRead(STATE_LED);
  digitalWrite(STATE_LED, !state);
}

void Helpers::startTick(int duration) {
  ticker.attach(duration, tick);
}

void Helpers::stopTick() {
  ticker.detach();
}

void Helpers::generateId(Config &config) {
  strcpy(config.mqttClient, config.devEui);
  long randNumber = random(10000);
  char randNumberBuffer[10];
  ltoa(randNumber, randNumberBuffer, 10);
  strcat(config.mqttClient, "-" );
  strcat(config.mqttClient, randNumberBuffer);
}

void Helpers::setPins() {
  pinMode(STATE_LED, OUTPUT);
  digitalWrite(STATE_LED, HIGH);
  pinMode(RELAY_SWITCH, OUTPUT);
  // todo: get switch value from config store
  digitalWrite(RELAY_SWITCH, LOW);
  debouncer.attach(OTA_BUTTON_PIN, INPUT_PULLUP);
  debouncer.interval(debouncerInterval);
  aSerial.vvv().pln(F("Pins set"));
}


void Helpers::checkButton() {
  int value = debouncer.read();
  if ( value == HIGH) {
    buttonState = 0;
    aSerial.vvv().pln(F("Button released"));
  } else {
    buttonState = 1;
    aSerial.vvv().pln(F("Long push detected --> config mode"));
    buttonPressTimeStamp = millis();
  }
}

void Helpers::readConfigButton() {
  boolean changed = debouncer.update();
  if ( changed ) {
    checkButton();
  }
  if  ( buttonState == 1 ) {
    if ( millis() - buttonPressTimeStamp >= debouncerInterval ) {
      buttonPressTimeStamp = millis();
      aSerial.vvv().pln(F("Retriggering button"));
      if ( manualConfig == true) {
        manualConfig = false;
      }
      else {
        manualConfig = true;
        callConfigMode = true;
      }
    }
  }
}

void Helpers::setReboot() { // Boot to sketch
  pinMode(STATE_LED, OUTPUT);
  digitalWrite(STATE_LED, HIGH);
  //  pinMode(D8, OUTPUT);
  //  digitalWrite(D8, LOW);
  aSerial.vv().pln(F("Pins set for reboot..."));
  Serial.flush();
  yield();
  delay(5000);
  aSerial.v().println(F("====== Reboot ======"));
  ESP.reset(); //ESP.restart();
  delay(2000);
}

// void Helpers::setDefault() {
//   //  ticker.attach(2, tick);
//   ticker.attach(2, helpers->tick);
//   aSerial.v().println(F("====== Reset config ======"));
//   //  resetConfig = false;
//   SPIFFS.begin();
//   delay(10);
//   SPIFFS.format();
//   wifiManager.resetSettings();
//   delay(100);
//   aSerial.v().println(F("====== System cleared ======"));
//   ticker.detach();
//   aSerial.v().pln(ESP.eraseConfig());
//   setReboot();
// }

void Helpers::loop() {
  readConfigButton();
}