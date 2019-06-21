#ifndef Globals_h
#define Globals_h

enum modules { NETWORK, DEVICE, MESSAGE, TRANSPORT, ALOES, EXEC };
modules module;

enum DeviceKeys { DEVICE_NAME, DEVICE_PASS, DEV_EUI, DEVICE_ID, API_KEY , 
	HTTP_HOST, HTTP_PORT, HTTP_SECURE, HTTP_API_ROOT, 
	MQTT_HOST, MQTT_PORT, MQTT_SECURE, MQTT_CLIENT_ID,
	MQTT_TOPIC_IN, MQTT_TOPIC_OUT, MQTT_PREFIX_IN, MQTT_PREFIX_OUT };

DeviceKeys deviceKey;

enum MessageKeys { METHOD, OBJECT_ID, SENSOR_ID, RESOURCE_ID, 
	PAYLOAD, TOPIC, MQTT_PREFIX, 
	COLLECTION, PATH, PARAM, URL, HTTP_PREFIX };

MessageKeys messageKey;

#if defined(ESP8266) || defined(ESP32)
#include <functional>
#define ERROR_CALLBACK_SIGNATURE std::function<void(modules module, char*)> onError
#else
#define ERROR_CALLBACK_SIGNATURE void (*onError)(modules module, char*)
#endif

enum transportLayer { HTTP, MQTT, UART };
transportLayer transportType;

String postTopics[200] = {};

bool callConfigMode = false;
bool manualConfig = false;
int configMode = 0;
unsigned long debouncerInterval = 2000;

#endif
