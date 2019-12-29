#include "Device.h"

bool Device::reportErrors = false;

inline const char *const BoolToChar(bool b) { return b ? "true" : "false"; }

Device::Device() {

// #ifdef DEFAULT_DEVICE_NAME
//   setName((char*)DEFAULT_DEVICE_NAME);
// #endif
#ifdef DEFAULT_DEVICE_PASS
  setPass((char *)DEFAULT_DEVICE_PASS);
#endif
#ifdef DEFAULT_MQTT_TOPIC_PREFIX_IN
  setMQTTPrefixIn((char *)DEFAULT_MQTT_TOPIC_PREFIX_IN);
#else
  setMQTTPrefixIn((char *)"-in");
#endif
#ifdef DEFAULT_MQTT_TOPIC_PREFIX_OUT
  setMQTTPrefixOut((char *)DEFAULT_MQTT_TOPIC_PREFIX_OUT);
#else
  setMQTTPrefixOut((char *)"-out");
#endif

#ifdef DEFAULT_HTTP_API_ROOT
  setHTTPApiRoot((char *)DEFAULT_HTTP_API_ROOT);
#else
  setHTTPApiRoot((char *)"/api/");
#endif
  generateId();
}

// void Device::setCallback(void (*callback)()) {
//   this->onDeviceUpdate = callback;
// }

#if defined(ESP8266) || defined(ESP32)
#include <functional>
void Device::setCallback(std::function<void()> callback) {
  this->onDeviceUpdate = callback;
}

#else
void Device::setCallback(void (*callback)()) {
  this->onDeviceUpdate = callback;
}
#endif

void Device::setErrorCallback(ERROR_CALLBACK_SIGNATURE) {
  reportErrors = true;
  this->errorCallback = onError;
}

void Device::setError(char *error) {
  _error = error;
  if (reportErrors && strcmp(error, "") != 0) {
    errorCallback(DEVICE, error);
  }
}

void Device::setError(const char *error) { setError((char *)error); }

void Device::generateId() {
  byte mac[6];
  String devEui;
  WiFi.macAddress(mac);
  devEui = String(mac[0], HEX) + String(mac[1], HEX) + String(mac[2], HEX) +
           String(mac[3], HEX) + String(mac[4], HEX) + String(mac[5], HEX);
  devEui.toUpperCase();
  aSerial.vvv().p(F("device DevEUI : ")).pln(devEui);

  set(DEV_EUI, devEui.c_str());
  char mqttClientId[50];
  strcpy(mqttClientId, get(DEV_EUI));
  long randNumber = random(10000);
  char randNumberBuffer[10];
  snprintf(randNumberBuffer, sizeof(randNumberBuffer), "%ld", randNumber);
  strcat(mqttClientId, "-");
  strcat(mqttClientId, randNumberBuffer);
  set(MQTT_CLIENT_ID, mqttClientId);
}

char *Device::getName() { return std::move(_deviceName); }

size_t Device::getNameSize() { return _deviceNameSize; }

void Device::setName(char *name) {
  strlcpy(_deviceName, name, sizeof(_deviceName) - 1);
}

char *Device::getPass() { return _devicePass; }

void Device::setPass(char *pass) {
  strlcpy(_devicePass, pass, sizeof(_devicePass) - 1);
}

char *Device::getDevEui() { return _devEui; }

size_t Device::getDevEuiSize() { return _devEuiSize; }

void Device::setDevEui(char *devEui) {
  strlcpy(_devEui, devEui, sizeof(_devEui) - 1);
}

char *Device::getDeviceId() { return _deviceId; }

size_t Device::getDeviceIdSize() { return _deviceIdSize; }

void Device::setDeviceId(char *id) {
  strlcpy(_deviceId, id, getDeviceIdSize());
}

char *Device::getApiKey() { return _deviceApiKey; }

size_t Device::getApiKeySize() { return _deviceApiKeySize; }

void Device::setApiKey(char *key) {
  strlcpy(_deviceApiKey, key, sizeof(_deviceApiKey) - 1);
}

// HTTP Settings

char *Device::getHTTPHost() { return _httpHost; }

size_t Device::getHTTPHostSize() { return _httpHostSize; }

void Device::setHTTPHost(char *host) {
  strlcpy(_httpHost, host, sizeof(_httpHost) - 1);
}

char *Device::getHTTPPort() { return _httpPort; }

size_t Device::getHTTPPortSize() { return _httpPortSize; }

void Device::setHTTPPort(char *port) {
  strlcpy(_httpPort, port, sizeof(_httpPort) - 1);
}

void Device::setHTTPPort(int port) {
  char portChar[6];
  itoa(port, portChar, 6);
  strlcpy(_httpPort, portChar, sizeof(_httpPort) - 1);
}

char *Device::getHTTPSecure() { return _httpSecure; }

size_t Device::getHTTPSecureSize() { return _httpSecureSize; }

void Device::setHTTPSecure(char *secure) {
  strlcpy(_httpSecure, secure, sizeof(_httpSecure) - 1);
}

void Device::setHTTPSecure(bool secure) {
  setHTTPSecure(BoolToChar(secure));
  // if (secure) {
  //     strlcpy(_httpSecure, "1", sizeof(_httpSecure));
  // } else {
  //   strlcpy(_httpSecure, "0", sizeof(_httpSecure));
  // }
}

char *Device::getHTTPApiRoot() { return _httpApiRoot; }

void Device::setHTTPApiRoot(char *apiRoot) {
  strlcpy(_httpApiRoot, apiRoot, sizeof(_httpApiRoot) - 1);
}

size_t Device::getHTTPApiRootSize() { return _httpApiRootSize; }

// MQTT Settings

char *Device::getMQTTHost() { return _mqttHost; }

size_t Device::getMQTTHostSize() { return _mqttHostSize; }

void Device::setMQTTHost(char *host) {
  strlcpy(_mqttHost, host, sizeof(_mqttHost) - 1);
}

char *Device::getMQTTPort() { return _mqttPort; }

size_t Device::getMQTTPortSize() { return _mqttPortSize; }

void Device::setMQTTPort(char *port) {
  strlcpy(_mqttPort, port, sizeof(_mqttPort) - 1);
}

void Device::setMQTTPort(int port) {
  char portChar[6];
  itoa(port, portChar, 6);
  strlcpy(_mqttPort, portChar, sizeof(_mqttPort) - 1);
}

char *Device::getMQTTSecure() { return _mqttSecure; }

size_t Device::getMQTTSecureSize() { return _mqttSecureSize; }

void Device::setMQTTSecure(char *secure) {
  strlcpy(_mqttSecure, secure, sizeof(_mqttSecure) - 1);
}

void Device::setMQTTSecure(bool secure) {
  setMQTTSecure(BoolToChar(secure));
  // if (secure) {
  //     strlcpy(_mqttSecure, "true", sizeof(_mqttSecure));
  // } else {
  //   strlcpy(_mqttSecure, "false", sizeof(_mqttSecure));
  // }
}

char *Device::getMQTTClientId() { return _mqttClientId; }

size_t Device::getMQTTClientIdSize() { return _mqttClientIdSize; }

void Device::setMQTTClientId(char *clientId) {
  strlcpy(_mqttClientId, clientId, sizeof(_mqttClientId) - 1);
}

char *Device::getMQTTTopicIn() { return _mqttTopicIn; }

size_t Device::getMQTTTopicInSize() { return _mqttTopicInSize; }

void Device::setMQTTTopicIn(char *topic) {
  strlcpy(_mqttTopicIn, topic, sizeof(_mqttTopicIn) - 1);
}

char *Device::getMQTTTopicOut() { return _mqttTopicOut; }

size_t Device::getMQTTTopicOutSize() { return _mqttTopicOutSize; }

void Device::setMQTTTopicOut(char *topic) {
  strlcpy(_mqttTopicOut, topic, sizeof(_mqttTopicOut) - 1);
}

char *Device::getMQTTPrefixIn() { return _mqttPrefixIn; }

char *Device::getMQTTPrefixOut() { return _mqttPrefixOut; }

void Device::setMQTTPrefixIn(char *prefix) {
  strlcpy(_mqttPrefixIn, prefix, sizeof(_mqttPrefixIn) - 1);
}

void Device::setMQTTPrefixOut(char *prefix) {
  strlcpy(_mqttPrefixOut, prefix, sizeof(_mqttPrefixOut) - 1);
}

// Key value methods
char *Device::get(DeviceKeys key) {
  char *value;
  switch (key) {
  case DEVICE_NAME: {
    value = getName();
    break;
  }
  case DEVICE_PASS: {
    value = getPass();
    break;
  }
  case DEV_EUI: {
    value = getDevEui();
    break;
  }
  case DEVICE_ID: {
    value = getDeviceId();
    break;
  }
  case API_KEY: {
    value = getApiKey();
    break;
  }
  case HTTP_HOST: {
    value = getHTTPHost();
    break;
  }
  case HTTP_PORT: {
    value = getHTTPPort();
    break;
  }
  case HTTP_SECURE: {
    value = getHTTPSecure();
    break;
  }
  case HTTP_API_ROOT: {
    value = getHTTPApiRoot();
    break;
  }
  case MQTT_HOST: {
    value = getMQTTHost();
    break;
  }
  case MQTT_PORT: {
    value = getMQTTPort();
    break;
  }
  case MQTT_SECURE: {
    value = getMQTTSecure();
    break;
  }
  case MQTT_CLIENT_ID: {
    value = getMQTTClientId();
    break;
  }
  case MQTT_TOPIC_IN: {
    value = getMQTTTopicIn();
    break;
  }
  case MQTT_TOPIC_OUT: {
    value = getMQTTTopicOut();
    break;
  }
  case MQTT_PREFIX_IN: {
    value = getMQTTPrefixIn();
    break;
  }
  case MQTT_PREFIX_OUT: {
    value = getMQTTPrefixOut();
    break;
  }
  default: { setError("Invalid Key for getter."); }
  }
  return std::move(value);
}

size_t Device::getSize(DeviceKeys key) {
  size_t value = 0;
  switch (key) {
  case DEVICE_NAME: {
    value = getNameSize();
    break;
  }
  // case DEVICE_PASS : {
  //   value = getPassSize();
  //   break;
  // }
  case DEV_EUI: {
    value = getDevEuiSize();
    break;
  }
  case DEVICE_ID: {
    value = getDeviceIdSize();
    break;
  }
  case API_KEY: {
    value = getApiKeySize();
    break;
  }
  case HTTP_HOST: {
    value = getHTTPHostSize();
    break;
  }
  case HTTP_PORT: {
    value = getHTTPPortSize();
    break;
  }
  case HTTP_SECURE: {
    value = getHTTPSecureSize();
    break;
  }
  case HTTP_API_ROOT: {
    value = getHTTPApiRootSize();
    break;
  }
  case MQTT_HOST: {
    value = getMQTTHostSize();
    break;
  }
  case MQTT_PORT: {
    value = getMQTTPortSize();
    break;
  }
  case MQTT_SECURE: {
    value = getMQTTSecureSize();
    break;
  }
  case MQTT_CLIENT_ID: {
    value = getMQTTClientIdSize();
    break;
  }
  case MQTT_TOPIC_IN: {
    value = getMQTTTopicInSize();
    break;
  }
  case MQTT_TOPIC_OUT: {
    value = getMQTTTopicOutSize();
    break;
  }
  // case MQTT_PREFIX_IN : {
  //   value = getMQTTPrefixInSize();
  //   break;
  // }
  // case MQTT_PREFIX_OUT : {
  //   value = getMQTTPrefixOutSize();
  //   break;
  // }
  default: {
    setError("Invalid Key for size getter.");
    return -1;
  }
  }
  return value;
}

void Device::setKV(DeviceKeys key, char *value) {
  switch (key) {
  case DEVICE_NAME: {
    setName(value);
    break;
  }
  case DEVICE_PASS: {
    setPass(value);
    break;
  }
  case DEV_EUI: {
    setDevEui(value);
    break;
  }
  case DEVICE_ID: {
    setDeviceId(value);
    break;
  }
  case API_KEY: {
    setApiKey(value);
    break;
  }
  case HTTP_HOST: {
    setHTTPHost(value);
    break;
  }
  case HTTP_PORT: {
    setHTTPPort(value);
    break;
  }
  case HTTP_SECURE: {
    setHTTPSecure(value);
    break;
  }
  case HTTP_API_ROOT: {
    setHTTPApiRoot(value);
    break;
  }
  case MQTT_HOST: {
    setMQTTHost(value);
    break;
  }
  case MQTT_PORT: {
    setMQTTPort(value);
    break;
  }
  case MQTT_SECURE: {
    setMQTTSecure(value);
    break;
  }
  case MQTT_CLIENT_ID: {
    setMQTTClientId(value);
    break;
  }
  case MQTT_TOPIC_IN: {
    setMQTTTopicIn(value);
    break;
  }
  case MQTT_TOPIC_OUT: {
    setMQTTTopicOut(value);
    break;
  }
  case MQTT_PREFIX_IN: {
    setMQTTPrefixIn(value);
    break;
  }
  case MQTT_PREFIX_OUT: {
    setMQTTPrefixOut(value);
    break;
  }
  default: { setError("Invalid Key for setter."); }
  }
}

Device &Device::set(DeviceKeys key, char *value) {
  setKV(key, value);
  return *this;
}

Device &Device::set(DeviceKeys key, const char *value) {
  setKV(key, (char *)value);
  return *this;
}

// Storage sync
bool Device::setInstance(uint8_t *buffer, size_t length) {
  // use arduinojson.org/v6/assistant
  aSerial.vvvv().pln(F("[DEVICE] setInstance"));

  int objectSize = 6;
  int bufferSize = 180; // 180
  const size_t capacity = JSON_OBJECT_SIZE(objectSize) + bufferSize;
  DynamicJsonDocument doc(capacity);
  DeserializationError error = deserializeJson(doc, buffer, length);
  if (error) {
    aSerial.vvvv().p(F("[DEVICE] setInstance :err : ")).pln(error.c_str());
    // char err[150];
    // strlcpy(err, error.c_str(), sizeof(err));
    // setError(err);
    return false;
  }

  JsonObject obj = doc.as<JsonObject>();
  const char *name = obj["name"];
  if (name != nullptr) {
    //  set(DEVICE_NAME, name);
  }
  const char *devEui = obj["devEui"];
  // const char* type = doc["type"];
  // bool status = doc["status"];
  aSerial.vvvv()
      .pln(F("[ALOES] set Device : "))
      .p("  devEui : ")
      .pln(devEui)
      .p(F("  name : "))
      .pln(name);
  //  setError("");
  return true;
}

bool Device::setInstance(const char *instance) {
  // use arduinojson.org/v6/assistant
  aSerial.vvvv().p(F("[DEVICE] setInstance : ")).pln(instance);
  int objectSize = 6;
  int bufferSize = 180; // 180
  const size_t capacity = JSON_OBJECT_SIZE(objectSize) + bufferSize;
  DynamicJsonDocument doc(capacity * 2);
  DeserializationError error = deserializeJson(doc, instance);
  if (error) {
    aSerial.vvvv().p(F("[DEVICE] setInstance :err : ")).pln(error.c_str());
    // char err[200];
    // strlcpy(err, error.c_str(), sizeof(err));
    // setError(err);
    return false;
  }
  //  JsonObject obj = doc.as<JsonObject>();
  aSerial.vvvv().pln(F("[DEVICE] setInstance :2"));
  const char *name = doc["name"];
  const char *devEui = doc["devEui"];
  aSerial.vvvv()
      .pln(F("[ALOES] set Device : "))
      .p("  devEui : ")
      .pln(devEui)
      .p(F("  name : "))
      .pln(name);
  //  setError("");
  return true;
}

bool Device::initSensors() {
  const int capacity = JSON_ARRAY_SIZE(25) + 25 * JSON_OBJECT_SIZE(5);
  aSerial.vvvv().p(F("[DEVICE] initSensors : ")).pln(sensorsFileName.c_str());

#if defined(ESP8266)
  File sensorsFile = SPIFFS.open(sensorsFileName.c_str(), "r");
#elif defined(ESP32)
  File sensorsFile = SPIFFS.open(sensorsFileName.c_str());
#endif

  if (!sensorsFile) {
    // setError("Sensors file doesn't exist.");
    aSerial.vvvv()
        .p(F("[DEVICE] loadSensors err: "))
        .pln(sensorsFileName.c_str());
    return false;
  }
  size_t size = sensorsFile.size();
  std::unique_ptr<char[]> buf(new char[size]);
  sensorsFile.readBytes(buf.get(), size);

  DynamicJsonDocument doc(capacity * 2);
  // StaticJsonDocument<capacity> doc;
  DeserializationError error = deserializeJson(doc, buf.get(), size);
  sensorsFile.close();

  if (error) {
    char err[150];
    strncpy(err, error.c_str(), sizeof(err));
    setError(err);
    return false;
  }

  JsonArray sensorsArray = doc.as<JsonArray>();
  int arrayLength = sensorsArray.size();
  this->sensors = new Sensors<int>(arrayLength, SensorKeysLength);
  unsigned int i = 1;
  for (JsonObject sensor : sensorsArray) {
    this->sensors->set(i, S_OBJECT_ID, sensor["objectId"].as<int>());
    this->sensors->set(i, S_NODE_ID, sensor["nodeId"].as<int>());
    this->sensors->set(i, S_SENSOR_ID, sensor["sensorId"].as<int>());
    this->sensors->set(i, S_RESOURCE_ID, sensor["resourceId"].as<int>());
    i++;
  }

  return true;
}

bool Device::loadConfig() {
  aSerial.vvv().pln(F("[DEVICE] Reading config file."));
#if defined(ESP8266)
  File configFile = SPIFFS.open(configFileName.c_str(), "r");
#elif defined(ESP32)
  File configFile = SPIFFS.open(configFileName.c_str());
#endif

  if (configFile) {
    size_t size = configFile.size();
    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);
    configFile.readBytes(buf.get(), size);
    DynamicJsonDocument obj(objBufferSize * 2);
    DeserializationError error = deserializeJson(obj, buf.get(), size);
    configFile.close();

    if (error) {
      char err[150];
      strlcpy(err, error.c_str(), sizeof(err));
      setError(err);
      return false;
    } else {
#if DEBUG != 0
      if (serializeJsonPretty(obj, Serial) == 0) {
        setError("Failed to write to Serial");
        return false;
      }
      aSerial.v().pln();
#endif
      const char *deviceName = obj["deviceName"];
      const char *deviceId = obj["deviceId"];
      const char *apiKey = obj["deviceApiKey"];
      set(DEVICE_NAME, deviceName)
          .set(DEVICE_ID, deviceId)
          .set(API_KEY, apiKey);
      const char *httpHost = obj["httpHost"];
      const char *httpPort = obj["httpPort"];
      const char *httpSecure = obj["httpSecure"];
      set(HTTP_PORT, httpPort)
          .set(HTTP_HOST, httpHost)
          .set(HTTP_SECURE, httpSecure);
      const char *mqttHost = obj["mqttHost"];
      const char *mqttPort = obj["mqttPort"];
      const char *mqttSecure = obj["mqttSecure"];
      set(MQTT_PORT, mqttPort)
          .set(MQTT_HOST, mqttHost)
          .set(MQTT_SECURE, mqttSecure);
      const char *mqttTopicIn = obj["mqttTopicIn"];
      const char *mqttTopicOut = obj["mqttTopicOut"];
      set(MQTT_TOPIC_IN, mqttTopicIn).set(MQTT_TOPIC_OUT, mqttTopicOut);
      if (obj["ip"]) {
        // strcpy(config.staticIp, obj["ip"]);
        // strcpy(config.staticGw, obj["gateway"]);
        // strcpy(config.staticSn, obj["subnet"]);
      }
      //  onDeviceUpdate();
      //  setError("");
      return true;
    }
  }
  setError("Failed to load config file");
  return false;
}

bool Device::initConfig() {
  set(DEVICE_NAME, DEFAULT_DEVICE_NAME)
      .set(DEVICE_ID, defaultDeviceId)
      .set(API_KEY, defaultDeviceApiKey);
  set(HTTP_PORT, defaultHttpPort)
      .set(HTTP_HOST, defaultHttpHost)
      .set(HTTP_SECURE, BoolToChar(defaultHttpSecure));
  set(MQTT_PORT, defaultMqttPort)
      .set(MQTT_HOST, defaultMqttHost)
      .set(MQTT_SECURE, BoolToChar(defaultMqttSecure));
  set(MQTT_TOPIC_IN, defaultMqttTopicIn)
      .set(MQTT_TOPIC_OUT, defaultMqttTopicOut);
  return true;
}

bool Device::init() {
  aSerial.vvv().pln(F("[DEVICE] init..."));
  if (SPIFFS.begin()) {
    aSerial.vvv().pln(F("[DEVICE] FS mounted"));
    if (SPIFFS.exists(configFileName)) {
      if (loadConfig()) {
        return true;
      }
    } else if (initConfig()) {
      return true;
    }
    setError("Failed to load device config.");
    return false;
  }
  aSerial.vv().pln(F("[DEVICE] Failed to mount FS."));
  setError("Failed to mount FS.");
  return false;
}

bool Device::saveConfig() {
  DynamicJsonDocument obj(objBufferSize * 2);
  obj["deviceName"] = getName();
  obj["deviceId"] = getDeviceId();
  obj["deviceApiKey"] = getApiKey();
  obj["httpHost"] = getHTTPHost();
  obj["httpPort"] = getHTTPPort();
  obj["httpSecure"] = getHTTPSecure();
  obj["mqttHost"] = getMQTTHost();
  obj["mqttPort"] = getMQTTPort();
  obj["mqttSecure"] = getMQTTSecure();
  obj["mqttClient"] = getMQTTClientId();
  obj["mqttTopicIn"] = getMQTTTopicIn();
  obj["mqttTopicOut"] = getMQTTTopicOut();
  //    obj["ip"] = WiFi.localIP().toString();
  //    obj["gateway"] = WiFi.gatewayIP().toString();
  //    obj["subnet"] = WiFi.subnetMask().toString();
#if defined(ESP8266)
  File configFile = SPIFFS.open(configFileName.c_str(), "w");
#elif defined(ESP32)
  File configFile = SPIFFS.open(configFileName.c_str(), FILE_WRITE);
#endif
  if (!configFile) {
    setError("Failed to create config file.");
    return false;
  }
  if (serializeJson(obj, configFile) == 0) {
    setError("Failed to write to file.");
    return false;
  }
#if defined(ESP8266)
  configFile.close();
#endif

#if DEBUG != 0
  if (serializeJsonPretty(obj, Serial) == 0) {
    aSerial.vv().pln(F("[DEVICE] Failed to write to Serial"));
  }
  aSerial.v().pln();
#endif

  onDeviceUpdate();
  return true;
}
