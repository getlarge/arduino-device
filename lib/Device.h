/*
 * Device.h
 *
 * Author:  Getlarge
 * Created: 2019-06-19
 */

#ifndef Device_h
#define Device_h

#if defined(ESP8266) || defined(ESP32)
#include <functional>
#define DEVICE_CALLBACK_SIGNATURE std::function<void()> onDeviceUpdate
#else
#define DEVICE_CALLBACK_SIGNATURE void (*onDeviceUpdate)()
#endif

inline const char * const BoolToChar(bool b); 

class Device {
public:
	Device();
	Device(Device &device);
	bool init();
	void setErrorCallback(ERROR_CALLBACK_SIGNATURE);
	void setError(char* error);
	void setError(const char* error);
	void setCallback(DEVICE_CALLBACK_SIGNATURE);

	char* getName();
	const size_t getNameSize();
	void setName(char *name);
	char* getPass();
	void setPass(char *pass);
	char* getDevEui();
	const size_t getDevEuiSize();
	void setDevEui(char *devEui);
	char* getDeviceId();
	const size_t getDeviceIdSize();
	void setDeviceId(char *id);
	char* getApiKey();
	const size_t getApiKeySize();
	void setApiKey(char *key);


	char* getHTTPHost();
	const size_t getHTTPHostSize();
	void setHTPPHost(IPAddress *host);
	void setHTTPHost(char *host);
	char* getHTTPPort();
	const size_t getHTTPPortSize();
	void setHTTPPort(char *port);
	void setHTTPPort(int port);
	char* getHTTPSecure();
	const size_t getHTTPSecureSize();
	void setHTTPSecure(char* secure);
	void setHTTPSecure(bool secure);
	char* getHTTPApiRoot();
	const size_t getHTTPApiRootSize();
	void setHTTPApiRoot(char *apiRoot);

	char* getMQTTHost();
	const size_t getMQTTHostSize();
	void setMQTTHost(IPAddress *host);
	void setMQTTHost(char *host);
	char* getMQTTPort();
	const size_t getMQTTPortSize();
	void setMQTTPort(char* port);
	void setMQTTPort(int port);
	char* getMQTTSecure();
	const size_t getMQTTSecureSize();
	void setMQTTSecure(char* secure);
	void setMQTTSecure(bool secure);
	char* getMQTTClientId();
	const size_t getMQTTClientIdSize();
	void setMQTTClientId(char *clientId);
	char* getMQTTTopicIn();
	const size_t getMQTTTopicInSize();
	void setMQTTTopicIn(char *topic);
	char* getMQTTTopicOut();
	const size_t getMQTTTopicOutSize();
	void setMQTTTopicOut(char *topic);
	char* getMQTTPrefixIn();
	const size_t getMQTTPrefixInSize();
	void setMQTTPrefixIn(char *prefix);
	char* getMQTTPrefixOut();
	const size_t getMQTTPrefixOutSize();
	void setMQTTPrefixOut(char *prefix);


	char* get(DeviceKeys deviceKey);
	const size_t getSize(DeviceKeys deviceKey);
	//	Device& get(); 
	void setKV(DeviceKeys deviceKey, char *value);
    Device& set(DeviceKeys deviceKey, char* value); 
    Device& set(DeviceKeys deviceKey, const char* value); 

    bool setInstance(uint8_t *buffer, size_t length);
    bool setInstance(char *instance);
	bool loadConfig(const String filename);
	bool initConfig();
	bool saveConfig();

private:
	DEVICE_CALLBACK_SIGNATURE;

	ERROR_CALLBACK_SIGNATURE;
	static bool reportErrors;
	const char* _error;

	const String configFileName = "config.json";
	static const size_t objBufferSize = 512;

	static const size_t _deviceNameSize = 30;
  	char _deviceName[_deviceNameSize];
  	static const size_t _devEuiSize = 20;
 	char _devEui[_devEuiSize];
 	static const size_t _devicePassSize = 40;
	char _devicePass[_devicePassSize];

 	static const size_t _deviceIdSize = 40;
	char _deviceId[_deviceIdSize];
	static const size_t _deviceApiKeySize = 70;
	char _deviceApiKey[_deviceApiKeySize];

	static const size_t _httpHostSize = 40;
	char _httpHost[_httpHostSize];
	static const size_t _httpPortSize = 5;
	char _httpPort[_httpPortSize];
	static const size_t _httpSecureSize = 5;
	char _httpSecure[_httpSecureSize];
	static const size_t _httpApiRootSize = 50;
	char _httpApiRoot[_httpApiRootSize];

	static const size_t _mqttHostSize = 40;
	char _mqttHost[_mqttHostSize];
	static const size_t _mqttPortSize = 5;
	char _mqttPort[_mqttPortSize];
	static const size_t _mqttSecureSize = 5;
	char _mqttSecure[_mqttSecureSize];
	static const size_t _mqttClientIdSize = 50;
	char _mqttClientId[_mqttClientIdSize];
	static const size_t _mqttPrefixInSize = 10;
	char _mqttPrefixIn[_mqttPrefixInSize];
	static const size_t _mqttPrefixOutSize = 10;
	char _mqttPrefixOut[_mqttPrefixOutSize];
	static const size_t _mqttTopicOutSize = 60;
	char _mqttTopicOut[_mqttTopicOutSize];
	static const size_t _mqttTopicInSize = 60;
	char _mqttTopicIn[_mqttTopicInSize];

	// char _staticIp[16] = "192.168.1.35";
	// char _staticGw[16] = "192.168.1.254";
	// char _staticSn[16] = "91.121.61.147";
};

#endif
