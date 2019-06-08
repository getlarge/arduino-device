#include "Exec.h"

void onReceive(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  // byte* p = (byte*)malloc(length);
  // memcpy(p, payload, length);
  aSerial.v().println(F("====== Received message ======"));
  aSerial.vv().p(F("Topic : ")).pln((const char*)topic).p(F("Payload : ")).pln((const char*)payload);
  aloes.parseTopic(topic);
  aloes.parseMessage(payload);
  //  free(p);
}

void beforeLoop() {

}

void initDevice() {
  Serial.begin(BAUD_RATE);
  //checkSerial();
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
  randomSeed(micros());
  helpers.setPins();
  helpers.startTick(1.5);
  aloes.getDeviceId(config);
  aSerial.vvv().print(F("before heap size : ")).println( ESP.getFreeHeap());
  aSerial.v().println(F("====== Setup started ======"));
  aSerial.vvv().pln(F("mounting FS..."));
  if (SPIFFS.begin()) {
    aSerial.vvv().pln(F("FS mounted"));
    if (SPIFFS.exists(manager.configFileName)) {
      manager.loadConfig(manager.configFileName, config);
    } else {
      helpers.generateId(config);
      manager.initDefaultConfig(manager.configFileName, config);
    }
  } else {
    aSerial.vv().pln(F("Failed to mount FS."));
  }
  //  if (config.otaSignal == 1 ) {
  //    //WiFi.persistent(false);
  //    String ssid = WiFi.SSID();
  //    String pass = WiFi.psk();
  //    WiFiMulti.addAP(ssid.c_str(), pass.c_str());
  //    while (WiFiMulti.run() != WL_CONNECTED) { //use this when using ESP8266WiFiMulti.h
  //      aSerial.vvv().println(F("Attempting Wifi connection.... "));
  //      delay(500);
  //    }
  //    aSerial.vv().print(F("Wifi connected. IP Address : ")).println(WiFi.localIP());
  //    //getUpdated(int which, const char* url, const char* fingerprint);
  //  }

  aloes.setSensors(config);
  network.connect(config);
  transport.init(config, onReceive);
  transport.connect(config);

 // if (transport.connected()) {
  aloes.presentSensors(config);
  //  } 

  aSerial.vvv().print(F("setup heap size : ")).println(ESP.getFreeHeap());
  digitalWrite(STATE_LED, HIGH);
  helpers.stopTick();
  aSerial.v().println(F("====== Setup ended ======"));
}

void deviceRoutine() {
  if ( !executeOnce ) {
    executeOnce = true;
    aSerial.v().println(F("====== Loop started ======"));
    //  beforeLoop();
  }
  helpers.loop();
  transport.loop();
  manager.loop();
}
