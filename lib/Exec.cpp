#include "Exec.h"

void onMQTTMessage(char *topic, uint8_t *payload, unsigned int length) {
  // execMQTTCallback = true;
  aSerial.vv().println(F("====== [MQTT] message ======"));
  //  aSerial.vv().p(F("Topic : ")).pln((const char*)topic);
  if (!aloes.parseRoute(MQTT, topic)) {
    onErrorCallback(EXEC, (char *)"Failed to parse topic.");
    return;
  }
  if (!aloes.parseMessage(MQTT, payload, length)) {
    onErrorCallback(EXEC, (char *)"Failed to parse payload.");
    return;
  }
}

void onHTTPMessage(char *url, uint8_t *body, unsigned int length) {
  // execHTTPCallback = true;
#ifndef NO_HTTP_CLIENT_FEATURE
  aSerial.vv().println(F("====== [HTTP] message ======"));
  // if contenttype = json
  if (!aloes.parseRoute(HTTP, url)) {
    onErrorCallback(EXEC, (char *)"Failed to parse url.");
    return;
  }
  if (!aloes.parseMessage(HTTP, body, length)) {
    onErrorCallback(EXEC, (char *)"Failed to parse body.");
    return;
  }
  // execHTTPCallback = false;
#endif
}

void onDeviceUpdate() {
  aSerial.vv().println(F("====== [DEVICE] updated ======"));
  aloes.onDeviceUpdate();
}

void onErrorCallback(modules module, char *error) {
#ifdef PRINT_ERRORS
  switch (module) {
  case NETWORK: {
    aSerial.vv().p(F("[NETWORK] error : ")).pln(error);
    break;
  }
  case DEVICE: {
    aSerial.vv().p(F("[DEVICE] error : ")).pln(error);
    break;
  }
  case MESSAGE: {
    aSerial.vv().p(F("[MESSAGE] error : ")).pln(error);
    break;
  }
  case TRANSPORT: {
    aSerial.vv().p(F("[TRANSPORT] error : ")).pln(error);
    break;
  }
  case ALOES: {
    aSerial.vv().p(F("[ALOES] error : ")).pln(error);
    break;
  }
  case EXEC: {
    aSerial.vv().p(F("[EXEC] error : ")).pln(error);
    break;
  }
  default: { aSerial.vv().p(F("[] error : ")).pln(error); }
  }
#endif
  onError(module, error);
}

bool aloesSetup() {
  Serial.begin(BAUD_RATE);

#if DEBUG != 0
  aSerial.setPrinter(Serial);
#elif DEBUG == 0
  aSerial.off();
  Serial.setDebugOutput(false);
#endif

#if DEBUG == 1
  aSerial.setFilter(Level::v);
#elif DEBUG == 2
  aSerial.setFilter(Level::vv);
#elif DEBUG == 3
  aSerial.setFilter(Level::vvv);
#elif DEBUG == 4
  aSerial.setFilter(Level::vvvv);
#endif
  aSerial.v().p(F("====== ")).p(SKETCH_NAME).pln(F(" ======"));

#if defined(ESP8266)
  ESP.resetFreeContStack();
  uint32_t freeStackStart = ESP.getFreeContStack();
#elif defined(ESP32)
  //  ESP.resetFreeHeap();
  uint32_t freeStackStart = ESP.getFreeHeap();
#endif
  aSerial.v().println(F("====== Setup ======"));

  if (!aloes.init(onDeviceUpdate, onErrorCallback)) {
    onErrorCallback(
        EXEC, (char *)"Failed to init device, fix your device configuration.");
    return false;
  }

  aloes.setMsgCallback(onMessage, onMQTTMessage, onHTTPMessage);

#if defined(ESP8266)
  uint32_t freeStackEnd = ESP.getFreeContStack();
#elif defined(ESP32)
  uint32_t freeStackEnd = ESP.getFreeHeap();
#endif

#if defined(ESP8266) || defined(ESP32)
  aSerial.v().p(F("Stack used : ")).pln(freeStackStart - freeStackEnd);
#endif

  return true;
}

bool aloesLoop() {

#ifndef NO_HTTP_CLIENT_FEATURE
  if (!aloes.stateReceived) {
    // if (!aloes.stateReceived && !callConfigMode) {
    MilliSec lastStateReqAttempt = millis();
    static AsyncWait retryGetState;
    if (aloes.retryGetStateCount >= 4) {
      retryGetState.cancel();
      aloes.stateReceived = true;
      return true;
    }
    if (retryGetState.isWaiting(lastStateReqAttempt)) {
      return false;
    }
    if (!aloes.getState() || !aloes.stateReceived) {
      retryGetState.startWaiting(lastStateReqAttempt, 5000);
      aloes.retryGetStateCount += 1;
      return false;
    }
  }
#endif

  if (!aloes.client->connected(MQTT)) {
    //  helpers.startTick(0.3);
    MilliSec lastMQTTConnAttempt = millis();
    static AsyncWait asyncMQTT;
    if (asyncMQTT.isWaiting(lastMQTTConnAttempt)) {
      return false;
    }
    if (aloes.client->mqttFailCount > aloes.client->mqttMaxFailedCount) {
      // and not network.configmode
      aloes.client->mqttFailCount = 0;
      // callConfigMode = true;
      asyncMQTT.cancel();
      return false;
    }
    if (!aloes.client->connect(MQTT)) {
      ++aloes.client->mqttFailCount;
      asyncMQTT.startWaiting(lastMQTTConnAttempt, 5000);
      return false;
    }
    //  if (_transport.asyncConnect(MQTT, &asyncMQTT, lastMQTTConnAttempt,
    //  2000)) { asyncMQTT.cancel(); return false;
    //  }
  }

  if (!aloes.client->loop()) {
    return false;
  }

  if (!aloes.sensorsPresented) {
    if (aloes.client->connected(MQTT)) {
      aloes.presentSensors();
      aloes.sensorsPresented = true;
    }
    return false;
  }

  // MilliSec lastMQTTConnAttempt = millis();
  // static AsyncWait asyncMQTT;
  // if (asyncMQTT.isWaiting(lastMQTTConnAttempt)) {
  //   // if (aloes.client->mqttFailCount > aloes.client->mqttMaxFailedCount &&
  //   // !callConfigMode) {
  //   //   aloes.client->mqttFailCount = 0;
  //   //   callConfigMode = true;
  //   //   asyncMQTT.cancel();
  //   // }
  //   return false;
  // }

  // bool rc = aloes.client->loop();
  // if (!rc) {
  //   asyncMQTT.startWaiting(lastMQTTConnAttempt, 5000);
  //   ++aloes.client->mqttFailCount;
  //   if (!aloes.client->connect(MQTT)) {
  //     return false;
  //   }
  //   // asyncMQTT.cancel();
  // }

  // if (!aloes.sensorsPresented) {
  //   if (aloes.client->connected(MQTT)) {
  //     aloes.presentSensors();
  //     aloes.sensorsPresented = true;
  //     return true;
  //   }
  //   return false;
  // }

  return true;
}

void stop() { aloes.client->cleanMQTT(); }