#ifndef Manager_h
#define Manager_h

// #include <ESP8266WebServer.h>
// #include <WiFiManager.h>
// #include <ArduinoJson.h>

#include "Network.h"
#include "Helpers.h"
#include "Transport.h"

#define DEFAULT_PORTAL_TIMEOUT 120

class Manager {
public:
	Manager(Config &config);
	void loadConfig(const String filename, Config &config);
	void initDefaultConfig(const String filename, Config &config);
	void saveConfig(const String filename, Config &config);
	static void quitConfigMode();
	static void configModeCallback(WiFiManager *myWiFiManager);
	static void saveConfigCallback();
	void updateFile(const String fileName, int value);
	void configManager(Config &config);
	void getUpdated(int which, const char* url, const char* fingerprint);
	void loop();
	const String configFileName = "config.json";
	const String otaFile = "ota.txt";
	int otaSignal = 0;
	int configCount = 0;

private:
	//	WiFiManager wifiManager;
	static const size_t objBufferSize = 512;
	static const int fileSpaceOffset = 700000;
	int fileTotalKB = 0;
	int fileUsedKB = 0;
	int fileCount = 0;
	String errMsg = "";
	static bool shouldSaveConfig;
	//	bool manualConfig = false;
};

#endif