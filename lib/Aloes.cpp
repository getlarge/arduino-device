#include "lib/Message.cpp"

#include "Aloes.h"

Aloes::Aloes() {
  Message _message; 
  // _message = message;
  // get sensor count from arg
  //  Sensor sensors[sensorCount];
}

Aloes::Aloes(Aloes &aloes) {

}

bool Aloes::initDevice(DEVICE_CALLBACK_SIGNATURE) {
  if (!device.init()) {
    return false;
  }
  device.setCallback(onDeviceUpdate);
  _message.setup(device);
  return true;
}

void Aloes::setMsgCallback(MESSAGE_CALLBACK_SIGNATURE) {
  this->msgCallback = msgCallback;
}

void Aloes::onDeviceUpdate() {
  _message.update(device);
  _transport.update(device);
}


// "pattern": "+prefixedDevEui/+method/+omaObjectId/+sensorId/+ipsoResourceId",
void Aloes::initSensors() {
  for (size_t i = 0; i < sizeof(sensors) / sizeof(sensors[0]); i++) {
    const char** sensor = sensors[i];
    for (size_t j = 0; sensor[j]; j++) {
      if (j == 0 ) {
        setMsg(OBJECT_ID, sensor[j]);
      } else if (j == 1 ) {
        setMsg(SENSOR_ID, sensor[j]);
      }  else if (j == 2 ) {
        setMsg(RESOURCE_ID, sensor[j]);
      }
    }
    setMsg(METHOD, "1");
    char *topic = _message.fillTopic();
    //  postTopics[index] = String(topic);
  }
}

void Aloes::presentSensors() {
  for (size_t i = 0; i < sizeof(sensors) / sizeof(sensors[0]); i++) {
    const char** sensor = sensors[i];
    for (size_t j = 0; sensor[j]; j++) {
      if (j == 0 ) {
        setMsg(OBJECT_ID, sensor[j]);
      } else if (j == 1 ) {
        setMsg(SENSOR_ID, sensor[j]);
      }  else if (j == 2 ) {
        setMsg(RESOURCE_ID, sensor[j]);
      } else if (j == 3 ) {
        setMsg(PAYLOAD, sensor[j]);
      }
    }
    setMsg(METHOD, "0");
    sendMessage(MQTT);
  }
}

char* Aloes::getConfig(DeviceKeys deviceKey) {
  return device.get(deviceKey);
}

void Aloes::setConfig(DeviceKeys deviceKey, char *value) {
  device.set(deviceKey, value);
}

Aloes& Aloes::setCnf(DeviceKeys deviceKey, char* value) {
  setConfig(deviceKey, value);
  return *this;  
}

void Aloes::setPayload(uint8_t *payload, size_t length, const char* type) {
  _message.setPayload(payload, length, type);
}

char* Aloes::getMsg(MessageKeys messageKey) {
  return _message.get(messageKey);
}

Aloes& Aloes::setMsg(MessageKeys messageKey, char* value) {
  _message.set(messageKey, value);
  return *this;  
}

Aloes& Aloes::setMsg(MessageKeys messageKey, const char* value) {
  _message.set(messageKey, value);
  return *this;  
}

Aloes& Aloes::setMsg(MessageKeys messageKey, uint8_t* value, size_t length) {
  _message.set(messageKey, value, length);
  return *this;  
}

bool Aloes::sendMessage(transportLayer transportType) {
  if (transportType == MQTT) {
    char *topic = _message.fillTopic();
    char *payload = getMsg(PAYLOAD);
    aSerial.vvv().p(F("[ALOES] sendMessage res: ")).p(topic).p("  ").pln(payload);
    if (_transport.publish((const char*)topic, payload, false)) {
      return true;
    }
    return false;
  } else if (transportType == HTTP) {
    const char *method = getMsg(METHOD);
    char *url = _message.fillUrl();
    char *payload = getMsg(PAYLOAD);
    aSerial.vvv().p(F("[ALOES] sendMessage res: ")).p(method).p("/").pln(url);
    if (_transport.setRequest(method, url, payload)) {
      return true;
    }
    return false;
  }
  return false;
}

// bool Aloes::sendMessage(Config &config, size_t length) {
//   char *topic = _message.setTopic(config);
//   aSerial.vvv().p(F("sendMessage : ")).p(topic).p("  ").pln(_message.payload);
//   //if (transport.publish((const char*)topic, _message.payload, false)) {
//   if (transport.publish((const char*)topic, (const uint8_t*)_message.payload, length*2, false)) {
//     return true;
//   }
//   return false;
// }

bool Aloes::startStream(size_t length) {
  // char *topic; 
  // _message.fillTopic(std::move(topic));
  const char *topic = _message.fillTopic();
  if (_transport.beginPublish(topic, length, false)) {
    return true;
  }
  return false;
}

size_t Aloes::writeStream(const uint8_t *payload, size_t length) {
  _transport.write(payload, length);
}

bool Aloes::endStream() {
  if (_transport.endPublish()) {
    return true;
  }
  return false;
}

bool Aloes::parseTopic(char* topic) {
  char *str, *p;
  uint8_t i = 0;
  // first sanity check
   if (topic != strstr(topic, device.get(MQTT_TOPIC_IN))) {
     Serial.print("error in the protocol");
     return false;
   }

  // pattern = "prefixedDevEui/+method/+objectId/+sensorId/+resourceId",
  for (str = strtok_r(topic + 1, "/", &p); str && i <= 4; str = strtok_r(NULL, "/", &p)) {
    switch (i) {
      case 0: {
        // prefixedDevEui
        break;
      }
      case 1: {
        _message.set(METHOD, str);
        break;
      }
      case 2: {
        _message.set(OBJECT_ID, str);
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

bool Aloes::parseMessage(char *payload) {
  bool foundSensor = false;
  for (size_t i = 0; i < sizeof(sensors) / sizeof(sensors[0]); i++) {
    const char** sensor = sensors[i];
    char objectId[5];
    char sensorId[4];
    for (size_t j = 0; sensor[j]; j++) {
      if (j == 0 ) {
        strlcpy(objectId, sensor[j], sizeof(objectId));
      } else if (j == 1 ) {
        strlcpy(sensorId, sensor[j], sizeof(sensorId));
      }
    }
    if (strcmp(_message.get(OBJECT_ID), objectId) == 0 && strcmp(_message.get(SENSOR_ID), sensorId) == 0) {
      foundSensor = true;
    }
  }
  if (foundSensor) {
    _message.set(PAYLOAD, payload);
     msgCallback(MQTT, &_message);
     return true;
  }
  return false;
}

bool Aloes::parseMessage(uint8_t *payload, size_t length) {
  bool foundSensor = false;
  for (size_t i = 0; i < sizeof(sensors) / sizeof(sensors[0]); i++) {
    const char** sensor = sensors[i];
    char objectId[5];
    char sensorId[4];
    for (size_t j = 0; sensor[j]; j++) {
       if (j == 0 ) {
        strlcpy(objectId, sensor[j], sizeof(objectId));
      } else if (j == 1 ) {
        strlcpy(sensorId, sensor[j], sizeof(sensorId));
      }
    }
    if (strcmp(_message.get(OBJECT_ID), objectId) == 0 && strcmp(_message.get(SENSOR_ID), sensorId) == 0) {
      foundSensor = true;
    }
  }
  if (foundSensor) {
    setMsg(PAYLOAD, payload, length);
    msgCallback(MQTT, &_message);
    return true;
  }
  return false;
}

bool Aloes::parseUrl(char *url) {
  char *str, *p;
  uint8_t i = 0;
  // pattern = "apiRoot/+collection/+path/#param"
  if (url != strstr(url, device.get(HTTP_API_ROOT))) {
    //  Serial.print("error in the protocol");
    return false;
  }

  for (str = strtok_r(url + 1, "/", &p); str && i <= 3; str = strtok_r(NULL, "/", &p)) {
    switch (i) {
      case 0: {
        // apiRoot
        break;
      }
      case 1: {
        _message.set(COLLECTION, str);
        break;
      }
      case 2: {
        _message.set(PATH, str);
        break;
      }
      case 3: {
        _message.set(PARAM, str);
        break;
      }
    }
    i++;
  }
  return true;
}

void Aloes::parseBody(char *body) {
  _message.set(PAYLOAD, body);
  if (strcmp(_message.get(COLLECTION), "Devices") == 0) {
    //  if (strcmp(path, "get-state") == 0) { }
    if (device.setInstance(body)) {
      if (!stateReceived) {
        stateReceived = true;
      }
    } else {
     //  aSerial.vvvv().pln(F("[ALOES] device setting failed: ")).pln(device._deviceError);
    }
  }
  msgCallback(HTTP, &_message);
}

void Aloes::parseBody(uint8_t *body, size_t length) {
  //  const char* path = _message.getPath();
  setMsg(PAYLOAD, body, length);
  if (strcmp(_message.get(COLLECTION), "Devices") == 0) {
    if (device.setInstance(body, length)) {
      if (!stateReceived) {
        stateReceived = true;
      }
    } else {
     //  aSerial.vvvv().pln(F("[ALOES] device setting failed: ")).pln(device._deviceError);
    }
  }

  aSerial.vvv().p(F("[ALOES] parseBody : ")).pln(_message.get(PAYLOAD));
  msgCallback(HTTP, &_message);
}

void Aloes::parseBody(Stream *stream) {
  //  const char* path = _message.getPath();
  if (strcmp(_message.get(COLLECTION), "Devices") == 0) {
  }
  msgCallback(HTTP, &_message);
}

bool Aloes::getState() {
  const char* deviceId = device.get(DEVICE_ID);
  _message.set(METHOD, "2").set(COLLECTION, "Devices").set(PATH, "get-state").set(PARAM, deviceId).set(PAYLOAD, "");
  if (sendMessage(HTTP)) {
    aSerial.vvv().p(F("[ALOES] getState res: ")).pln("success");
    return true;
  } 
  aSerial.vvv().p(F("[ALOES] getState res: ")).pln("error");
  return false;
}

// void Aloes::setSensors(uint8_t *buffer, size_t length) {
//   // use arduinojson.org/v6/assistant
//   // int objectSize = 6;
//   // int bufferSize = 1024; // 180
//   // const size_t capacity = JSON_OBJECT_SIZE(objectSize) + bufferSize;
//   DynamicJsonDocument doc(1024);
//   DeserializationError error = deserializeJson(doc, buffer, length);
//   if (error) {
//     aSerial.vvvv().pln(F("[ALOES] deserializeJson() failed: ")).pln(error.c_str());
//     return;
//   }
//   JsonArray array=doc.as<JsonArray>();
//   // const char* name = obj["name"]; 
//   // if(name != nullptr) {
//   //   // copy in Device
//   // }
// }

// void Aloes::authenticate() {

// }

void Aloes::getFirmwareUpdate() {
  // implement safety routine with user defined callback ?
  
  // const char* url = "/api/Devices/get-ota-update/";
  _message.set(COLLECTION, "Devices").set(PATH, "get-ota-update").set(PARAM, device.get(DEVICE_ID));
  const char *url = _message.fillUrl();
  aSerial.vvv().p(F("[ALOES] getFirmwareUpdate filePath : ")).pln(url);
  //  manager.getUpdated(0, (const char*)url);
  _transport.getUpdated(0, (const char*)device.get(HTTP_HOST), atoi(device.get(HTTP_PORT)), url);
}
