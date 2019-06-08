#include "Aloes.h"

// initialize node manager
Aloes::Aloes() {
  // setup the message container
 // message = Message();
  //  _message = Message();
  // allocate block for all the sensors if sensor_count is provided
  //  if (sensor_count > 0) sensors.allocateBlocks(sensor_count);
  // setup serial port baud rate
}

void Aloes::getDeviceId(Config &config) {
#if ID_TYPE == 0
  char *espChipId;
  float chipId = ESP.getChipId();
  char chipIdBuffer[sizeof(chipId)];
  espChipId = dtostrf(chipId, sizeof(chipId), 0, chipIdBuffer);
  strcpy(config.devEui, espChipId);
#endif
#if ID_TYPE == 1
  String macAdress = WiFi.macAddress();
  char macAdressBuffer[20];
  macAdress.toCharArray(macAdressBuffer, 20);
  // next => remove the ":" in the mac adress
  strcpy(config.devEui, macAdressBuffer);
#endif
  //    #if ID_TYPE == 2
  //// soyons fous, let's create an eui64 address ( like ipv6 )
  ////      Step #1: Split the MAC address in the middle:
  ////      Step #2: Insert FF:FE in the middle:
  ////      Step #4: Convert the first eight bits to binary:
  ////      Step #5: Flip the 7th bit:
  ////      Step #6: Convert these first eight bits back into hex:
  //    #endif
  aSerial.vvv().p(F("DeviceID : ")).pln(config.devEui);
  helpers.generateId(config);
}

void Aloes::setSensorRoutes(Config &config, const char* objectId, const char* sensorId, const char* resourceId, size_t index) {
  // "pattern": "+prefixedDevEui/+method/+omaObjectId/+sensorId/+ipsoResourceId",
  //  char topic[100];
  char topic[150];
  strcpy(topic, config.mqttTopicOut);
  strcat(topic, "/1/" );
  strcat(topic, objectId );
  strcat(topic, "/" );
  strcat(topic, sensorId );
  strcat(topic, "/" );
  strcat(topic, resourceId );
  aSerial.vvvv().p(F("set topic : ")).pln(topic);
  postTopics[index] = String(topic);
}

void Aloes::setSensors(Config &config) {
  for (size_t i = 0; i < sizeof(sensors) / sizeof(sensors[0]); i++) {
    const char** sensor = sensors[i];
    char objectId[5];
    char sensorId[4];
    char resourceId[5];
    //  printf("sensor[%u]\n", i);
    for (size_t j = 0; sensor[j]; j++) {
      if (j == 0 ) {
        strlcpy(objectId, sensor[j], sizeof(objectId));
      } else if (j == 1 ) {
        strlcpy(sensorId, sensor[j], sizeof(sensorId));
      }  else if (j == 2 ) {
        strlcpy(resourceId, sensor[j], sizeof(resourceId));
      }
      //  printf("  [%s]\n", sensor[j]);
    }
    setSensorRoutes(config, (const char*)objectId, (const char*)sensorId, (const char*)resourceId, i);
  }
}

void Aloes::presentSensors(Config &config) {
  for (size_t i = 0; i < sizeof(sensors) / sizeof(sensors[0]); i++) {
    const char** sensor = sensors[i];
    char objectId[5];
    char sensorId[4];
    char resourceId[5];
    char payload[30];
    //  printf("sensor[%u]\n", i);
    for (size_t j = 0; sensor[j]; j++) {
      if (j == 0 ) {
        strlcpy(objectId, sensor[j], sizeof(objectId));
      } else if (j == 1 ) {
        strlcpy(sensorId, sensor[j], sizeof(sensorId));
      }  else if (j == 2 ) {
        strlcpy(resourceId, sensor[j], sizeof(resourceId));
      } else if (j == 3 ) {
        strlcpy(payload, sensor[j], sizeof(payload));
      }
      //  printf("  [%s]\n", sensor[j]);
    }
    char presentationTopic[80];
    strlcpy(presentationTopic, config.mqttTopicOut, sizeof(presentationTopic));
    strcat(presentationTopic, "/0/" );
    strcat(presentationTopic, objectId );
    strcat(presentationTopic, "/" );
    strcat(presentationTopic, sensorId );
    strcat(presentationTopic, "/" );
    strcat(presentationTopic, resourceId );
    transport.publish((const char*)presentationTopic, payload, false);
    //  mqttClient.publish((const char*)presentationTopic, payload);
  }
}

void Aloes::setMessage(Message &message, char method[5], char objectId[5], char sensorId[4], char resourceId[5], char payload[100] ) {
//  void Aloes::setMessage(Message &message, char method[5], char *objectId, char *sensorId, char *resourceId, const char *payload) {
  aSerial.vvv().p(F("setMessage : ")).p(resourceId).p("  ").pln(objectId);
  strlcpy(message.method, method, sizeof(message.method));
  strlcpy(message.omaObjectId, objectId, sizeof(message.omaObjectId));
  strlcpy(message.sensorId, sensorId, sizeof(message.sensorId));
  strlcpy(message.omaResourceId, resourceId, sizeof(message.omaResourceId));
  //  message.payload = ((char*)payload);
  strcpy(message.payload, payload);
  //  message.payload = payload;
  // strlcpy(message.payload, payload, sizeof(message.payload));
}

bool Aloes::sendMessage(Config &config, Message &message ) {
  char topic[80];
  strlcpy(topic, config.mqttTopicOut, sizeof(config.mqttTopicOut));
  strcat(topic, "/");
  strcat(topic, message.method );
  strcat(topic, "/");
  strcat(topic, message.omaObjectId );
  strcat(topic, "/");
  strcat(topic, message.sensorId );
  strcat(topic, "/");
  strcat(topic, message.omaResourceId );
  if (transport.publish((const char*)topic, message.payload, false)) {
    return true;
  }
  return false;
}

bool Aloes::startStream(Config &config, Message &message, size_t length) {
  char topic[80];
  strlcpy(topic, config.mqttTopicOut, sizeof(config.mqttTopicOut));
  strcat(topic, "/");
  strcat(topic, message.method);
  strcat(topic, "/");
  strcat(topic, message.omaObjectId);
  strcat(topic, "/");
  strcat(topic, message.sensorId);
  strcat(topic, "/");
  strcat(topic, message.omaResourceId);
  if (transport.beginPublish((const char*)topic, length, false)) {
    return true;
  }
  return false;
}

size_t Aloes::writeStream(const uint8_t *payload, size_t length) {
  transport.write(payload, length);
}

bool Aloes::endStream() {
  if (transport.endPublish()) {
    return true;
  }
  return false;
}

// void Aloes::onReceive(char* topic, byte* payload, unsigned int length) {
//   payload[length] = '\0';
//   byte* p = (byte*)malloc(length);
//   memcpy(p, payload, length);
//   aSerial.v().println(F("====== Received message ======"));
//   aSerial.vv().p(F("Topic : ")).pln((const char*)topic).p(F("Payload : ")).pln((const char*)payload);
//   aloes->parseTopic(topic);
//   aloes->parseMessage(p);
//   free(p);
// }

void Aloes::parseTopic(char* topic) {
  char *str, *p;
  uint8_t i = 0;
  // first sanity check
  //  if (topic != strstr(topic, config.mqttTopicIn)) {
  //    Serial.print("error in the protocol");
  //    return;
  //  }
  // "pattern": "+prefixedDevEui/+method/+omaObjectId/+sensorId/+omaResourceId",
  for (str = strtok_r(topic + 1, "/", &p); str && i <= 4;
       str = strtok_r(NULL, "/", &p)) {
    switch (i) {
      case 0: {
          //prefixedDevEui
          break;
        }
      case 1: {
          strlcpy(message.method, str, sizeof(message.method));
          break;
        }
      case 2: {
          strlcpy(message.omaObjectId, str, sizeof(message.omaObjectId));
          break;
        }
      case 3: {
          strlcpy(message.sensorId, str, sizeof(message.sensorId));
          break;
        }
      case 4: {
          strlcpy(message.omaResourceId, str, sizeof(message.omaResourceId));
          break;
        }
    }
    i++;
  }
  aSerial.vvv().p(F("Method : ")).pln((const char*)message.method).p(F("For sensor : ")).pln((const char*)message.omaObjectId);
}


void Aloes::parseMessage(byte *payload) {
  message.payload = (char*)payload;
  for (size_t i = 0; i < sizeof(sensors) / sizeof(sensors[0]); i++) {
    const char** sensor = sensors[i];
    char objectId[5];
    char sensorId[4];
    char resourceId[5];
    //  printf("sensor[%u]\n", i);
    for (size_t j = 0; sensor[j]; j++) {
      if (j == 0 ) {
        strlcpy(objectId, sensor[j], sizeof(objectId));
      } else if (j == 1 ) {
        strlcpy(sensorId, sensor[j], sizeof(sensorId));
      }  else if (j == 2 ) {
        strlcpy(resourceId, sensor[j], sizeof(resourceId));
      }
    }
    if ( strcmp(message.sensorId, sensorId) == 0) {
      return onMessage(message);
    }
  }
}