/// FILE MANAGER

void updateFile(const String fileName, int value) {
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

void loadConfig(const String fileName, Config &config) {
  aSerial.vvv().pln(F("Reading config file."));
  File configFile = SPIFFS.open(fileName, "r");
  if (configFile) {
    size_t size = configFile.size();
    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);
    configFile.readBytes(buf.get(), size);
    //  StaticJsonDocument<(objBufferSize * 2)> obj;
    DynamicJsonDocument obj(objBufferSize * 2);
    //  DeserializationError error = deserializeJson(obj, buf.get());
    DeserializationError error = deserializeJson(obj, buf.get(), size);
    if (error) {
      aSerial.vv().p(F("Failed to load json config : ")).pln(error.c_str());
    }
    else {
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
  }
  else {
    aSerial.vv().pln(F("Failed to load config file."));
  }
  ticker.detach();
}

void saveConfig(const String fileName, Config &config) {
  strlcpy(config.mqttTopicIn, config.devEui, sizeof(config.mqttTopicIn));
  strcat(config.mqttTopicIn, config.inPrefix);
  strlcpy(config.mqttTopicOut, config.devEui, sizeof(config.mqttTopicOut));
  strcat(config.mqttTopicOut, config.outPrefix);
  //  StaticJsonDocument<objBufferSize> obj;
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

void initDefaultConfig(const String fileName, Config &config) {
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

void tick() {
  int state = digitalRead(STATE_LED);
  digitalWrite(STATE_LED, !state);
}

void setPins() {
  pinMode(STATE_LED, OUTPUT);
  digitalWrite(STATE_LED, HIGH);
  pinMode(RELAY_SWITCH, OUTPUT);
  // todo: get switch value from config store
  digitalWrite(RELAY_SWITCH, LOW);
  debouncer.attach(OTA_BUTTON_PIN, INPUT_PULLUP);
  debouncer.interval(debouncerInterval);
  aSerial.vvv().pln(F("Pins set"));
}

void checkButton() {
  // Get the update value
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

void setReboot() { // Boot to sketch
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

void setDefault() {
  ticker.attach(2, tick);
  aSerial.v().println(F("====== Reset config ======"));
  resetConfig = false;
  SPIFFS.begin();
  delay(10);
  SPIFFS.format();
  wifiManager.resetSettings();
  delay(100);
  aSerial.v().println(F("====== System cleared ======"));
  ticker.detach();
  aSerial.v().pln(ESP.eraseConfig());
  setReboot();
}
