/*
 * Aloes.h
 *
 * Author:  Getlarge
 * Created: 2019-06-10
 */

#ifndef Aloes_h
#define Aloes_h

#include "Device.cpp"
#include "Message.cpp"
#include "Transport.cpp"

class Aloes {
public:
  Aloes();
  Transport *client = 0;
  Device *device = 0;

  bool init(void (*deviceUpdateCb)(), ERROR_CALLBACK_SIGNATURE);

  void setMsgCallback(MESSAGE_CALLBACK_SIGNATURE);
  void setMsgCallback(MESSAGE_CALLBACK_SIGNATURE, MQTT_CALLBACK, HTTP_CALLBACK);

#if defined(ESP8266) || defined(ESP32)
#include <functional>
  std::function<void(transportLayer type, Message *message)> onMessage;
  std::function<void(modules moduleName, char *)> errorCallback;
#else
  void (*onMessage)(transportLayer type, Message *message);
  void (*errorCallback)(modules moduleName, char *);
#endif

  void onDeviceUpdate();
  void presentSensors();

  char *getDevice(DeviceKeys key);
  void setDevice(DeviceKeys key, char *value);
  Aloes &setCnf(DeviceKeys key, char *value);
  char *getMsg(MessageKeys key);
  Aloes &setMsg(MessageKeys key, char *value);
  Aloes &setMsg(MessageKeys key, const char *value);
  Aloes &setMsg(MessageKeys key, uint8_t *value, size_t length);
  Aloes &setMsg(MessageKeys key, int value);
  void setPayload(uint8_t *payload, size_t length, const char *type);

  bool sendMessage(transportLayer type);
  bool sendMessage(size_t length);
  bool startStream(size_t length);
  virtual size_t writeStream(const uint8_t *payload, size_t length);
  bool endStream();

  bool parseUrl(char *url);
  bool parseTopic(char *topic);
  bool parseRoute(transportLayer type, char *route);
  bool parseBody(uint8_t *body, size_t length);
  //	void parseBody(Stream *stream);
  bool parsePayload(uint8_t *payload, size_t length);
  bool parseMessage(transportLayer type, uint8_t *message, size_t length);

  bool getState();
  //	void authenticate();
  void getFirmwareUpdate();

  bool stateReceived;
  bool sensorsPresented;
  int retryGetStateCount = 0;

private:
  Message _message;
};

#endif
