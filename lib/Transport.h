/*
 * Transport.h
 *
 * Author:  Getlarge
 * Created: 2019-06-10
 */
#ifndef Transport_h
#define Transport_h

#if CLIENT_SECURE == 1
#include <WiFiClientSecure.h>
//	#include <WiFiClientSecureBearSSL.h>
#endif

#if defined(ESP8266) 
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#elif defined(ESP32)
#include <HTTPClient.h>
#include <Update.h>
#endif

#include <PubSubClient.h>

#if defined(ESP8266) || defined(ESP32)
#include <functional>
#define MQTT_CALLBACK_SIGNATURE std::function<void(char*, uint8_t*, unsigned int)> callback
#else
#define MQTT_CALLBACK_SIGNATURE void (*callback)(char*, uint8_t*, unsigned int)
#endif

#if defined(ESP8266) || defined(ESP32)
#include <functional>
#define HTTP_CALLBACK_SIGNATURE std::function<void(char*, uint8_t*, unsigned int)> httpCallback
#else
#define HTTP_CALLBACK_SIGNATURE void (*httpCallback)(char*, uint8_t*, unsigned int)
#endif

// struct mqttPacket {
// 	char *topic;
// 	char *payload;
// };

// struct HTTPRes {
// 	static char *method;
// 	static char *collection;
// 	static char *path;
// 	static char *result;
// 	int httpCode;
// };

class Transport {
public:
	Transport();
	void setErrorCallback(ERROR_CALLBACK_SIGNATURE);
	void setError(char* error);
	void setError(const char* error);

	bool setupMQTT(Device &device);
	bool setupHTTP(Device &device);
	void update(Device &device);
	void setMQTTCallback(MQTT_CALLBACK_SIGNATURE);
	void setHTTPCallback(HTTP_CALLBACK_SIGNATURE);
	bool loop(Device &device);

	bool connect(transportLayer transportType);
	bool connectHTTP();
	bool connectMQTT();
	bool asyncConnect(transportLayer transportType, AsyncWait *async, MilliSec startTime);
	bool asyncConnect(transportLayer transportType, AsyncWait *async, MilliSec startTime, unsigned long interval);
	bool asyncConnectHTTP(AsyncWait *async, MilliSec startTime, unsigned long interval);
	bool asyncConnectMQTT(AsyncWait *async, MilliSec startTime, unsigned long interval);

	void disconnect(transportLayer transportType);
    bool connected(transportLayer transportType);

	bool publish(const char* topic, const char* payload, bool retained);
    bool publish(const char* topic, const uint8_t *payload, size_t length, bool retained);
	bool subscribe(const char* topic, int qos);
	bool beginPublish(const char* topic, size_t length, bool retained);
	virtual size_t write(const uint8_t *payload, size_t length);
	bool endPublish();
	void parseStream(const char *url, size_t length);
	void setRequestHeaders();
	void setResponseHeaders(char* headers[][100]);
	int sendRequest(int method, const char *payload);
	bool setRequest(const char *method, const char *url, const char* payload);
	void getUpdated(int which, const char* host, int port, const char* url);

	int mqttFailCount = 0;
	int mqttMaxFailedCount = 10;
	int httpFailCount = 0;
	int httpMaxFailedCount = 5;

private:

	bool mqttConfigured = false;
	bool httpConfigured = false;
	unsigned long httpReconnectInterval = 2000;
	unsigned long mqttReconnectInterval = 1000;
	bool connectingMqtt = false;
	bool connectingHttp = false;

	MQTT_CALLBACK_SIGNATURE;
	HTTP_CALLBACK_SIGNATURE;

	ERROR_CALLBACK_SIGNATURE;
	const char* _error;
	static bool reportErrors;

	//	HTTPRes httpRes;

	WiFiClient _client;
	WiFiClient _client2;

#if CLIENT_SECURE == 1
	// 	//	BearSSL::WiFiClientSecure *_clientS;
	WiFiClientSecure _clientS;
	WiFiClientSecure _client2S;
#endif

	HTTPClient _httpClient;
	PubSubClient _mqttClient;

	const char* _userId;
	const char* _apiKey;
	const char* _httpHost;
	const char* _httpUrl;
	int _httpPort;
	const char* _httpApiRoot;
	static bool _httpSecure;
	const char* _mqttClientId;
	const char* _mqttHost;
	const char* _mqttTopicIn;
	const char* _mqttTopicOut;
	int _mqttPort;
	static bool _mqttSecure;

	uint8_t fingerprint[20];

};

#endif
