/*
 * Manager.h
 *
 * Author:  Getlarge
 * Created: 2019-06-10
 */

 #ifndef Manager_h
#define Manager_h

#include <WiFiManager.h>

#define DEFAULT_PORTAL_TIMEOUT 60

WiFiManager wifiManager;

class Manager {
public:
	Manager(Device &device);

	void setup();
	void loop(Device &device);

	bool setConfig();
	static void configModeCallback(WiFiManager *myWiFiManager);
	static void saveConfigCallback();
	static void saveParamsCallback();
	static void bindServerCallback();
	static void handleRoute();
	const char* getParam(String name);
	
	void startPortal();
	static void closePortal();
	void stopPortal();

	const String otaFile = "ota.txt";
	int otaSignal = 0;
 
private:
	//	WiFiManager wifiManager;
	WiFiManagerParameter *customDeviceName;
	WiFiManagerParameter *customDeviceId;
	WiFiManagerParameter *customDeviceApiKey;

	WiFiManagerParameter *customHttpHost;
	WiFiManagerParameter *customHttpPort;
	WiFiManagerParameter *customHttpSecure;

	WiFiManagerParameter *customMqttHost;
	WiFiManagerParameter *customMqttPort;
	WiFiManagerParameter *customMqttSecure;

	static bool portalRunning;
	static bool startAP;
	static unsigned int startTime;
	static unsigned int portalTimeout;
	static int configCount;
	static bool shouldSaveConfig;
	static bool shouldSetConfig;
	
	const char* _portalScript = "<script>document.addEventListener(`DOMContentLoaded`,function(){if(window.location.pathname===`/param`){try{var paramsKV=window.location.search.substring(1).split(`&`);if(!paramsKV||paramsKV.length<1){return}paramsKV.forEach(function(paramKV){console.log(`set param : `,paramKV);var paramKey=paramKV.split(`=`)[0];var paramValue=paramKV.split(`=`)[1];if(paramKey && document.getElementById(paramKey)){document.getElementById(paramKey).value=paramValue}return})}catch(e){console.log(`WARNING`,e)}}})</script>";
	static const int fileSpaceOffset = 700000;
	int fileTotalKB = 0;
	int fileUsedKB = 0;
	int fileCount = 0;
	String errMsg = "";
};

#endif