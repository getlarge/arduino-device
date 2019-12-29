/*
 * Transport.h
 *
 * Author:  Getlarge
 * Created: 2019-06-10
 */
#ifndef Transport_h
#define Transport_h

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
  Transport(Device *device);
  ~Transport();

  void setErrorCallback(ERROR_CALLBACK_SIGNATURE);
  void setError(char *error);
  void setError(const char *error);

  void setCredentials(Device *device);
  bool setupMQTT(Device *device);
  void cleanMQTT();
  bool setupHTTP(Device *device);
  void setMQTTCallback(MQTT_CALLBACK);
  void setHTTPCallback(HTTP_CALLBACK);

#if defined(ESP8266) || defined(ESP32)
#include <functional>
  std::function<void(char *, uint8_t *, unsigned int)> onMQTTMessage;
  std::function<void(char *, uint8_t *, unsigned int)> onHTTPMessage;
  std::function<void(modules moduleName, char *)> errorCallback;
#else
  void (*onHTTPMessage)(char *, uint8_t *, unsigned int);
  void (*onMQTTMessage)(char *, uint8_t *, unsigned int);
  void (*errorCallback)(modules moduleName, char *);
#endif

  bool connect(transportLayer type);
  bool connectHTTP();
  bool connectMQTT();

  void disconnect(transportLayer type);
  bool connected(transportLayer type);
  void resetConnectedState();

  bool loop();

  bool publish(const char *topic, const char *payload, bool retained);
  bool publish(const char *topic, const uint8_t *payload, size_t length,
               bool retained);
  bool subscribe(const char *topic, int qos);
  bool beginPublish(const char *topic, size_t length, bool retained);
  virtual size_t write(const uint8_t *payload, size_t length);
  bool endPublish();
  void parseStream(const char *url, size_t length);
  void setRequestHeaders();
  void setResponseHeaders(char *headers[][100]);
  int sendRequest(int method, const char *payload);
  bool setRequest(const char *method, const char *url, const char *payload);
  void getUpdated(int which, const char *host, int port, const char *url);

  int mqttFailCount = 0;
  int mqttMaxFailedCount = 10;
  bool _mqttClientAuthenticated;
  char mainMQTTTopic[60];
  int httpFailCount = 0;
  int httpMaxFailedCount = 5;
  bool _httpClientAuthenticated;

private:
  bool _mqttConfigured = false;
  bool _httpConfigured = false;
  unsigned long _httpReconnectInterval = 2000;
  unsigned long _mqttReconnectInterval = 1000;
  bool connectingMqtt = false;
  bool connectingHttp = false;

  const char *_error;
  static bool reportErrors;

  WiFiClient _client;
  WiFiClient _client2;

#if CLIENT_SECURE == 1
  // 	//	BearSSL::WiFiClientSecure *_clientS;
  WiFiClientSecure _clientS;
  // WiFiClientSecure _client2S;
  // BearSSL::WiFiClientSecure _clientS;
  BearSSL::WiFiClientSecure _client2S;
#endif

  HTTPClient _httpClient;
  PubSubClient _mqttClient;
  // PubSubClient *_mqttClient;

  const char *_userId;
  const char *_devEui;
  const char *_apiKey;
  const char *_httpHost;
  const char *_httpUrl;
  int _httpPort;
  const char *_httpApiRoot;
  static bool _httpSecure;
  const char *_mqttClientId;
  const char *_mqttHost;
  const char *_mqttTopicIn;
  const char *_mqttTopicOut;
  int _mqttPort;
  static bool _mqttSecure;

  uint8_t fingerprint[20];
};

#endif
