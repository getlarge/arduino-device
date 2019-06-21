#include "Exec.h"

void onMQTTMessage(char* topic, uint8_t* payload, unsigned int length) {
  aSerial.vv().println(F("====== [MQTT] message ======"));
  //  aSerial.vv().p(F("Topic : ")).pln((const char*)topic);
  if (!aloes.parseTopic(topic)) {
    onErrorCallback(EXEC, (char*)"Failed to parse topic.");
    return;
  }
  if (!aloes.parseMessage(payload, length)) {
    onErrorCallback(EXEC, (char*)"Failed to parse payload.");
  }
}

void onHTTPMessage(char* url, uint8_t* body, unsigned int length) {
  aSerial.vv().println(F("====== [HTTP] message ======"));
  // if contenttype = json
  if (!aloes.parseUrl(url)) {
    onErrorCallback(EXEC, (char*)"Failed to parse url.");
    return;
  }
  aloes.parseBody(body, length);
}

void onDeviceUpdate() {
  aSerial.vv().println(F("====== [DEVICE] updated ======"));
  aloes.onDeviceUpdate();
}

void onErrorCallback(modules module, char* error) {
#ifdef PRINT_ERRORS
  switch(module) {
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
    default: {
      aSerial.vv().p(F("[] error : ")).pln(error);
    }
  }
#endif
  onError(module, error);
}

bool initDevice() {
  Serial.begin(BAUD_RATE);
  //  checkSerial();
  
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
  aSerial.v().println(F("====== Before setup ======"));

  helpers.setPins();
  helpers.startTick(1.5);

  for (uint8_t t = 4; t > 0; t--) {
    aSerial.vvv().print(F("[SETUP] WAIT ")).print(t).println(" ...");
    Serial.flush();
    delay(1000);
  }
  // if (wifiResetConfig) { // add a button ?
  //   WiFiManager wifiManager;
  //   wifiManager.resetSettings();
  // }
  // if (resetConfig) {
  //   setDefault();
  // }
  //  randomSeed(micros());

#if defined(ESP8266) 
  ESP.resetFreeContStack();
  uint32_t freeStackStart = ESP.getFreeContStack();
#elif defined(ESP32)
  //  ESP.resetFreeHeap();
  uint32_t freeStackStart = ESP.getFreeHeap();
#endif
  aSerial.v().println(F("====== Setup started ======"));

  _network.generateId(device);

  if (!aloes.initDevice(onDeviceUpdate)) {
    onErrorCallback(EXEC, (char*)"Failed to init device, fix your device configuration.");
    return false;
  }
  aloes.initSensors();

  if (!_network.setup(device)) {
    onErrorCallback(EXEC, (char*)"Failed to connect to WiFi AP, update your configuration.");
  }

  if (!_transport.setupHTTP(device)) {
    onErrorCallback(EXEC, (char*)"Failed to configure HTTP Client.");
  }
  _transport.setHTTPCallback(onHTTPMessage);

  if (!_transport.setupMQTT(device)) {
    onErrorCallback(EXEC, (char*)"Failed to configure MQTT Client.");
  }
  _transport.setMQTTCallback(onMQTTMessage);

  // to receive formatted messages
  aloes.setMsgCallback(onMessage);

  // to receive error messages
  device.setErrorCallback(onErrorCallback);
  _network.setErrorCallback(onErrorCallback);
  _transport.setErrorCallback(onErrorCallback);

  manager.setup();

  helpers.stopTick();
  digitalWrite(STATE_LED, HIGH);
  aSerial.v().println(F("====== Setup ended ======"));
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

bool asyncConnectMQTT(AsyncWait *async, MilliSec startTime, unsigned long interval) {
  if (async->isWaiting(startTime)) {
    return true;
  }
  if (_transport.connect(MQTT)) {
    _transport.mqttFailCount = 0;
    async->cancel();
    return false;
  }
  if (_transport.mqttFailCount > _transport.mqttMaxFailedCount && !callConfigMode) {
    _transport.mqttFailCount = 0;
    callConfigMode = true;
    async->cancel();
    return false;
  }
  ++_transport.mqttFailCount;
  async->startWaiting(startTime, interval);
  return true;
}

bool deviceRoutine() {
  if (!executeOnce && _network.connected()) {
    aSerial.v().println(F("====== Loop started ======"));
    randomSeed(micros());
    _network.setClock();
    executeOnce = true;
  }

  helpers.loop();
  manager.loop(device);

  if (!_network.connected()) {
    _transport.disconnect(MQTT);
    MilliSec lastWifiConnAttempt = millis();
    static AsyncWait asyncWifi;
    //  onErrorCallback(EXEC, (char*)"Trying to reconnect WiFi Client ...");
    if (_network.asyncConnect(&asyncWifi, lastWifiConnAttempt, 1000)) {
      return false;
    }
  }

  if (!_network.loop()) {
    _transport.disconnect(MQTT);
    return false;
  }

  if (!_transport.connected(MQTT)) {
     //  helpers.startTick(0.3);
    MilliSec lastMQTTConnAttempt = millis();
    static AsyncWait asyncMQTT;
    if (!callConfigMode) {
      if (_transport.asyncConnect(MQTT, &asyncMQTT, lastMQTTConnAttempt, 2000)) {
        return false;
      }
    } else {
      asyncMQTT.cancel();
    }
    return false;
  }

  _transport.loop(device);


  if (!aloes.stateReceived && !callConfigMode) {
    MilliSec lastStateReqAttempt = millis();
    static AsyncWait retryGetState;
    if (retryGetState.isWaiting(lastStateReqAttempt)) {
      return false;
    }
    if (!aloes.getState() || !aloes.stateReceived) {
      retryGetState.startWaiting(lastStateReqAttempt, 5000);
      return false;
    }
  }

  if (!aloes.sensorsPresented) {
    if (_transport.connected(MQTT)) {
      aloes.presentSensors();
      aloes.sensorsPresented = true;
    } 
    return false;
  }

  return true;
}
