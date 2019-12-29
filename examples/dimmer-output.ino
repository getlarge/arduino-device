#include "config.h"

#if CLIENT_SECURE == 1
#include "secure-credentials.h"
#endif

#include <AloesDevice.h>

#include <RBDdimmer.h>

#define outputPin D2

#if defined(ESP8266) || defined(ESP32)
#define zeroCross D1
#endif

#if defined(ESP8266) || defined(ESP32)
dimmerLamp dimmer(outputPin, zeroCross);
#else
dimmerLamp dimmer(outputPin);
#endif

int minDimmerVal = 0;
int maxDimmerVal = 100;

void setupDimmer();
void setDimmer(int value, int minRange, int maxRange);

void setupDimmer() {
  dimmer.begin(NORMAL_MODE,
               OFF); // dimmer initialisation: name.begin(MODE, STATE)
  //  dimmer.begin(TOGGLE_MODE, OFF);
  //  dimmer.toggleSettings(minDimmerVal, maxDimmerVal);
  //  dimmer.setState(ON);
}

void setDimmer(int value, int minRange, int maxRange) {
  if (value > maxRange) {
    value = maxRange;
  } else if (value < minRange) {
    value = minRange;
  }
  int outVal = 0;
  outVal = map(value, minRange, maxRange, minDimmerVal, maxDimmerVal);
  aSerial.vv().p(F("setDimmer to : ")).p(outVal).pln(F("%"));
  dimmer.setPower(outVal);
}

//  CALLED on errors
void onError(modules moduleName, char *error) {
  switch (moduleName) {
  case EXEC: {
    aSerial.vv().p(F("[EXEC] error : ")).pln(error);
    break;
  }
  }
}

// CALLED on incoming http/mqtt/serial message
// MQTT pattern = "prefixedDevEui/+method/+objectId/+sensorId/+resourceId",
// HTTP pattern = "apiRoot/+collection/+path/#param"
void onMessage(transportLayer type, Message *message) {
  if (type == MQTT) {
    char *payload = message->get(PAYLOAD);
    const char *method = message->get(METHOD);
    const char *objectId = message->get(OBJECT_ID);
    const char *resourceId = message->get(RESOURCE_ID);
    const char *sensorId = message->get(SENSOR_ID);

    if (strcmp(objectId, "3306") == 0 && strcmp(resourceId, "5850") == 0) {
      if (strcmp(method, "1") == 0) {
        if ((strcmp(payload, "true") == 0 || strcmp(payload, "1") == 0)) {
          dimmer.setState(ON);
          digitalWrite(STATE_LED, 0);
        } else if ((strcmp(payload, "false") == 0 ||
                    strcmp(payload, "0") == 0)) {
          dimmer.setState(OFF);
          digitalWrite(STATE_LED, 1);
        }
      } else if (strcmp(method, "2") == 0) {
        //  dimmer.getPower();
        bool val = dimmer.getState();
        const char *newPayload;
        if (val) {
          newPayload = "true";
        } else {
          newPayload = "false";
        }
        message->set(METHOD, "2").set(PAYLOAD, newPayload);
        aloes.sendMessage(MQTT);
      }
    } else if (strcmp(objectId, "3306") == 0 &&
               strcmp(resourceId, "5851") == 0) {
      if (strcmp(method, "1") == 0) {
        int val = atoi(payload);
        setDimmer(val, 10, 90);
      } else if (strcmp(method, "2") == 0) {
        int val = dimmer.getPower();
        char newPayload[10];
        itoa(val, newPayload, 10);
        message->set(METHOD, "2").set(PAYLOAD, newPayload);
        aloes.sendMessage(MQTT);
      }
    }

    return;
  } else if (type == HTTP) {
    const char *method = message->get(METHOD);
    const char *collection = message->get(COLLECTION);
    const char *path = message->get(PATH);
    //  const char* body = message->get(PAYLOAD);
    //  aSerial.vvv().p(F("onMessage : HTTP : ")).p(collection).p("
    //  ").pln(path);
    return;
  }
}

void setup() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(DEFAULT_WIFI_SSID, DEFAULT_WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  if (!aloesSetup()) {
    return;
  }
  setupDimmer();
}

void loop() {
  if (!aloesLoop()) {
    return;
  }
}