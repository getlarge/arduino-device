#include "Transport.h"
#include "AsyncWait.h"

bool Transport::_httpSecure = false;
bool Transport::_mqttSecure = false;
bool Transport::reportErrors = false;

Transport::Transport() {

}

void Transport::setErrorCallback(ERROR_CALLBACK_SIGNATURE) {
  reportErrors = true;
  this->onError = onError;
}

void Transport::setError(char *error) {
  _error = error;
  if (reportErrors && strcmp(error, "") != 0) {
    onError(TRANSPORT, error);
  }
}

void Transport::setError(const char *error) {
  setError((char*)error);
}

bool Transport::setupHTTP(Device &device) {
  _userId = device.get(DEVICE_ID);
  _apiKey = device.get(API_KEY);
  _httpHost = device.get(HTTP_HOST);
  _httpPort = atoi(device.get(HTTP_PORT));
  _httpApiRoot = device.get(HTTP_API_ROOT);

  if (strcmp(device.get(HTTP_SECURE), "true") == 0) {
    _httpSecure = true;

#if CLIENT_SECURE == 1
    //  WiFiClientSecure _client;
    //  BearSSL::WiFiClientSecure *_client = new BearSSL::WiFiClientSecure();
    //  std::unique_ptr<BearSSL::WiFiClientSecure>_client(new BearSSL::WiFiClientSecure);
#if CA_CERT == 1
    BearSSL::X509List x509CaCert(CA_CERT_PROG);
    //  _client->setTrustAnchors(&x509CaCert);
    //  _client->allowSelfSignedCerts();
    _clientS.setCACert(&x509CaCert);
#endif
#if CLIENT_CERT == 1 && CLIENT_KEY == 1
    BearSSL::X509List x509ClientCert(CLIENT_CERT_PROG);
    BearSSL::PrivateKey PrivateClientKey(CLIENT_KEY_PROG);
    //  _client->setClientRSACert(&x509ClientCert, &PrivateClientKey);
    _clientS.setCertificate(&x509ClientCert);
    _clientS.setPrivateKey(&PrivateClientKey);
#endif
#if SERVER_KEY == 1
    BearSSL::PublicKey PublicServerKey(SERVER_KEY_PROG);
    //  _client->setKnownKey(&PublicServerKey);
#endif
#if FINGERPRINT == 1
    //  client->setFingerprint(fingerprint);
#endif
#if CA_CERT == 0 && CLIENT_CERT == 0 && CLIENT_KEY == 0 && SERVER_KEY == 0
    //  _client->setInsecure();
    _clientS.setInsecure();
    
  //  int mfln = _wifiClient.probeMaxFragmentLength(_httpHost, atoi(_httpPort), 1024);
  //  aSerial.vvv().p(F("MFLN status : ")).pln(_wifiClient.getMFLNStatus());
  //  if (mfln) {
  //      _wifiClient->setBufferSizes(1024, 1024);
  //  }
#endif
#endif
    aSerial.vvv().p(F("[HTTP] Init https://"));
  } else {
    _httpSecure = false;
    aSerial.vvv().p(F("[HTTP] Init http://"));
  }

  aSerial.vvv().p(_httpHost).p(":").pln(_httpPort);
  _httpClient.setReuse(false);
  httpConfigured = true;
  return httpConfigured;
}

bool Transport::setupMQTT(Device &device) {
  _mqttHost = device.get(MQTT_HOST);
  _mqttClientId = device.get(MQTT_CLIENT_ID);
  _mqttPort = atoi(device.get(MQTT_PORT));
  _mqttTopicIn = device.get(MQTT_TOPIC_IN);
  _mqttTopicOut = device.get(MQTT_TOPIC_OUT);

  if (strcmp(device.get(MQTT_SECURE), "true") == 0) {
    _mqttSecure = true;
#if CLIENT_SECURE == 1 
    _mqttClient.setClient(_client2S);
#if CA_CERT == 1
    BearSSL::X509List x509CaCert(CA_CERT_PROG);
    // _client2.setTrustAnchors(&x509CaCert);
    // _client2.allowSelfSignedCerts();
    _client2S.setCACert(&x509CaCert);
#endif
#if CLIENT_CERT == 1 && CLIENT_KEY == 1
    BearSSL::X509List x509ClientCert(CLIENT_CERT_PROG);
    BearSSL::PrivateKey PrivateClientKey(CLIENT_KEY_PROG);
    //  _client2.setClientRSACert(&x509ClientCert, &PrivateClientKey);
    _client2S.setCertificate(&x509ClientCert);
    _client2S.setPrivateKey(&PrivateClientKey);
#endif
#if SERVER_KEY == 1
    BearSSL::PublicKey PublicServerKey(SERVER_KEY_PROG);
    //  _client2.setKnownKey(&PublicServerKey);
#endif
#if CA_CERT == 0 && CLIENT_CERT == 0 && CLIENT_KEY == 0 && SERVER_KEY == 0
    _client2S.setInsecure();
  //  int mfln = _wifiClient.probeMaxFragmentLength(_mqttHost, atoi(_mqttPort), 1024);
  //  aSerial.vvv().p(F("MFLN status : ")).pln(_wifiClient.getMFLNStatus());
  //  if (mfln) {
  //      _wifiClient->setBufferSizes(1024, 1024);
  //  }
#endif

#else
    _mqttClient.setClient(_client2);
#endif
    aSerial.vvv().p(F("[MQTT] Init mqtts://"));
  } else {
    _mqttClient.setClient(_client2);
    _mqttSecure = false;
    aSerial.vvv().p(F("[MQTT] Init mqtt://"));
  }
// #if CLIENT_SECURE == 1 && defined(MQTT_CLIENT_SECURE)

//   aSerial.vvv().p(F("[MQTT] Init mqtts://"));
// // #elif CLIENT_SECURE == 1 && !defined(MQTT_CLIENT_SECURE)
// //   _client2.setInsecure();
// #else
//   aSerial.vvv().p(F("[MQTT] Init mqtt://"));
// #endif
  if (_mqttHost && _mqttPort) {
    aSerial.vvv().p(_mqttHost).p(":").pln(_mqttPort);
    _mqttClient.setServer(_mqttHost, _mqttPort);
    mqttConfigured = true;
  }
  return mqttConfigured;
}

void Transport::update(Device &device) {
  _userId = device.get(DEVICE_ID);
  _apiKey = device.get(API_KEY);
  _httpHost = device.get(HTTP_HOST);
  _httpPort = atoi(device.get(HTTP_PORT));
  _httpApiRoot = device.get(HTTP_API_ROOT);
  if (strcmp(device.get(HTTP_SECURE), "1") == 0) {
    _httpSecure = true;
  } else {
    _httpSecure = false;
  }

  _mqttHost = device.get(MQTT_HOST);
  _mqttClientId = device.get(MQTT_CLIENT_ID);
  _mqttPort = atoi(device.get(MQTT_PORT));
  _mqttTopicIn = device.get(MQTT_TOPIC_IN);
  _mqttTopicOut = device.get(MQTT_TOPIC_OUT);
  if (strcmp(device.get(MQTT_SECURE), "1") == 0) {
    _mqttSecure = true;
  } else {
    _mqttSecure = false;
  }
}

bool Transport::connectHTTP() {
  if (httpConfigured) {
    if (connected(HTTP)) {
      // if (configMode == 1) {
      //   callConfigMode = false;
      // }
      setError("");
      return true;
    }

    if (!_httpUrl) {
      _httpUrl = "/";
    }
    bool connected = false;
    if (_httpSecure) {
#if CLIENT_SECURE == 1
    //  if (_httpClient.begin(dynamic_cast<WiFiClient&>(*_client), _httpHost, _httpPort, url, true)) {
      aSerial.vvv().p(F("[HTTP] Connecting to https://")).p(_httpHost).p(F(":")).pln(_httpPort);
      if (_httpClient.begin(dynamic_cast<WiFiClient&>(_clientS), _httpHost, _httpPort, _httpUrl, true)) {
        connected = true;
      }

#else
      aSerial.vvv().p(F("[HTTP] Connecting to http://")).p(_httpHost).p(F(":")).pln(_httpPort);
      if (_httpClient.begin(_client, _httpHost, _httpPort, _httpUrl, false)) {
        connected = true;
      }  
#endif
    } else {
      aSerial.vvv().p(F("[HTTP] Connecting to http://")).p(_httpHost).p(F(":")).pln(_httpPort);
      if (_httpClient.begin(_client, _httpHost, _httpPort, _httpUrl, false)) {
        connected = true;
      }  
    }

    if (connected) {
      setError("");
      return true;
    }
    setError("Couldn't connect to HTTP Server");
    return false;
  }
  setError("HTTP Client not configured");
  return false;
}

bool Transport::connectMQTT() {
  if (mqttConfigured) {
    if (connected(MQTT)) {
      // if (configMode == 1) {
      //   callConfigMode = false;
      // }
      setError("");
      return true;
    }
    if (_mqttSecure) {
      aSerial.vvv().p(F("[MQTT] Connecting to mqtts://"));
    } else {
      aSerial.vvv().p(F("[MQTT] Connecting to mqtt://"));
    }
    aSerial.vvv().p(_mqttHost).p(":").pln(_mqttPort);

    // boolean connect (_mqttClientId, _userId, _apiKey, willTopic, willQoS, willRetain, willMessage, cleanSession)
    
    if (_mqttClient.connect(_mqttClientId, _userId, _apiKey)) {
      aSerial.vvv().p(F("[MQTT] Connected to broker as : ")).pln(_mqttClientId);
      mqttFailCount = 0;
      char masterTopic[60];
      strlcpy(masterTopic, _mqttTopicIn, sizeof(masterTopic));
      strcat(masterTopic, "/+/+/+/+" );
      subscribe((const char*)masterTopic, 0);
      setError("");
      return true;
    } 
    setError("MQTT connection failed");
    return false;
  }
  setError("MQTT Client not configured");
  return false;
}

bool Transport::connect(transportLayer transportType) {
  if (transportType == MQTT) {
    return connectMQTT();
  } else if (transportType == HTTP) {
    return connectHTTP();
  }
  setError("Invalid transport layer defined");
  return false;
}

bool Transport::asyncConnectHTTP(AsyncWait *async, MilliSec startTime, unsigned long interval) {
  if (async->isWaiting(startTime)) {
    return true;
  }
  if (connect(HTTP)) {
    httpFailCount = 0;
    async->cancel();
    return false;
  }
  if (httpFailCount > httpMaxFailedCount && !callConfigMode) {
    httpFailCount = 0;
    callConfigMode = true;
    async->cancel();
    return false;
  }
  ++httpFailCount;
  async->startWaiting(startTime, interval);
  return true;
}

bool Transport::asyncConnectMQTT(AsyncWait *async, MilliSec startTime, unsigned long interval) {
  if (async->isWaiting(startTime)) {
    return true;
  }
  if (connect(MQTT)) {
    mqttFailCount = 0;
    async->cancel();
    return true;
  }
  if (mqttFailCount > mqttMaxFailedCount && !callConfigMode) {
    mqttFailCount = 0;
    callConfigMode = true;
    //  async->cancel();
    //  return false;
  }
  ++mqttFailCount;
  async->startWaiting(startTime, interval);
  return true;
}

bool Transport::asyncConnect(transportLayer transportType, AsyncWait *async, MilliSec startTime, unsigned long interval) {
  if (transportType == MQTT) {
    return asyncConnectMQTT(async, startTime, interval);
  } else if (transportType == HTTP) {
    return asyncConnectHTTP(async, startTime, interval);
  }
  setError("Invalid transport layer defined");
  return false;
}

bool Transport::asyncConnect(transportLayer transportType, AsyncWait *async, MilliSec startTime) {
  if (transportType == MQTT) {
    return asyncConnectMQTT(async, startTime, mqttReconnectInterval);
  } else if (transportType == HTTP) {
    return asyncConnectHTTP(async, startTime, httpReconnectInterval);
  }
  setError("Invalid transport layer defined");
  return false;
}

void Transport::disconnect(transportLayer transportType) {
  if (transportType == MQTT) {
    _mqttClient.disconnect();
  } else if (transportType == HTTP) {
    _httpClient.end();
  } else {
    setError("Invalid transport layer defined");
  }
}

bool Transport::connected(transportLayer transportType) {
  if (transportType == MQTT) {
    return _mqttClient.connected();
  } else if (transportType == HTTP) {
    return _httpClient.connected();
  }
  setError("Invalid transport layer defined");
  return false;
}

void Transport::setMQTTCallback(MQTT_CALLBACK_SIGNATURE) {
  _mqttClient.setCallback(callback);
}

void Transport::setHTTPCallback(HTTP_CALLBACK_SIGNATURE) {
 this->httpCallback = httpCallback;
}

bool Transport::publish(const char* topic, const char* payload, bool retained) {
  if (connected(MQTT)) {
    if (_mqttClient.publish(topic, payload, retained)) {
      setError("");
      return true;
    }
    setError("MQTT publish failed");
    return false;
  }
  setError("MQTT client not connected");
  return false;
}

bool Transport::publish(const char* topic, const uint8_t *payload, size_t length, bool retained) {
  if (connected(MQTT)) {
    if (_mqttClient.publish(topic, payload, length, retained)) {
      setError("");
      return true;
    }
    setError("MQTT publish failed");
    return false;
  }
  setError("MQTT client not connected");
  return false;
}

bool Transport::subscribe(const char* topic, int qos) {
  if (!qos) {
    qos = 0;
  }
  bool substate = _mqttClient.subscribe(topic, qos);
  aSerial.vvv().p(F("[MQTT] Subscribing to ")).p(topic);
  if (substate) {
    aSerial.vvv().pln(" - success");
  } else {
    aSerial.vvv().pln(" - failure");
  }
  return (substate);
}

bool Transport::beginPublish(const char* topic, size_t length, bool retained) {
  if (_mqttClient.beginPublish(topic, length, retained)) {
    setError("");
    return true;
  }
  setError("MQTT Stream start failed");
  return false;
}

size_t Transport::write(const uint8_t *payload, size_t length) {
  _mqttClient.write(payload, length);
}

bool Transport::endPublish() {
  if (_mqttClient.endPublish()) {
    setError("");
    return true;
  }
  setError("MQTT Stream end failed");
  return false;
}


int Transport::sendRequest(int method, const char* payload) {
  switch(method) {
    case 1 : {
      return _httpClient.POST(payload);
    }
    case 2 : {
      return _httpClient.GET();
    }
    default : {
      setError("Invalid HTTP method");
    }
  }
}

void Transport::setRequestHeaders() {
  _httpClient.addHeader("Host", _httpHost);
  //  _httpClient.addHeader("User-Agent", device.getDeviceName());
  _httpClient.addHeader("Accept", "application/json, text/plain, */*");
  _httpClient.addHeader("ApiKey", _apiKey);
}

void Transport::setResponseHeaders(char* headers[][100]) {
  for (size_t j = 0; j < 1; j++) {
    const char *header = _httpClient.header(headers[j][0]).c_str(); 
    int i = 0;
    do {
      headers[j][1][i] = header[i];
    } while (header[i++] != '\0');
  }
  aSerial.vvvv().p(F("[HTTP] contentType : ")).pln(headers[0][1]);
}

void Transport::parseStream(const char *url, size_t length) {
  size_t bufferSize = length;
  unsigned char buffer[bufferSize];
  int pushedBufferSize = 0;
  Stream *_stream = _httpClient.getStreamPtr();
  //  _stream->setTimeout(bufferSize * 2);
  _stream->setTimeout(3000);

  String contentType = _httpClient.header("Content-Type");
  aSerial.vvvv().p(F("[HTTP] contentType : ")).pln(contentType);
  aSerial.vvvv().p(F("[HTTP] Body size : ")).pln(length);

  while (_httpClient.connected() && (length > 0 || length == -1)) {
    size_t size = _stream->available();
    if (size) {
      std::unique_ptr<char[]> tmpBuff(new char[size]);
      int c = _stream->readBytes(tmpBuff.get(), ((size > bufferSize) ? bufferSize : size));
      for (int i = 0; i <= c; i++) {
        if (bufferSize == length) {
          buffer[i] = tmpBuff.get()[i];
        } else {
          buffer[pushedBufferSize + i] = tmpBuff.get()[i];
        }
      }      
      pushedBufferSize += c;
      //  Serial.write(tmpBuff.get(), c);
      if (length > 0) {
        length -= c;
      }
    }
    delay(1);
  }
  if (httpCallback) {
    httpCallback((char*)url, buffer, bufferSize);  
  }
}

bool Transport::setRequest(const char* method, const char *url, const char *payload) {
  if (!url) {
    url = "/";
  }
  _httpUrl = url;
  //  todo :  if (3 first char of config.hhtpHost are letters) 
  // else try ip address
  // IPAddress httpHost;
  // httpHost.fromString(_httpHost);

  // _httpClient.setAuthorization(_apiKey);
  if (connect(HTTP)) {
    bool resSuccess = false;
  
    const char *keys[] = {"Content-Type"};
    //  aSerial.vvvv().p(F("[HTTP] header name : ")).pln(sizeof(keys[0]));
    // char* headers[][100] = {};
    // char** headers[60];
    // for (size_t j = 0; j < 1; j++) {
    //   int i = 0;
    //   //  headers[j][0][i] = new char[sizeof(keys[j]) + 1];
    //   headers[j][0] = new char[60];
    //   do {
    //     headers[j][0][i] = keys[j][i];
    //   } while (keys[j][i++] != '\0');
    //   //  aSerial.vvvv().p(F("[HTTP] header key : ")).pln(key);
    // }
    // aSerial.vvvv().p(F("[HTTP] header name : ")).pln(headers[0][0]);

    _httpClient.collectHeaders(keys, 1);
    
    setRequestHeaders();
    int httpCode = sendRequest(atoi(method), payload);

    if (httpCode > 0) {
      aSerial.vvv().p(F("[HTTP] Status : ")).pln(httpCode);
       // setResponseHeaders(headers);
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        int len = _httpClient.getSize();
        if (len != -1) {
          parseStream(url, len);
        } 
        setError("");
        resSuccess = true;
        //  aSerial.vvv().pln(F("[HTTP] connection closed or file end "));
      }
    } else {
      char err[150];
      strlcpy(err, _httpClient.errorToString(httpCode).c_str(), sizeof(err));
      setError(err);
      resSuccess = false;
    }
    _httpClient.end();
    return resSuccess;
  }
  setError("HTTP client not connected");
  return false;
}

void Transport::getUpdated(int which, const char* host, int port, const char* url) {
  if (_network.connected()) {
    helpers.startTick(0.7);
    //  otaSignal = 0;
#if MANUAL_SIGNING
   //  Update.installSignature(transport.hash, trasnport.sign);
#endif

#if defined(ESP8266) 
    //  updateFile(otaFile, otaSignal);
    //  ESPhttpUpdate.rebootOnUpdate(true);
    ESPhttpUpdate.setLedPin(STATE_LED, LOW);
    t_httpUpdate_return ret;

    if (_httpSecure) {
#if CLIENT_SECURE == 1
      ret = ESPhttpUpdate.update(_clientS, host, port, url);
#else
      ret = ESPhttpUpdate.update(_client, host, port, url);
#endif
    } else {
      ret = ESPhttpUpdate.update(_client, host, port, url);
    }

   switch (ret) {
     case HTTP_UPDATE_FAILED:
       aSerial.v().p(F("HTTP_UPDATE_FAILD Error : ")).p(ESPhttpUpdate.getLastError()).p(" / ").pln(ESPhttpUpdate.getLastErrorString().c_str());
       break;
     case HTTP_UPDATE_NO_UPDATES:
       aSerial.v().pln(F("HTTP_UPDATE_NO_UPDATES "));
       break;
     case HTTP_UPDATE_OK:
       aSerial.v().pln(F("HTTP_UPDATE_OK"));
       break;
   }

#elif defined(ESP32)
   // todo : implement https://github.com/espressif/arduino-esp32/blob/master/libraries/Update/examples/AWS_S3_OTA_Update/AWS_S3_OTA_Update.ino
   return;
#endif
    helpers.stopTick();
  }
}

bool Transport::loop(Device &device) {
  return _mqttClient.loop();
}
