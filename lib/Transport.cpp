#include "Transport.h"

Transport::Transport(Client &client) {
  _client = client;
  //  init(config);
}

#if MQTT_CLIENT == 1
void Transport::init(Config &config, MQTT_CALLBACK_SIGNATURE) {
#if CLIENT_SECURE == 1
  aSerial.vvv().p(F("Init mqtts://"));
#else
  aSerial.vvv().p(F("Init mqtt://"));
#endif
  if (config.mqttServer && config.mqttPort) {
    aSerial.vvv().p((const char*)config.mqttServer).p(":").pln(atoi(config.mqttPort));
    _client.setServer(config.mqttServer, atoi(config.mqttPort));
    configured = true;
  }
  setCallback(callback);
}
#elif WEB_SERVER == 1
void Transport::init(Config &config, HTTP_CALLBACK_SIGNATURE) {
#if CLIENT_SECURE == 1
  aSerial.vvv().p(F("Init https://"));
#else
  aSerial.vvv().p(F("Init http://"));
#endif
  setCallback(httpCallback);
  server.begin();
  configured = true;
  aSerial.vvv().println(F("Web server started"));
}
#endif

void Transport::connect(Config &config) {
  // Loop until we're reconnected
  mqttFailCount = 0;
  helpers.startTick(0.3);
  while (!_client.connected()) {
    ++mqttFailCount;
#if CLIENT_SECURE == 1
    aSerial.vvv().p(F("Connecting to mqtts://"));
#else
    aSerial.vvv().p(F("Connecting to mqtt://"));
#endif
    aSerial.vvv().p((const char*)config.mqttServer).p(":").pln(atoi(config.mqttPort));
    helpers.generateId(config);
    if (_client.connect(((const char*)config.mqttClient), ((const char*)config.mqttUser), ((const char*)config.mqttPassword))) {
      aSerial.vvv().p(F("Connected to broker as : ")).pln((const char*)config.mqttClient);
      mqttFailCount = 0;
      helpers.stopTick();
      char masterTopic[60];
      strlcpy(masterTopic, config.mqttTopicIn, sizeof(masterTopic));
      strcat(masterTopic, "/+/+/+/+" );
      subscribe((const char*)masterTopic, 0);
      //  return aloes.presentSensors(config);
    } else {
      aSerial.vvv().p(F("Failed mqtt connection : ")).pln(_client.state());
      if (mqttFailCount > mqttMaxFailedCount && configMode == 0) {
        aSerial.vv().p(mqttMaxFailedCount).pln(F("+ MQTT connection failure --> config mode"));
        mqttFailCount = 0;
        callConfigMode = true;
        return;
        // return manager.configManager(config);
      }
      delay(reconnectInterval);
    }
  }
}

bool Transport::reconnect(Config &config) {
  if (configured) {
    _client.disconnect();
    mqttFailCount = 0;
#if CLIENT_SECURE == 1
    aSerial.vvv().p(F("Connecting to mqtts://"));
#else
    aSerial.vvv().p(F("Connecting to mqtt://"));
#endif
    aSerial.vvv().p((const char*)config.mqttServer).p(":").pln(atoi(config.mqttPort));
    for (int i = 0; i < mqttMaxFailedCount; i++) {
      if (!_client.connect(((const char*)config.mqttClient), ((const char*)config.mqttUser), ((const char*)config.mqttPassword)) ) {
        aSerial.vvv().p(F(" . "));
        delay(reconnectInterval);
      }
    }
    if (_client.connected()) {
      if (configMode == 1) {
        callConfigMode = false;
        //  manager.quitConfigMode();
      }
      return true;
    }
    return false;
  }
  return false;
}

bool Transport::connected() {
  return _client.connected();
}

bool Transport::publish(const char* topic, const char* payload, bool retained) {
  if (connected()) {
#if MQTT_CLIENT == 1
    if (_client.publish(topic, payload, retained)) {
      return true;
    }
#elif WEB_SERVER == 1
    // get content type from topic from omaObjectId
    String response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: image/jpeg\r\n";
    response += "Content-Length: " + String(len) + "\r\n\r\n";
    server.sendContent(response);
    if (_client.write(topic, payload)) {
      return true;
    }
#endif
    return false;
  } else {
    aSerial.vvv().pln(F("Not connected"));
    return false;
  }
}

bool Transport::subscribe(const char* topic, int qos) {
  if (!qos) {
    qos = 0;
  }
  bool substate = _client.subscribe(topic, qos);
  aSerial.vvv().p(F("Subscribing to ")).p(topic);
  if (substate) {
    aSerial.vvv().pln(" - success");
  } else {
    aSerial.vvv().pln(" - failure");
  }
  return (substate);
}

bool Transport::beginPublish(const char* topic, size_t length, bool retained) {
#if MQTT_CLIENT == 1
  if (_client.beginPublish(topic, length, retained)) {
    return true;
  }
#elif WEB_SERVER == 1
  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
  server.sendContent(response);
  return true;
#endif
  return false;
}

size_t Transport::write(const uint8_t *payload, size_t length) {
#if MQTT_CLIENT == 1
  _client.write(payload, length);
#elif WEB_SERVER == 1
  String response = "--frame\r\n";
  response += "Content-Type: image/jpeg\r\n\r\n";
  server.sendContent(response);
  _client.write(payload, length);
#endif
}

bool Transport::endPublish() {
#if MQTT_CLIENT == 1
  if (_client.endPublish()) {
    return true;
  }
  return false;
#elif WEB_SERVER == 1
  return true;
#endif
  return false;
}

#if MQTT_CLIENT == 1
void Transport::setCallback(MQTT_CALLBACK_SIGNATURE) {
  _client.setCallback(callback);
}
#elif WEB_SERVER == 1
void Transport::setCallback(HTTP_CALLBACK_SIGNATURE) {
  server.handleNotFound(httpCallback);
  // String message = "Server is running!\n\n";
  // message += "URI: ";
  // message += server.uri();
  // message += "\nMethod: ";
  // message += (server.method() == HTTP_GET) ? "GET" : "POST";
  // message += "\nArguments: ";
  // message += server.args();
  // message += "\n";
  // server.send(200, "text/plain", message);
  // if (server.hasArg("ql")) {
  //   aSerial.vvv().print(F("QL changed to : ")).println(server.arg("ql"));
  // }
}
#endif

void Transport::loop() {
#if MQTT_CLIENT == 1
  if (!connected()) {
    connect(config);
  }
  _client.loop();
#elif WEB_SERVER == 1
  server.handleClient();
#endif
}
