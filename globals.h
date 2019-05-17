bool resetConfig = false, wifiResetConfig = false; // set to true to reset FS and/or Wifimanager, don't forget to set this to false after
bool shouldSaveConfig = true, executeOnce = false, manualConfig = false;
int wifiFailCount = 0, wifiMaxFailedCount = 30, mqttFailCount = 0, mqttMaxFailedCount = 60;
int configCount = 0, configMode = 0, buttonState;
unsigned long configTimeout = 120, reconnectInterval = 500, debouncerInterval = 2000;
unsigned long lastMqttReconnectAttempt = 0, lastWifiReconnectAttempt = 0, buttonPressTimeStamp;

//// FILE / STREAM MANAGER
static const size_t objBufferSize = 512;
static const int fileSpaceOffset = 700000;
const String otaFile = "ota.txt";
const String configFileName = "config.json";
int fileTotalKB = 0;
int fileUsedKB = 0;
int fileCount = 0;
String errMsg = "";
int otaSignal = 0;

//// NTP
#if NTP_SERVER == 1
static const char ntpServerName[] = "fr.pool.ntp.org";
const int timeZone = 1;     // Central European Time
unsigned int localPort = 8888;  // local port to listen for UDP packets
#endif

//  "pattern": "+prefixedDevEui/+method/+omaObjectId/+sensorId/+omaResourcesId",
struct Message {
  char method[5];
  char omaObjectId[5];
  char sensorId[4];
  char omaResourceId[5];
  char* payload;
};

String postTopics[50] = {};

ESP8266WiFiMulti WiFiMulti;
#if NTP_SERVER == 1
WiFiUDP Udp;
#endif
#if CLIENT_SECURE == 0
WiFiClient wifiClient;
#elif CLIENT_SECURE == 1
WiFiClientSecure wifiClient;
#endif
#if WEB_SERVER == 1
ESP8266WebServer server(80);
#endif
#if MQTT_CLIENT == 1
PubSubClient mqttClient(wifiClient);
#endif
Bounce debouncer = Bounce();
Ticker ticker;
Config config;
Message message;
WiFiManager wifiManager;

WiFiManagerParameter customMqttServer("server", "mqtt server", config.mqttServer, sizeof(config.mqttServer));
WiFiManagerParameter customMqttPort("port", "mqtt port", config.mqttPort, sizeof(config.mqttPort));
WiFiManagerParameter customMqttUser("user", "mqtt user", config.mqttUser, sizeof(config.mqttUser));
WiFiManagerParameter customMqttPassword("password", "mqtt password", config.mqttPassword, sizeof(config.mqttPassword));


// UTILS
void loadConfig(const String filename, Config &config);
void saveConfig(const String filename, Config &config);
void initDefaultConfig(const String filename, Config &config);
void updateFile(const String fileName, int value);
void tick();
void setPins();
void checkButton();
void setReboot();
void setDefault();
void setPinsRebootUart();
//
//// NETWORK
void checkSerial();
void connectWifi(Config &config);
void reconnectWifi();
void getUpdated(int which, const char* url, const char* fingerprint);

// SETTINGS
void saveConfigCallback();
void quitConfigMode();
void configModeCallback (WiFiManager *myWiFiManager);
void initConfigManager(Config &config);
void configManager(Config &config);

// ALOES
void getDeviceId(Config &config);
void setSensors(Config &config);
void setSensorRoutes(Config &config, const char* objectId, const char* sensorId, const char* resourceId, size_t index);
void presentSensors(Config &config);
void setMessage(Message &message, char method[5], char* objectId, char* sensorId, char* resourceId, char* payload );
void sendMessage(Config &config, Message &message );
void parseMessage(Message &message);
void parseTopic(Message &message, char* topic);
void deviceInit();
void deviceSetup();
void beforeSetup();
void afterSetup();
void deviceLoop();
void beforeLoop();
void afterLoop();
void onMessage(Message &message, char sensorId[4]);

#if MQTT_CLIENT == 1
// MQTT
void generateMqttClientId(Config &config);
void mqttInit(Config &config);
void mqttError();
void mqttReconnect(Config &config);
void reconnectMqtt(Config &config);
void mqttCallback(char* topic, byte* payload, unsigned int length);
#endif

#if WEB_SERVER == 1
void handleNotFound();
#endif

#if NTP_SERVER == 1
time_t getNtpTime();
time_t prevDisplay = 0; // when the digital clock was displayed
void digitalClockDisplay();
void printDigits(int digits);
void sendNTPpacket(IPAddress &address);
#endif
