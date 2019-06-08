#ifndef Transport_h
#define Transport_h

#if MQTT_CLIENT == 1
#include <PubSubClient.h>
#elif WEB_SERVER == 1
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#endif

#include "Helpers.h"

#define MQTT_CALLBACK_SIGNATURE std::function<void(char*, uint8_t*, unsigned int)> callback
#define HTTP_CALLBACK_SIGNATURE std::function<void()> httpCallback

class Transport {
public:
	Transport(Client &client);
	void init(Config &config, MQTT_CALLBACK_SIGNATURE);
	void init(Config &config, HTTP_CALLBACK_SIGNATURE);
	void connect(Config &config);
	bool reconnect(Config &config);
    bool connected();
    bool publish(const char* topic, const char* payload, bool retained);
	bool subscribe(const char* topic, int qos);
	bool beginPublish(const char* topic, size_t length, bool retained);
	virtual size_t write(const uint8_t *payload, size_t length);
	bool endPublish();
	void setCallback(MQTT_CALLBACK_SIGNATURE);
	void setCallback(HTTP_CALLBACK_SIGNATURE);
	void loop();
	int mqttFailCount = 0;
	int mqttMaxFailedCount = 60;
	bool configured = false;

private:
#if MQTT_CLIENT == 1
	PubSubClient _client;
#elif WEB_SERVER == 1
	ESP8266WebServer server(80);
	WiFiClient _client;
#endif
	unsigned long lastMqttReconnectAttempt = 0;
	unsigned long reconnectInterval = 500;
};

#endif
