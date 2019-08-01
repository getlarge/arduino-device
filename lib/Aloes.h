/*
 * Aloes.h
 *
 * Author:  Getlarge
 * Created: 2019-06-10
 */

#ifndef Aloes_h
#define Aloes_h

#if defined(ESP8266) || defined(ESP32)
#include <functional>
#define MESSAGE_CALLBACK_SIGNATURE std::function<void(transportLayer transportType, Message *message)> msgCallback
#else
#define MESSAGE_CALLBACK_SIGNATURE void (*msgCallback)(transportLayer transportType, Message *message)
#endif

struct Sensor {
	char *name;
	char *type;
	char *value;
	char *nativeId;
	char *resourceId;
	char *id;
	char* resources[][60];
};

// const char* resources[][60] = {
//   { "5850", "digital_input"},
//   { "5850", "digital_input"},
//   { "5527", "text_input"},
// };

class Aloes {
public:
	Aloes();
	Aloes(Aloes &aloes);

  void setMsgCallback(MESSAGE_CALLBACK_SIGNATURE);
	bool initDevice(DEVICE_CALLBACK_SIGNATURE);
  void onDeviceUpdate();
	void initSensors();
	void presentSensors();

	char* getConfig(DeviceKeys deviceKey);
	void setConfig(DeviceKeys deviceKey, char *value);
	Aloes& setCnf(DeviceKeys deviceKey, char* value); 
	char* getMsg(MessageKeys messageKey);
	Aloes& setMsg(MessageKeys messageKey, char* value); 
	Aloes& setMsg(MessageKeys messageKey, const char* value); 
	Aloes& setMsg(MessageKeys messageKey, uint8_t* value, size_t length); 

	void setPayload(uint8_t *payload, size_t length, const char* type);

	bool sendMessage(transportLayer transportType);
	bool sendMessage(size_t length);
	bool startStream(size_t length);
	virtual size_t writeStream(const uint8_t *payload, size_t length);
	bool endStream();

	bool parseUrl(char *url);
	bool parseTopic(char* topic);
	bool parseRoute(transportLayer transportType, char* route);
	bool parseBody(uint8_t *body, size_t length);
  //	void parseBody(Stream *stream);
  bool parsePayload(uint8_t *payload, size_t length);
	bool parseMessage(transportLayer transportType, uint8_t *message, size_t length);

	bool getState();
	//	void authenticate();
	void getFirmwareUpdate();

  bool stateReceived;
  bool sensorsPresented;

private:
	//	Sensor sensors[];
	Message _message; 
	MESSAGE_CALLBACK_SIGNATURE;
};

#endif
