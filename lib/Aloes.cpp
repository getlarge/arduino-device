#include "Aloes.h"

Aloes::Aloes() {
  // get sensors count from arg
}

bool Aloes::init(void (*deviceUpdateCb)(), ERROR_CALLBACK_SIGNATURE) {
  device = new Device();
  device->setCallback(deviceUpdateCb);
  if (!device->init()) {
    return false;
  }
  if (!device->initSensors()) {
    return false;
  }
  // device->saveConfig();
  // device->loadConfig();

  _message.setup(device);
  client = new Transport(device);

  // to receive error messages
  device->setErrorCallback(onError);
  client->setErrorCallback(onError);
  return true;
}

void Aloes::setMsgCallback(MESSAGE_CALLBACK_SIGNATURE, MQTT_CALLBACK,
                           HTTP_CALLBACK) {
  this->onMessage = msgCallback;
  client->setMQTTCallback(mqttCallback);
  client->setHTTPCallback(httpCallback);
}

void Aloes::onDeviceUpdate() {
  _message.update(device);
  // delete client;
  client = new Transport(device);
}

// "pattern":
// "+prefixedDevEui/+method/+omaObjectId/+nodeId/+sensorId/+ipsoResourceId"
void Aloes::presentSensors() {
  for (unsigned int j = 1; j <= device->sensors->GetIndexCount(); ++j) {
    aSerial.vvv()
        .p(F("[ALOES] sensors res: "))
        .p(device->sensors->get(j, S_OBJECT_ID))
        .p(" ")
        .p(device->sensors->get(j, S_NODE_ID))
        .p(" ")
        .p(device->sensors->get(j, S_SENSOR_ID))
        .p(" ")
        .pln(device->sensors->get(j, S_RESOURCE_ID));
    setMsg(OBJECT_ID, device->sensors->get(j, S_OBJECT_ID));
    setMsg(NODE_ID, device->sensors->get(j, S_NODE_ID));
    setMsg(SENSOR_ID, device->sensors->get(j, S_SENSOR_ID));
    setMsg(RESOURCE_ID, device->sensors->get(j, S_RESOURCE_ID));
    setMsg(PAYLOAD, "presentation");
    setMsg(METHOD, "0");
    sendMessage(MQTT);
  }
}

char *Aloes::getDevice(DeviceKeys key) { return device->get(key); }

void Aloes::setDevice(DeviceKeys key, char *value) { device->set(key, value); }

Aloes &Aloes::setCnf(DeviceKeys key, char *value) {
  setDevice(key, value);
  return *this;
}

void Aloes::setPayload(uint8_t *payload, size_t length, const char *type) {
  _message.setPayload(payload, length, type);
}

char *Aloes::getMsg(MessageKeys key) { return _message.get(key); }

Aloes &Aloes::setMsg(MessageKeys key, char *value) {
  _message.set(key, value);
  return *this;
}

Aloes &Aloes::setMsg(MessageKeys key, const char *value) {
  _message.set(key, value);
  return *this;
}

Aloes &Aloes::setMsg(MessageKeys key, uint8_t *value, size_t length) {
  _message.set(key, value, length);
  return *this;
}

Aloes &Aloes::setMsg(MessageKeys key, int value) {
  String newValue = String(value);
  _message.set(key, newValue.c_str());
  return *this;
}

bool Aloes::sendMessage(transportLayer transportType) {
  if (transportType == MQTT) {
    char *topic = _message.fillTopic();
    char *payload = getMsg(PAYLOAD);
    aSerial.vvv()
        .p(F("[ALOES] sendMessage res: "))
        .p(topic)
        .p("  ")
        .pln(payload);
    if (client->publish((const char *)topic, payload, false)) {
      return true;
    }
    return false;
  } else if (transportType == HTTP) {
    const char *method = getMsg(METHOD);
    char *url = _message.fillUrl();
    char *payload = getMsg(PAYLOAD);
    aSerial.vvv().p(F("[ALOES] sendMessage res: ")).p(method).p(":").pln(url);
    if (client->setRequest(method, url, payload)) {
      return true;
    }
    return false;
  }
  return false;
}

bool Aloes::startStream(size_t length) {
  // char *topic;
  // _message.fillTopic(std::move(topic));
  const char *topic = _message.fillTopic();
  if (client->beginPublish(topic, length, false)) {
    return true;
  }
  return false;
}

size_t Aloes::writeStream(const uint8_t *payload, size_t length) {
  client->write(payload, length);
}

bool Aloes::endStream() {
  if (client->endPublish()) {
    return true;
  }
  return false;
}

// void remove_elt(char *str, int elem, size_t size) {
//     if ( elem < --size ) {
//         memmove( &str[elem], &str[elem + 1], size - elem );
//     }
// }

void removeElt(char *str, int i) {
  for (; str[i]; i++)
    str[i] = str[i + 1];
}

bool Aloes::parseUrl(char *url) {
  // pattern = "apiRoot/+collection/+path/#param"
  aSerial.vvvv().p(F("[ALOES] parseUrl : ")).pln(url);
  char *str, *p;
  uint8_t i = 0;
  char *pch;
  pch = strstr(url, device->get(HTTP_API_ROOT));
  if (strcmp(url, pch) != 0) {
    aSerial.vvvv().pln(F("[ALOES] parseUrl:error "));
    return false;
  }
  for (size_t i = 0; i < strlen(device->get(HTTP_API_ROOT)); i += 1) {
    removeElt(url, 0);
  }
  for (str = strtok_r(url, "/", &p); str && i < 3;
       str = strtok_r(NULL, "/", &p)) {
    switch (i) {
    case 0: {
      _message.set(COLLECTION, str);
      break;
    }
    case 1: {
      _message.set(PATH, str);
      break;
    }
    case 2: {
      _message.set(PARAM, str);
      break;
    }
    }
    i++;
  }
  return true;
}

bool Aloes::parseTopic(char *topic) {
  // pattern = "prefixedDevEui/+method/+objectId/+nodeId/+sensorId/+resourceId"
  char *str, *p;
  uint8_t i = 0;
  aSerial.vvvv().p(F("[ALOES] parseTopic : ")).pln(topic);
  char *pch;
  pch = strstr(topic, device->get(MQTT_TOPIC_IN));
  if (strcmp(topic, pch) != 0) {
    aSerial.vvvv().pln(F("[ALOES] parseTopic:error "));
    return false;
  }
  for (size_t i = 0; i < strlen(device->get(MQTT_TOPIC_IN)); i += 1) {
    removeElt(topic, 0);
  }
  for (str = strtok_r(topic, "/", &p); str && i < 5;
       str = strtok_r(NULL, "/", &p)) {
    switch (i) {
    case 0: {
      _message.set(METHOD, str);
      break;
    }
    case 1: {
      _message.set(OBJECT_ID, str);
      break;
    }
    case 2: {
      _message.set(NODE_ID, str);
      break;
    }
    case 3: {
      _message.set(SENSOR_ID, str);
      break;
    }
    case 4: {
      _message.set(RESOURCE_ID, str);
      break;
    }
    }
    i++;
  }
  return true;
}

bool Aloes::parseRoute(transportLayer transportType, char *route) {
  if (transportType == HTTP) {
    return parseUrl(route);
  } else if (transportType == MQTT) {
    return parseTopic(route);
  }
  return false;
}

bool Aloes::parseBody(uint8_t *body, size_t length) {
  aSerial.vvvv().p(F("[ALOES] parseBody : ")).pln(length);
  //  setMsg(PAYLOAD, body, length);
  _message.set(PAYLOAD, body, length);
  if (strcmp(_message.get(COLLECTION), "Devices") == 0 &&
      strcmp(_message.get(PATH), "get-state") == 0) {
    // if (device.setInstance(body, length)) {
    if (device->setInstance((const char *)_message.get(PAYLOAD))) {
      if (!stateReceived) {
        stateReceived = true;
      }
    }
    return true;
  }
  if (onMessage != 0) {
    onMessage(HTTP, &_message);
  }
  return true;
}

bool Aloes::parsePayload(uint8_t *payload, size_t length) {
  aSerial.vvvv().p(F("[ALOES] parsePayload : ")).pln(length);
  bool foundSensor = false;
  for (unsigned int j = 1; j <= device->sensors->GetIndexCount(); ++j) {
    int objectId = atoi(_message.get(OBJECT_ID));
    int nodeId = atoi(_message.get(NODE_ID));
    int sensorId = atoi(_message.get(SENSOR_ID));
    // fix this !
    aSerial.vvv()
        .p(F("[ALOES] parsePayload res: "))
        .p(device->sensors->get(j, S_OBJECT_ID))
        .p(" | ")
        .p(objectId)
        .p(" --- ")
        .p(device->sensors->get(j, S_NODE_ID))
        .p(" | ")
        .p(nodeId)
        .p(" --- ")
        .p(device->sensors->get(j, S_SENSOR_ID))
        .p(" | ")
        .pln(sensorId);
    if (objectId == device->sensors->get(j, S_OBJECT_ID) &&
        nodeId == device->sensors->get(j, S_NODE_ID) &&
        sensorId == device->sensors->get(j, S_SENSOR_ID)) {
      foundSensor = true;
      break;
    }
  }
  if (foundSensor) {
    // setMsg(PAYLOAD, payload);
    setMsg(PAYLOAD, payload, length);
    if (onMessage != 0) {
      onMessage(MQTT, &_message);
    }
    return true;
  }
  return false;
}

bool Aloes::parseMessage(transportLayer transportType, uint8_t *message,
                         size_t length) {
  if (transportType == HTTP) {
    return parseBody(message, length);
  } else if (transportType == MQTT) {
    return parsePayload(message, length);
  }
  return false;
}

bool Aloes::getState() {
  const char *deviceId = device->get(DEVICE_ID);
  _message.set(METHOD, "2")
      .set(COLLECTION, "Devices")
      .set(PATH, "get-state")
      .set(PARAM, deviceId)
      .set(PAYLOAD, "");
  if (sendMessage(HTTP)) {
    aSerial.vvv().p(F("[ALOES] getState res: ")).pln("success");
    return true;
  }
  aSerial.vvv().p(F("[ALOES] getState res: ")).pln("error");
  return false;
}

// void Aloes::authenticate() {

// }

void Aloes::getFirmwareUpdate() {
  // implement safety routine with user defined callback ?

  // const char* url = "/api/Devices/get-ota-update/";
  _message.set(COLLECTION, "Devices")
      .set(PATH, "get-ota-update")
      .set(PARAM, device->get(DEVICE_ID));
  const char *url = _message.fillUrl();
  aSerial.vvv().p(F("[ALOES] getFirmwareUpdate filePath : ")).pln(url);
  //  manager.getUpdated(0, (const char*)url);
  client->getUpdated(0, (const char *)device->get(HTTP_HOST),
                     atoi(device->get(HTTP_PORT)), url);
}
