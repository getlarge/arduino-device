#include "Manager.h"


WiFiManagerParameter *customMqttServer;
WiFiManagerParameter *customMqttPort;
WiFiManagerParameter *customMqttUser;
WiFiManagerParameter *customMqttPassword;

bool Manager::shouldSaveConfig = true;

Manager::Manager(Config &config) {
#if DEBUG >= 3
  wifiManager.setDebugOutput(true);
#endif
#if DEBUG == 0
  wifiManager.setDebugOutput(false);
#endif
  wifiManager.setAPStaticIPConfig(IPAddress(192, 168, 244, 1), IPAddress(192, 168, 244, 1), IPAddress(255, 255, 255, 0));
  customMqttServer = new WiFiManagerParameter("server", "mqtt server", config.mqttServer, sizeof(config.mqttServer));
  customMqttPort = new WiFiManagerParameter("port", "mqtt port", config.mqttPort, sizeof(config.mqttPort));
  customMqttUser= new WiFiManagerParameter("user", "mqtt user", config.mqttUser, sizeof(config.mqttUser));
  customMqttPassword = new WiFiManagerParameter("password", "mqtt password", config.mqttPassword, sizeof(config.mqttPassword));
  // wifiManager.addParameter(&customMqttServer);
  // wifiManager.addParameter(&customMqttPort);
  // wifiManager.addParameter(&customMqttUser);
  // wifiManager.addParameter(&customMqttPassword);
  //  IPAddress _ip, _gw, _sn;
  //  _ip.fromString(config.staticIp);
  //  _gw.fromString(config.staticGw);
  //  _sn.fromString(config.staticSn);
  //  wifiManager.setSTAStaticIPConfig(_ip, _gw, _sn);
}

void Manager::updateFile(const String fileName, int value) {
  File f = SPIFFS.open(fileName.c_str(), "w");
  if (!f) {
    aSerial.vv().p(fileName.c_str()).pln(F("opening failed"));
  }
  else {
    aSerial.vvv().p("Writing to ").pln(fileName.c_str());
    f.println(value);
    aSerial.vvv().p(fileName.c_str()).pln(F(" updated"));
    f.close();
  }
}

void Manager::loadConfig(const String fileName, Config &config) {
  aSerial.vvv().pln(F("Reading config file."));
  File configFile = SPIFFS.open(fileName, "r");
  if (configFile) {
    size_t size = configFile.size();
    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);
    configFile.readBytes(buf.get(), size);
    DynamicJsonDocument obj(objBufferSize * 2);
    //  DeserializationError error = deserializeJson(obj, buf.get());
    DeserializationError error = deserializeJson(obj, buf.get(), size);
    if (error) {
      aSerial.vv().p(F("Failed to load json config : ")).pln(error.c_str());
    } else {
      serializeJsonPretty(obj, Serial);
      strlcpy(config.mqttServer, obj["mqttServer"] | defaultMqttServer, sizeof(config.mqttServer));
      strlcpy(config.mqttPort, obj["mqttPort"] | defaultMqttPort, sizeof(config.mqttPort));
      strlcpy(config.mqttUser, obj["mqttUser"] | defaultMqttUser, sizeof(config.mqttUser));
      strlcpy(config.mqttPassword, obj["mqttPassword"] | defaultMqttPassword, sizeof(config.mqttPassword));
      strlcpy(config.mqttTopicIn, obj["mqttTopicIn"] | defaultMqttTopicIn,  sizeof(config.mqttTopicIn));
      strlcpy(config.mqttTopicOut, obj["mqttTopicOut"] | defaultMqttTopicOut,  sizeof(config.mqttTopicOut));
      if (obj["ip"]) {
        strcpy(config.staticIp, obj["ip"]);
        strcpy(config.staticGw, obj["gateway"]);
        strcpy(config.staticSn, obj["subnet"]);
      }
    }
  } else {
    aSerial.vv().pln(F("Failed to load config file."));
  }
}

void Manager::initDefaultConfig(const String fileName, Config &config) {
  DynamicJsonDocument obj(objBufferSize * 2);
  obj["mqttServer"] = defaultMqttServer;
  obj["mqttPort"] = defaultMqttPort;
  obj["mqttClient"] = config.mqttClient;
  obj["mqttUser"] = defaultMqttUser;
  obj["mqttPassword"] = defaultMqttPassword;
  obj["mqttTopicIn"] = defaultMqttTopicIn;
  obj["mqttTopicOut"] = defaultMqttTopicOut;
  File configFile = SPIFFS.open(fileName, "w");
  if (!configFile) {
    aSerial.vv().pln(F("Failed to open config file"));
  }
#if DEBUG != 0
  if (serializeJsonPretty(obj, Serial) == 0) {
    Serial.println(F("Failed to write to Serial"));
  }
#endif
  if (serializeJson(obj, configFile) == 0) {
    Serial.println(F("Failed to write to file"));
  }
  configFile.close();
  strlcpy(config.mqttServer, defaultMqttServer, sizeof(config.mqttServer));
  strlcpy(config.mqttPort, defaultMqttPort, sizeof(config.mqttPort));
  strlcpy(config.mqttUser, defaultMqttUser, sizeof(config.mqttUser));
  strlcpy(config.mqttPassword, defaultMqttPassword, sizeof(config.mqttPassword));
  strlcpy(config.mqttTopicIn, defaultMqttTopicIn,  sizeof(config.mqttTopicIn));
  strlcpy(config.mqttTopicOut,  defaultMqttTopicOut,  sizeof(config.mqttTopicOut));
  strlcpy(config.mqttTopicIn, config.devEui, sizeof(config.mqttTopicIn));
  strcat(config.mqttTopicIn, config.inPrefix);
  strlcpy(config.mqttTopicOut, config.devEui, sizeof(config.mqttTopicOut));
  strcat(config.mqttTopicOut, config.outPrefix);
}

void Manager::saveConfig(const String fileName, Config &config) {
  strlcpy(config.mqttTopicIn, config.devEui, sizeof(config.mqttTopicIn));
  strcat(config.mqttTopicIn, config.inPrefix);
  strlcpy(config.mqttTopicOut, config.devEui, sizeof(config.mqttTopicOut));
  strcat(config.mqttTopicOut, config.outPrefix);
  DynamicJsonDocument obj(objBufferSize * 2);
  obj["mqttServer"] = config.mqttServer;
  obj["mqttPort"] = config.mqttPort;
  obj["mqttClient"] = config.mqttClient;
  obj["mqttUser"] = config.mqttUser;
  obj["mqttPassword"] = config.mqttPassword;
  obj["mqttTopicIn"] = config.mqttTopicIn;
  obj["mqttTopicOut"] = config.mqttTopicOut;
  //    obj["ip"] = WiFi.localIP().toString();
  //    obj["gateway"] = WiFi.gatewayIP().toString();
  //    obj["subnet"] = WiFi.subnetMask().toString();
  File configFile = SPIFFS.open(fileName, "w");
  if (!configFile) {
    aSerial.vv().pln(F("Failed to open config file"));
  }
#if DEBUG != 0
  if (serializeJsonPretty(obj, Serial) == 0) {
    Serial.println(F("Failed to write to Serial"));
  }
#endif
  if (serializeJson(obj, configFile) == 0) {
    Serial.println(F("Failed to write to file"));
  }
  configFile.close();
}

void Manager::saveConfigCallback() {
  shouldSaveConfig = true;
}

void Manager::quitConfigMode() {
  callConfigMode = false;
  if ( configMode == 0 ) {
    return;
  } else if ( configMode == 1 ) {
    wifiManager.setTimeout(5);
    detachInterrupt(digitalPinToInterrupt(OTA_BUTTON_PIN));
    aSerial.vv().pln(F("Quit config mode"));
    return;
  }
}

void Manager::configModeCallback(WiFiManager *myWiFiManager) {
  aSerial.vv().pln(F("====== Config mode opening ======"));
  //aSerial.vvv().p(F("Portal SSID : ")).pln(myWiFiManager->getConfigPortalSSID());
  attachInterrupt(digitalPinToInterrupt(OTA_BUTTON_PIN), quitConfigMode, CHANGE);
  //  if (WiFi.status() != WL_CONNECTED) {
  //    reconnectWifi();
  //  }
  //  if (!mqttClient.connected()) {
  //    reconnectMqtt();
  //  }
  configMode = 1;
  //  network.wifiFailCount = 0;
  //  transport.mqttFailCount = 0;
  helpers.startTick(1);
}

// void Manager::configModeCallback(WiFiManager *myWiFiManager) {
//   configModeCallback(WiFiManager *myWiFiManager);
// }

void Manager::configManager(Config &config) {
  //  WiFiManager wifiManager;

  wifiManager.addParameter(customMqttServer);
  wifiManager.addParameter(customMqttPort);
  wifiManager.addParameter(customMqttUser);
  wifiManager.addParameter(customMqttPassword);

  wifiManager.setAPCallback(configModeCallback);

  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setAPStaticIPConfig(IPAddress(192, 168, 244, 1), IPAddress(192, 168, 244, 1), IPAddress(255, 255, 255, 0));
  //  IPAddress _ip, _gw, _sn;
  //  _ip.fromString(static_ip);
  //  _gw.fromString(static_gw);
  //  _sn.fromString(static_sn);
  //  wifiManager.setSTAStaticIPConfig(_ip, _gw, _sn);
  //wifiManager.setBreakAfterConfig(true);
  wifiManager.setMinimumSignalQuality(10);

  String script;
  script += "<script>";
  script += "document.addEventListener('DOMContentLoaded', function() {";
  script +=     "var params = window.location.search.substring(1).split('&');";
  script +=     "for (var param of params) {";
  script +=         "param = param.split('=');";
  script +=         "try {";
  script +=             "document.getElementById( param[0] ).value = param[1];";
  script +=         "} catch (e) {";
  script +=             "console.log('WARNING param', param[0], 'not found in page');";
  script +=         "}";
  script +=     "}";
  script += "});";
  script += "</script>";
  wifiManager.setCustomHeadElement(script.c_str());

  configCount++;
  aSerial.vv().p(F(" load config file.")).pln((const char*)config.mqttServer);

  //  // After first start, hard reset, or without any known WiFi AP
  //  if (WiFi.status() != WL_CONNECTED) {
  //    aSerial.vv().pln(F("Auto config access"));
  //    if (!wifiManager.autoConnect(config.devEui, config.devicePass)) {
  //      aSerial.v().pln(F("Connection failure --> Timeout"));
  //      delay(3000);
  //      setReboot();
  //    }
  //  }
  // When manually asking ...
  if ((configCount > 0 && manualConfig == true)) {
    //manualConfig = false;
    aSerial.vv().pln(F("Manual config access"));
    wifiManager.setTimeout(DEFAULT_PORTAL_TIMEOUT * 2);
    wifiManager.startConfigPortal(config.deviceName, config.devicePass);
  } else if ((WiFi.status() != WL_CONNECTED || !transport.connected() || (strcmp(config.mqttServer, "") == 0)) && manualConfig == false) {
    aSerial.vv().pln(F("Network config access"));
    wifiManager.setTimeout(DEFAULT_PORTAL_TIMEOUT);
    wifiManager.startConfigPortal(config.deviceName, config.devicePass);
  }

  detachInterrupt(digitalPinToInterrupt(OTA_BUTTON_PIN));
  helpers.stopTick();
  digitalWrite(STATE_LED, HIGH);
  manualConfig = false;
  configMode = 0;
  if ( (strcmp(customMqttServer->getValue(), "") != 0) ) {
    strlcpy(config.mqttServer, customMqttServer->getValue(), sizeof(config.mqttServer));
  }
  if ( (strcmp(customMqttPort->getValue(), "") != 0) ) {
    strlcpy(config.mqttPort, customMqttPort->getValue(), sizeof(config.mqttPort));
  }
  if ( (strcmp(customMqttUser->getValue(), "") != 0) ) {
    strlcpy(config.mqttUser, customMqttUser->getValue(), sizeof(config.mqttUser));
  }
  if ( (strcmp(customMqttPassword->getValue(), "") != 0) ) {
    strlcpy(config.mqttPassword, customMqttPassword->getValue(), sizeof(config.mqttPassword));
  }
  if ( shouldSaveConfig ) {
    saveConfig(configFileName, config);
  }

  callConfigMode = false;
  aSerial.v().pln();
  aSerial.vvv().pln(F("Config successful")).p(F("Config mode counter :")).pln(configCount);
  aSerial.vvv().print(F("config heap size : ")).println(ESP.getFreeHeap());
  aSerial.vv().p(F("IP Address : ")).pln(WiFi.localIP());
  aSerial.v().pln(F("====== Config ended ======"));
}

/// OTA
void Manager::getUpdated(int which, const char* url, const char* fingerprint) {
  if ((WiFi.status() == WL_CONNECTED)) {
    helpers.startTick(0.7);
    otaSignal = 0;
    updateFile(otaFile, otaSignal);
    ESPhttpUpdate.rebootOnUpdate(true);
    //  ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);

    t_httpUpdate_return ret;
    //    if ( which == 0 ) {
    //      WiFiClient client;
    //      aSerial.v().pln(F("Update Sketch..."));
    //      t_httpUpdate_return ret = ESPhttpUpdate.update(client, url);
    //    }
    //
    //    ticker.detach();
    //    switch (ret) {
    //      case HTTP_UPDATE_FAILED:
    //        aSerial.v().p(F("HTTP_UPDATE_FAILD Error : ")).p(ESPhttpUpdate.getLastError()).p(" / ").pln(ESPhttpUpdate.getLastErrorString().c_str());
    //        break;
    //      case HTTP_UPDATE_NO_UPDATES:
    //        aSerial.v().pln(F("HTTP_UPDATE_NO_UPDATES "));
    //        break;
    //      case HTTP_UPDATE_OK:
    //        aSerial.v().pln(F("HTTP_UPDATE_OK"));
    //        break;
    //    }
    helpers.stopTick();
  }
}

void Manager::loop() {
  if (callConfigMode && configMode == 0) {
    configManager(config);
  } else if (!callConfigMode && configMode == 1) {
    quitConfigMode();
  }
}
