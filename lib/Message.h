/*
 * Message.h
 *
 * Author:  Getlarge
 * Created: 2019-06-15
 */

#ifndef Message_h
#define Message_h

class Message {
public:
  Message();

  void setup(Device *device);
  void update(Device *device);

  char *getMethod();
  void setMethod(char *method);
  char *getObjectId();
  void setObjectId(char *objectId);
  char *getNodeId();
  void setNodeId(char *nodeId);
  char *getSensorId();
  void setSensorId(char *sensorId);
  char *getResourceId();
  void setResourceId(char *resourceId);
  char *getCollection();
  void setCollection(char *collection);
  char *getPath();
  void setPath(char *path);
  char *getParam();
  void setParam(char *param);

  char *getPayload();
  void setPayload(char *payload);
  void setPayload(uint8_t *payload, size_t length);
  void setPayload(uint8_t *payload, size_t length, const char *type);

  char *getTopic();
  void setTopic(char *topic);
  char *fillTopic();
  void fillTopic(char *&&topic);
  char *getUrl();
  void setUrl(char *url);
  char *fillUrl();
  void fillUrl(char *&&url);

  char *get(MessageKeys key);
  void setKV(MessageKeys key, char *value);
  Message &set(MessageKeys key, char *value);
  Message &set(MessageKeys key, const char *value);
  Message &set(MessageKeys key, uint8_t *value, size_t length);

private:
  char _method[6];
  char _objectId[6];
  char _nodeId[5];
  char _sensorId[5];
  char _resourceId[6];
  char *_payload;
  char _topic[200];
  char _mqttPrefix[50];
  char _httpPrefix[50];
  char _collection[20];
  char _path[100];
  char _param[50];
  char _url[200];
};

#endif
