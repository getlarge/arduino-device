#include <time.h>
#include "Network.h"
#include "AsyncWait.h"

bool Network::reportErrors = false;

void Network::setErrorCallback(ERROR_CALLBACK_SIGNATURE) {
  reportErrors = true;
  this->onError = onError;
}

void Network::setError(char *error) {
  _error = error;
  if (reportErrors && strcmp(error, "") != 0) {
    onError(NETWORK, error);
  }
}

void Network::setError(const char *error) {
  setError((char*)error);
}

// Set time via NTP, as required for x.509 validation
void Network::setClock() {
  configTime(2 * 3600, 0, "0.fr.pool.ntp.org", "1.fr.pool.ntp.org");
  setClock_status = STARTED;
  aSerial.vv().pln(F("Waiting for NTP time sync "));
  setClock_AsyncWait.startWaiting(millis(), 1000); // Log every second.
}


// Check Clock Status and update 'setClock_status' accordingly.
void Network::checkClockStatus() {
  time_t now = time(nullptr);
  if (now < 8 * 3600 * 2) {
      // The NTP request has not yet completed.
      if (!setClock_AsyncWait.isWaiting(millis())) {
        aSerial.vv().p(F("."));
        setClock_AsyncWait.startWaiting(millis(), 1000); // Log every second.
      }
      //  setError("Check clock status");
      return;
  }

  setClock_status = SUCCESS;
  //  wifiClient.setX509Time(now);
#if DEBUG > 0
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  aSerial.vv().pln(F("")).p(F("Current time: ")).pln(asctime(&timeinfo));
#endif // DEBUG
}


void Network::checkSerial() {
  while (Serial.available() == 0 && millis() < 4000);
  //On timeout or availability of data, we come here.
  if (Serial.available() > 0) {
    //If data is available, we enter here.
    int test = Serial.read(); //We then clear the input buffer
    Serial.println("DEBUG"); //Give feedback indicating mode
    Serial.println(DEBUG);
    aSerial.on();
  } else {
    aSerial.off();
    //Serial.setDebugOutput(false);
  }
}

bool Network::isReady() {
  return (setClock_status > FINISHED);
}

bool Network::connect() {
  //  IPAddress _ip, _gw, _sn;
  //  _ip.fromString(device.getStaticIp());
  //  _gw.fromString(device.getStaticGw());
  //  _sn.fromString(device.getSstaticSn());
#if defined(ESP8266) 
  WiFi.hostname(_hostname);
#elif defined(ESP32)
  WiFi.setHostname(_hostname);
#endif

  WiFi.mode(WIFI_STA);
  //  WiFi.begin(ssid, password);
  wiFiMulti.addAP(_ssid, _password);
  // String ssid = WiFi.SSID();
  // String pass = WiFi.psk();

  if (connected()) {
    if (!MDNS.begin(_hostname)) {
      setError("Failed to set up MDNS responder");
      return false;
    } 
    setError("");
    wifiFailCount = 0;
    aSerial.vv().p(F("[NETWORK] WiFi connected. IP Address : ")).pln(WiFi.localIP());
    // if (configMode == 1) {quitConfigMode();}
    //  MDNS.addService("http", "tcp", 80);
    return true;
  }
  setError("Wifi connection failure");
  return false;
}

bool Network::asyncConnect(AsyncWait *async, MilliSec startTime, unsigned long interval) {
  if (async->isWaiting(startTime)) {
    return true;
  }
  if (connect()) {
    wifiFailCount = 0;
    return false;
  } 
  if (wifiFailCount > wifiMaxFailedCount && !callConfigMode) {
    wifiFailCount = 0;
    callConfigMode = true;
  } 
  ++wifiFailCount;
  async->startWaiting(startTime, interval);
  return true;    
}

bool Network::asyncConnect(AsyncWait *async, MilliSec startTime) {
  return asyncConnect(async, startTime, reconnectInterval);
}

bool Network::connected() {
  return (wiFiMulti.run() == WL_CONNECTED);
  //  return (WiFi.status() == WL_CONNECTED);
}

String Network::getMacAddress() {
    byte mac[6];
    String macStr;
    WiFi.macAddress(mac);
    macStr = String(mac[0], HEX) + ":"
           + String(mac[1], HEX) + ":"
           + String(mac[2], HEX) + ":"
           + String(mac[3], HEX) + ":"
           + String(mac[4], HEX) + ":"
           + String(mac[5], HEX);
    
    return macStr;
}

String Network::getDevEui() {
    byte mac[6];
    String devEui;
    WiFi.macAddress(mac);
    devEui = String(mac[0], HEX)
           + String(mac[1], HEX)
           + String(mac[2], HEX)
           + String(mac[3], HEX)
           + String(mac[4], HEX)
           + String(mac[5], HEX);
    devEui.toUpperCase();
    return devEui;
}

void Network::generateId(Device &device) {
#if ID_TYPE == 0
#if defined(ESP8266)
  char *espChipId;
  float chipId = ESP.getChipId();
  char chipIdBuffer[sizeof(chipId)];
  espChipId = dtostrf(chipId, sizeof(chipId), 0, chipIdBuffer);
  device.set(DEV_EUI, espChipId);
#elif defined(ESP32)
  uint64_t chipId;  
  chipId = ESP.getEfuseMac();//The chip ID is essentially its MAC address(length: 6 bytes).
  // Serial.printf("ESP32 Chip ID = %04X",(uint16_t)(chipid>>32));//print High 2 bytes
  // Serial.printf("%08X\n",(uint32_t)chipid);//print Low 4bytes.
#endif
#elif ID_TYPE == 1
  String devEui = getDevEui();
  //  aSerial.vvv().p(F("devEui : ")).pln(devEui);
  device.set(DEV_EUI, (char*)devEui.c_str());
#endif
  //    #if ID_TYPE == 2
  //// soyons fous, let's create an eui64 address ( like ipv6 )
  ////      Step #1: Split the MAC address in the middle:
  ////      Step #2: Insert FF:FE in the middle:
  ////      Step #4: Convert the first eight bits to binary:
  ////      Step #5: Flip the 7th bit:
  ////      Step #6: Convert these first eight bits back into hex:
  //    #endif
  //  aSerial.vvv().p(F("DeviceID : ")).pln(config.devEui);
  char mqttClientId[50];
  strcpy(mqttClientId, device.get(DEV_EUI));
  long randNumber = random(10000);
  char randNumberBuffer[10];
  ltoa(randNumber, randNumberBuffer, 10);
  strcat(mqttClientId, "-" );
  strcat(mqttClientId, randNumberBuffer);
  device.set(MQTT_CLIENT_ID, mqttClientId);
}

bool Network::setup(Device &device) {
  //  checkSerial();
  _hostname = device.get(DEVICE_NAME);
#ifdef DEFAULT_WIFI_SSID
  _ssid = DEFAULT_WIFI_SSID;
#else
  _ssid = WiFi.SSID().c_str();
#endif
#ifdef DEFAULT_WIFI_PASS
  _password = DEFAULT_WIFI_PASS;
#else
  _password = WiFi.psk().c_str();
#endif

  aSerial.vv().pln(F("")).p(F("Network setup : ")).p(_ssid).p(" ").pln(_password);
  if (connect()) {
    return true;
  }
  // randomSeed(micros());
  // setClock();

// #if MANUAL_SIGNING
//   signPubKey = new BearSSL::PublicKey(pubkey);
//   hash = new BearSSL::HashSHA256();
//   sign = new BearSSL::SigningVerifier(signPubKey);
// #endif

  // int numCerts = certStore.initCertStore(&certs_idx, &certs_ar);
  // client->setCertStore(&certStore);
  // Serial.println(numCerts);
  return false;
}

bool Network::loop() {
  // Prevent ALL other actions here until the clock as been set by NTP.
  if (setClock_status < FINISHED) {
    checkClockStatus();
    return false;
  }
  if (!isReady()) {
    return false;
  }
  return true;
}
