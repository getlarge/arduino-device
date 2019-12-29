#ifndef Globals_h
#define Globals_h

#if defined(ESP8266) || defined(ESP32)
#include <functional>
#define ERROR_CALLBACK_SIGNATURE                                               \
  std::function<void(modules moduleName, char *)> onError
#define MQTT_CALLBACK                                                          \
  std::function<void(char *, uint8_t *, unsigned int)> mqttCallback
#define HTTP_CALLBACK                                                          \
  std::function<void(char *, uint8_t *, unsigned int)> httpCallback
#define MESSAGE_CALLBACK_SIGNATURE                                             \
  std::function<void(transportLayer type, Message * message)> msgCallback
#else
#define ERROR_CALLBACK_SIGNATURE void (*onError)(modules moduleName, char *)
#define HTTP_CALLBACK void (*httpCallback)(char *, uint8_t *, unsigned int)
#define MQTT_CALLBACK void (*mqttCallback)(char *, uint8_t *, unsigned int)
#define MESSAGE_CALLBACK_SIGNATURE                                             \
  void (*msgCallback)(transportLayer type, Message * message)
#endif

// #if defined(ESP8266) || defined(ESP32)
// #include <functional>
// #define DEVICE_CALLBACK_SIGNATURE std::function<void()> onDeviceUpdate
// #else
// #define DEVICE_CALLBACK_SIGNATURE void (*onDeviceUpdate)()
// #endif

enum DeviceKeys {
  DEVICE_NAME,
  DEVICE_PASS,
  DEV_EUI,
  DEVICE_ID,
  API_KEY,
  HTTP_HOST,
  HTTP_PORT,
  HTTP_SECURE,
  HTTP_API_ROOT,
  MQTT_HOST,
  MQTT_PORT,
  MQTT_SECURE,
  MQTT_CLIENT_ID,
  MQTT_TOPIC_IN,
  MQTT_TOPIC_OUT,
  MQTT_PREFIX_IN,
  MQTT_PREFIX_OUT
};

enum modules { NETWORK, DEVICE, MESSAGE, TRANSPORT, ALOES, EXEC };

enum MessageKeys {
  METHOD,
  OBJECT_ID,
  SENSOR_ID,
  NODE_ID,
  RESOURCE_ID,
  PAYLOAD,
  TOPIC,
  MQTT_PREFIX,
  COLLECTION,
  PATH,
  PARAM,
  URL,
  HTTP_PREFIX
};

enum transportLayer { HTTP, MQTT, UART };

#endif
