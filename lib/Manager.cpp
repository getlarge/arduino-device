#include "Manager.h"

bool Manager::portalRunning = false;
bool Manager::startAP = true;
bool Manager::shouldSaveConfig = false;
bool Manager::shouldSetConfig = false;
unsigned int Manager::startTime = millis();
unsigned int Manager::portalTimeout = DEFAULT_PORTAL_TIMEOUT * 1000;
int Manager::configCount = 0;

Manager::Manager(Device &device) {
  customDeviceName = new WiFiManagerParameter("device_name", "device name", device.get(DEVICE_NAME), device.getSize(DEVICE_NAME));
  customDeviceId = new WiFiManagerParameter("device_id", "device id", device.get(DEVICE_ID), device.getSize(DEVICE_ID));
  customDeviceApiKey = new WiFiManagerParameter("apikey", "apikey", device.get(API_KEY), device.getSize(API_KEY));
  customHttpHost = new WiFiManagerParameter("http_host", "http host", device.get(HTTP_HOST), device.getSize(HTTP_HOST));
  customHttpPort = new WiFiManagerParameter("http_port", "http port", device.get(HTTP_PORT), device.getSize(HTTP_PORT));
  customHttpSecure = new WiFiManagerParameter("http_secure", "http secure", device.get(HTTP_SECURE), device.getSize(HTTP_SECURE));
  customMqttHost = new WiFiManagerParameter("mqtt_host", "mqtt server", device.get(MQTT_HOST), device.getSize(MQTT_HOST));
  customMqttPort = new WiFiManagerParameter("mqtt_port", "mqtt port", device.get(MQTT_PORT), device.getSize(MQTT_PORT));
  customMqttSecure = new WiFiManagerParameter("mqtt_secure", "mqtt secure", device.get(MQTT_SECURE), device.getSize(MQTT_SECURE));
//  new (&customHttpPort) WiFiManagerParameter("http_port", "http port", config.httpPort, sizeof(config.httpPort));
  wifiManager.addParameter(customDeviceName);
  wifiManager.addParameter(customDeviceId);
  wifiManager.addParameter(customDeviceApiKey);
  wifiManager.addParameter(customHttpHost);
  wifiManager.addParameter(customHttpPort);
  wifiManager.addParameter(customHttpSecure);
  wifiManager.addParameter(customMqttHost);
  wifiManager.addParameter(customMqttPort);
  wifiManager.addParameter(customMqttSecure);
}

void Manager::saveConfigCallback() {
  shouldSaveConfig = true;
}

// const char* Manager::getParam(String name){
//   String value;
//   if(wifiManager.server->hasArg(name)) {
//     value = wifiManager.server->arg(name);
//   }
//   return value.c_str();
// }

bool Manager::setConfig() {
 // device.set("mqttHost", getParam("mqtt_host"));
  // todo validate settings
  device.set(DEVICE_NAME, customDeviceName->getValue()).set(API_KEY, customDeviceApiKey->getValue()).set(DEVICE_ID, customDeviceId->getValue());
  device.set(HTTP_HOST, customHttpHost->getValue()).set(HTTP_PORT, customHttpPort->getValue()).set(HTTP_SECURE, customHttpSecure->getValue());
  device.set(MQTT_HOST, customMqttHost->getValue()).set(MQTT_PORT, customMqttPort->getValue()).set(MQTT_SECURE, customMqttSecure->getValue());
  return true;
}

void Manager::saveParamsCallback () {
  aSerial.vv().pln(F("[MANAGER] saveParamsCallback"));
  shouldSetConfig = true;
}

void Manager::configModeCallback(WiFiManager *myWiFiManager) {
  configCount += 1;
  configMode = 1;
  portalRunning = true;
  startTime = millis();
  helpers.startTick(1);
}

void Manager::bindServerCallback() {
  // update timer to delay automatic portal stop
  startTime = millis();
  wifiManager.server->on("/exit", handleRoute);
}

void Manager::handleRoute() {
  //  aSerial.vv().pln(F("[MANAGER] handleRoute"));
  wifiManager.server->send(200, "text/plain", "Closing app now.");
  closePortal();
  //  portalTimeout = 0;
}

void Manager::startPortal() {
  customDeviceName->setValue(device.get(DEVICE_NAME), device.getSize(DEVICE_NAME));
  customDeviceId->setValue(device.get(DEVICE_ID), device.getSize(DEVICE_ID));
  customDeviceApiKey->setValue(device.get(API_KEY), device.getSize(API_KEY));
  customHttpHost->setValue(device.get(HTTP_HOST), device.getSize(HTTP_HOST));
  customHttpPort->setValue(device.get(HTTP_PORT), device.getSize(HTTP_PORT));
  customHttpSecure->setValue(device.get(HTTP_SECURE), device.getSize(HTTP_SECURE));
  customMqttHost->setValue(device.get(MQTT_HOST), device.getSize(MQTT_HOST));
  customMqttPort->setValue(device.get(MQTT_PORT), device.getSize(MQTT_PORT));
  customMqttSecure->setValue(device.get(MQTT_SECURE), device.getSize(MQTT_SECURE));

  if (startAP){
    aSerial.vv().pln(F("[MANAGER] Starting Config Portal"));
    wifiManager.startConfigPortal(device.get(DEVICE_NAME), device.get(DEVICE_PASS));
  } else {
    aSerial.vv().pln(F("[MANAGER] Starting Web Portal"));
    wifiManager.startWebPortal();
  }  
}

void Manager::closePortal() {
  if (startAP) {
    wifiManager.stopConfigPortal();
  } else {
    wifiManager.stopWebPortal();
  } 
  portalRunning = false;
  callConfigMode = false;
  configMode = 0;
  manualConfig = false;
  helpers.stopTick();
  digitalWrite(STATE_LED, HIGH);
  //  aSerial.vv().pln(F("[MANAGER] Config mode ended"));
}

void Manager::stopPortal() {
  if (!portalRunning) {
    return;
  } else if (portalRunning) {
    closePortal();
    return;
  }
}

void Manager::setup() {
  #if DEBUG >= 3
  wifiManager.setDebugOutput(true);
#elif DEBUG <= 2
  wifiManager.setDebugOutput(false);
#endif
  //  IPAddress _ip, _gw, _sn;
  //  _ip.fromString(config.staticIp);
  //  _gw.fromString(config.staticGw);
  //  _sn.fromString(config.staticSn);
  //  wifiManager.setSTAStaticIPConfig(_ip, _gw, _sn);
  wifiManager.setAPStaticIPConfig(IPAddress(192, 168, 244, 1), IPAddress(192, 168, 244, 1), IPAddress(255, 255, 255, 0));
  wifiManager.setBreakAfterConfig(true);
  wifiManager.setMinimumSignalQuality(10);

  wifiManager.setCustomHeadElement(_portalScript);
  std::vector<const char *> menu = {"wifi","info","param","sep","restart","exit"};
  wifiManager.setMenu(menu);
  // wifiManager.setClass("invert");

  //  wifiManager.setAPClientCheck(true); // avoid timeout if client connected to softap
  //  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setWebServerCallback(bindServerCallback);
  wifiManager.setConfigPortalBlocking(false);
  wifiManager.setSaveParamsCallback(saveParamsCallback);
  wifiManager.setAPCallback(configModeCallback);
}

void Manager::loop(Device &device) {
  if (portalRunning){
    wifiManager.process();
    if ((millis() - startTime) > portalTimeout){
      Serial.println("portaltimeout");
      stopPortal();
      return;
    } 
    if (!callConfigMode) {
      stopPortal();
      return;
    }
    if (shouldSetConfig) {
      if (setConfig()) {
        shouldSetConfig = false;
        shouldSaveConfig = true;
      }
    }
    if (shouldSaveConfig) {
      if (device.saveConfig()) {
        shouldSaveConfig = false;
      }
    }
  } else if (callConfigMode && !portalRunning) {
    if (manualConfig) {
      aSerial.vv().pln(F("[MANAGER] Manual config access"));
      portalTimeout = DEFAULT_PORTAL_TIMEOUT * 2 * 1000;
    } else {
      aSerial.vv().pln(F("[MANAGER] Network config access"));
      portalTimeout = DEFAULT_PORTAL_TIMEOUT * 1000;
      //  wifiManager.setConfigPortalTimeout(DEFAULT_PORTAL_TIMEOUT);
    }
    startPortal();
  }
}
