#include "Message.h"

Message::Message() {}

void Message::setup(Device *device) {
  strlcpy(_mqttPrefix, device->get(MQTT_TOPIC_OUT), sizeof(_mqttPrefix) - 1);
  strlcpy(_httpPrefix, device->get(HTTP_API_ROOT), sizeof(_httpPrefix) - 1);
}

void Message::update(Device *device) {
  strlcpy(_mqttPrefix, device->get(MQTT_TOPIC_OUT), sizeof(_mqttPrefix) - 1);
  strlcpy(_httpPrefix, device->get(HTTP_API_ROOT), sizeof(_httpPrefix) - 1);
  // aSerial.vvvv().p(F("[MESSAGE] set _httpPrefix : ")).pln(_httpPrefix);
  // aSerial.vvvv().p(F("[MESSAGE] set _mqttPrefix : ")).pln(_mqttPrefix);
}

char *Message::getMethod() { return std::move(_method); }

void Message::setMethod(char method[5]) {
  strlcpy(_method, method, sizeof(_method) - 1);
}

char *Message::getObjectId() { return _objectId; }

void Message::setObjectId(char *objectId) {
  strlcpy(_objectId, objectId, sizeof(_objectId) - 1);
}

char *Message::getNodeId() { return _nodeId; }

void Message::setNodeId(char *nodeId) {
  strlcpy(_nodeId, nodeId, sizeof(_nodeId) - 1);
}

char *Message::getSensorId() { return _sensorId; }

void Message::setSensorId(char *sensorId) {
  strlcpy(_sensorId, sensorId, sizeof(_sensorId) - 1);
}

char *Message::getResourceId() { return _resourceId; }

void Message::setResourceId(char *resourceId) {
  strlcpy(_resourceId, resourceId, sizeof(_resourceId) - 1);
}

char *Message::getCollection() { return _collection; }

void Message::setCollection(char *collection) {
  strlcpy(_collection, collection, sizeof(_collection) - 1);
}

char *Message::getPath() { return std::move(_path); }

void Message::setPath(char *path) { strlcpy(_path, path, sizeof(_path) - 1); }

char *Message::getParam() { return std::move(_param); }

void Message::setParam(char *param) {
  strlcpy(_param, param, sizeof(_param) - 1);
}

char *Message::getPayload() { return std::move(_payload); }

void Message::setPayload(char *payload) {
  int x = 0;
  _payload = new char[strlen(payload) + 1];
  do {
    _payload[x] = payload[x];
  } while (payload[x++] != '\0');
  //  aSerial.vvvv().p(F("[MESSAGE] setPayload: ")).p(sizeof((const
  //  char*)payload)).p(" ").pln(_payload);
}

void Message::setPayload(uint8_t *payload, size_t length) {
  int x = 0;
  payload[length] = '\0';
  _payload = new char[length + 1];
  do {
    _payload[x] = payload[x];
  } while (payload[x++] != '\0');
  //  _payload = (char*)payload;
}

void Message::setPayload(uint8_t *payload, size_t length, const char *type) {
  payload[length] = '\0';
  _payload = new char[length * 3 + 1];
  //  _payload = new char[length * 2 + 1];
  for (size_t i = 0; i < length; i++) {
    // sprintf(&payload[2 * i], "%s",( i + 1 ) % 16 == 0 ? "\r\n" : " " );
    if (type == "HEX") {
      sprintf(&_payload[2 * i], "%02X", payload[i]);
    } else if (type == "DEC") {
      sprintf(&_payload[3 * i], "%d", payload[i]);
    }
  }
  //  strcpy(message.payload, payload);
}

char *Message::getTopic() { return std::move(_topic); }

void Message::setTopic(char *topic) {
  //  aSerial.vvvv().p(F("[MESSAGE] setTopic: ")).pln(topic);
  strlcpy(_topic, topic, sizeof(_topic) - 1);
}

char *Message::fillTopic() {
  char topic[200];
  strcpy(topic, _mqttPrefix);
  strcat(topic, "/");
  strcat(topic, getMethod());
  strcat(topic, "/");
  strcat(topic, getObjectId());
  strcat(topic, "/");
  strcat(topic, getNodeId());
  strcat(topic, "/");
  strcat(topic, getSensorId());
  strcat(topic, "/");
  strcat(topic, getResourceId());
  setTopic(topic);
  return std::move(_topic);
}

void Message::fillTopic(char *&&topic) {
  topic = new char[200];
  strcpy(topic, _mqttPrefix);
  strcat(topic, "/");
  strcat(topic, getMethod());
  strcat(topic, "/");
  strcat(topic, getObjectId());
  strcat(topic, "/");
  strcat(topic, getNodeId());
  strcat(topic, "/");
  strcat(topic, getSensorId());
  strcat(topic, "/");
  strcat(topic, getResourceId());
  setTopic(topic);
}

char *Message::getUrl() { return std::move(_url); }

void Message::setUrl(char *url) { strlcpy(_url, url, sizeof(_url) - 1); }

char *Message::fillUrl() {
  char url[200];
  strcpy(url, _httpPrefix);
  strcat(url, getCollection());
  strcat(url, "/");
  strcat(url, getPath());
  strcat(url, "/");
  strcat(url, getParam());
  setUrl(url);
  return std::move(_url);
}

void Message::fillUrl(char *&&url) {
  url = new char[200];
  strcpy(url, _httpPrefix);
  strcat(url, getCollection());
  strcat(url, "/");
  strcat(url, getPath());
  strcat(url, "/");
  strcat(url, getParam());
  setUrl(url);
}

char *Message::get(MessageKeys key) {
  char *value;
  // todo regexep on key
  switch (key) {
  case METHOD: {
    value = getMethod();
    break;
  }
  case OBJECT_ID: {
    value = getObjectId();
    break;
  }
  case NODE_ID: {
    value = getNodeId();
    break;
  }
  case SENSOR_ID: {
    value = getSensorId();
    break;
  }
  case RESOURCE_ID: {
    value = getResourceId();
    break;
  }
  case TOPIC: {
    value = getTopic();
    break;
  }
  case PAYLOAD: {
    value = getPayload();
    break;
  }
  case COLLECTION: {
    value = getCollection();
    break;
  }
  case PATH: {
    value = getPath();
    break;
  }
  case PARAM: {
    value = getParam();
    break;
  }
  case URL: {
    value = getUrl();
    break;
  }
  default: {
    // setError("")
  }
  }
  return std::move(value);
}

void Message::setKV(MessageKeys key, char *value) {
  switch (key) {
  case METHOD: {
    setMethod(value);
    break;
  }
  case OBJECT_ID: {
    setObjectId(value);
    break;
  }
  case NODE_ID: {
    setNodeId(value);
    break;
  }
  case SENSOR_ID: {
    setSensorId(value);
    break;
  }
  case RESOURCE_ID: {
    setResourceId(value);
    break;
  }
  case TOPIC: {
    setTopic(value);
    break;
  }
  case PAYLOAD: {
    setPayload(value);
    break;
  }
  case COLLECTION: {
    setCollection(value);
    break;
  }
  case PATH: {
    setPath(value);
    break;
  }
  case PARAM: {
    setParam(value);
    break;
  }
  case URL: {
    setUrl(value);
    break;
  }
  default: {
    // setError("")
  }
  }
}

Message &Message::set(MessageKeys key, char *value) {
  setKV(key, value);
  return *this;
}

Message &Message::set(MessageKeys key, const char *value) {
  setKV(key, (char *)value);
  return *this;
}

Message &Message::set(MessageKeys key, uint8_t *value, size_t length) {
  value[length] = '\0';
  // char *newValue;
  // newValue = (char*)value;
  // int x = 0;
  char *newValue = new char[length + 1];
  // do {
  //   newValue[x] = value[x];
  // } while (value[x++] != '\0');
  strlcpy(newValue, (char *)value, length + 1);
  aSerial.vvvv()
      .p(F("[MESSAGE] setMessage res: "))
      .pln(newValue)
      .p(F(" length: "))
      .pln(strlen(newValue));
  setKV(key, newValue);
  return *this;
}