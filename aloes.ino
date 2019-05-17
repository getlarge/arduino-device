void getDeviceId(Config &config) {
#if ID_TYPE == 0
  char *espChipId;
  float chipId = ESP.getChipId();
  char chipIdBuffer[sizeof(chipId)];
  espChipId = dtostrf(chipId, sizeof(chipId), 0, chipIdBuffer);
  strcpy(config.devEui, espChipId);
#endif
#if ID_TYPE == 1
  String macAdress = WiFi.macAddress();
  char macAdressBuffer[20];
  macAdress.toCharArray(macAdressBuffer, 20);
  // next => remove the ":" in the mac adress
  strcpy(config.devEui, macAdressBuffer);
#endif
  //    #if ID_TYPE == 2
  //// soyons fous, let's create an eui64 address ( like ipv6 )
  ////      Step #1: Split the MAC address in the middle:
  ////      Step #2: Insert FF:FE in the middle:
  ////      Step #4: Convert the first eight bits to binary:
  ////      Step #5: Flip the 7th bit:
  ////      Step #6: Convert these first eight bits back into hex:
  //    #endif
  aSerial.vvv().p(F("DeviceID : ")).pln(config.devEui);
  generateMqttClientId(config);
}

void setSensorRoutes(Config &config, const char* objectId, const char* sensorId, const char* resourceId, size_t index) {
  // "pattern": "+prefixedDevEui/+method/+omaObjectId/+sensorId/+ipsoResourceId",
  //  char topic[100];
  char topic[150];
  strcpy(topic, config.mqttTopicOut);
  strcat(topic, "/1/" );
  strcat(topic, objectId );
  strcat(topic, "/" );
  strcat(topic, sensorId );
  strcat(topic, "/" );
  strcat(topic, resourceId );
  aSerial.vvvv().p(F("set topic : ")).pln(topic);
  postTopics[index] = String(topic);
}

void setSensors(Config &config) {
  size_t i = 0;
  while (i < sizeof(sensors) / sizeof(sensors[0])) {
    //  for (size_t i = 0; i < sizeof(sensors) / sizeof(sensors[0]); i++) {
    const char** sensor = sensors[i];
    char objectId[5];
    char sensorId[4];
    char resourceId[5];
    //  printf("sensor[%u]\n", i);
    for (size_t j = 0; sensor[j]; j++) {
      if (j == 0 ) {
        strlcpy(objectId, sensor[j], sizeof(objectId));
      } else if (j == 1 ) {
        strlcpy(sensorId, sensor[j], sizeof(sensorId));
      }  else if (j == 2 ) {
        strlcpy(resourceId, sensor[j], sizeof(resourceId));
      }
      //  printf("  [%s]\n", sensor[j]);
    }
    setSensorRoutes(config, (const char*)objectId, (const char*)sensorId, (const char*)resourceId, i);
    i++;
  }
}

void presentSensors(Config &config) {
  for (size_t i = 0; i < sizeof(sensors) / sizeof(sensors[0]); i++) {
    const char** sensor = sensors[i];
    char objectId[5];
    char sensorId[4];
    char resourceId[5];
    char payload[30];
    //  printf("sensor[%u]\n", i);
    for (size_t j = 0; sensor[j]; j++) {
      if (j == 0 ) {
        strlcpy(objectId, sensor[j], sizeof(objectId));
      } else if (j == 1 ) {
        strlcpy(sensorId, sensor[j], sizeof(sensorId));
      }  else if (j == 2 ) {
        strlcpy(resourceId, sensor[j], sizeof(resourceId));
      } else if (j == 3 ) {
        strlcpy(payload, sensor[j], sizeof(payload));
      }
      //  printf("  [%s]\n", sensor[j]);
    }
    char presentationTopic[70];
    strlcpy(presentationTopic, config.mqttTopicOut, sizeof(presentationTopic));
    strcat(presentationTopic, "/0/" );
    strcat(presentationTopic, objectId );
    strcat(presentationTopic, "/" );
    strcat(presentationTopic, sensorId );
    strcat(presentationTopic, "/" );
    strcat(presentationTopic, resourceId );
    mqttClient.publish((const char*)presentationTopic, payload);
  }
}

void setMessage(Message &message, char method[5], char* objectId, char* sensorId, char* resourceId, char* payload ) {
  strlcpy(message.method, method, sizeof(method));
  strlcpy(message.omaObjectId, objectId, sizeof(objectId));
  strlcpy(message.sensorId, sensorId, sizeof(sensorId));
  strlcpy(message.omaResourceId, resourceId, sizeof(resourceId));
  strlcpy(message.payload, payload, sizeof(payload));
}

void sendMessage(Config &config, Message &message ) {
  char topic[70];
  strlcpy(topic, config.mqttTopicOut, sizeof(config.mqttTopicOut));
  strcat(topic, "/" );
  strcat(topic, message.method );
  strcat(topic, "/" );
  strcat(topic, message.omaObjectId );
  strcat(topic, "/" );
  strcat(topic, message.sensorId );
  strcat(topic, "/" );
  strcat(topic, message.omaResourceId );
  mqttClient.publish((const char*)topic, message.payload);
}

void parseMessage(Message &message) {
  for (size_t i = 0; i < sizeof(sensors) / sizeof(sensors[0]); i++) {
    const char** sensor = sensors[i];
    char objectId[5];
    char sensorId[4];
    char resourceId[5];
    //  printf("sensor[%u]\n", i);
    for (size_t j = 0; sensor[j]; j++) {
      if (j == 0 ) {
        strlcpy(objectId, sensor[j], sizeof(objectId));
      } else if (j == 1 ) {
        strlcpy(sensorId, sensor[j], sizeof(sensorId));
      }  else if (j == 2 ) {
        strlcpy(resourceId, sensor[j], sizeof(resourceId));
      }
    }
    if ( strcmp(message.sensorId, sensorId) == 0) {
      return onMessage(message, sensorId);
    }
  }
}

void parseTopic(Message &message, char* topic) {
  char *str, *p;
  uint8_t i = 0;

  // first sanity check
  //  if (topic != strstr(topic, config.mqttTopicIn)) {
  //    Serial.print("error in the protocol");
  //    return;
  //  }

  // "pattern": "+prefixedDevEui/+method/+omaObjectId/+sensorId/+omaResourcesId",
  for (str = strtok_r(topic + 1, "/", &p); str && i <= 4;
       str = strtok_r(NULL, "/", &p)) {
    switch (i) {
      case 0: {
          //prefixedDevEui
          break;
        }
      case 1: {
          strlcpy(message.method, str, sizeof(message.method));
          break;
        }
      case 2: {
          strlcpy(message.omaObjectId, str, sizeof(message.omaObjectId));
          break;
        }
      case 3: {
          strlcpy(message.sensorId, str, sizeof(message.sensorId));
          break;
        }
      case 4: {
          strlcpy(message.omaResourceId, str, sizeof(message.omaResourceId));
          break;
        }
    }
    i++;
  }
}

void deviceInit() {
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
  ticker.attach(1.5, tick);
  if (wifiResetConfig) { // add a button ?
    WiFiManager wifiManager;
    wifiManager.resetSettings();
  }
  if (resetConfig) {
    setDefault();
  }
  randomSeed(micros());
  getDeviceId(config);
  aSerial.vvv().print(F("before heap size : ")).println( ESP.getFreeHeap());
}

void deviceSetup() {
  deviceInit();
  aSerial.v().println(F("====== Setup started ======"));
  beforeSetup();
  aSerial.vvv().pln(F("mounting FS..."));
  if (SPIFFS.begin()) {
    aSerial.vvv().pln(F("FS mounted"));
    if (SPIFFS.exists(configFileName)) {
      loadConfig(configFileName, config);
    } else {
      generateMqttClientId(config);
      initDefaultConfig(configFileName, config);
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

  setPins();
  setSensors(config);
  ticker.attach(1.5, tick);
  connectWifi(config);
#if MQTT_CLIENT == 1
  mqttInit(config);
#endif
  initConfigManager(config);
#if NTP_SERVER == 1
  aSerial.vvv().println(F("Starting UDP"));
  Udp.begin(localPort);
  aSerial.vvv().print(F("Local port : ")).println(Udp.localPort());
  aSerial.vvvv().println(F("Waiting for sync "));
  setSyncProvider(getNtpTime);
  setSyncInterval(300);
  //  delay(100);
#endif
#if WEB_SERVER == 1
  //  server.onNotFound(handleNotFound);
  server.begin();
  aSerial.vvv().println(F("Web server started"));
#endif
  aSerial.vvv().print(F("setup heap size : ")).println(ESP.getFreeHeap());
  afterSetup();
  digitalWrite(STATE_LED, HIGH);
  ticker.detach();
  aSerial.v().println(F("====== Setup ended ======"));
}

void deviceLoop() {
  beforeLoop();
#if MQTT_CLIENT == 1
  if (!mqttClient.connected()) {
    mqttReconnect(config);
  }
  //ticker.detach();
  mqttClient.loop();
#endif
#if WEB_SERVER == 1
  server.handleClient();
#endif

#if NTP_SERVER == 1
  if (timeStatus() != timeNotSet) {
    if (now() != prevDisplay) { //update the display only if time has changed
      prevDisplay = now();
      digitalClockDisplay();
    }
  }
#endif
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
        return configManager(config);
      }
    }
  }
  afterLoop();
}
