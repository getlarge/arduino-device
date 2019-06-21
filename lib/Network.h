#ifndef Network_h
#define Network_h

#if defined(ESP8266) 
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#elif defined(ESP32)
#include <ESPmDNS.h>
#include <WiFiMulti.h>
#endif

class Network {
public:
	Network() { };
    virtual ~Network() { }
 //    ESP8266WiFiMulti & getWiFiClient() {
 //       return WiFiMulti;
	// }
	//	WiFiClient getWiFiClient();

	void setErrorCallback(ERROR_CALLBACK_SIGNATURE);
	void setError(char* error);
	void setError(const char* error);
	
	bool isReady();
    void setClock();
	void checkSerial();

	bool asyncConnect(AsyncWait *async, MilliSec startTime);
	bool asyncConnect(AsyncWait *async, MilliSec startTime, unsigned long interval);
	bool connect();
	bool connected();

	void generateId(Device &device);
 	String getMacAddress();
 	String getDevEui();

	bool setup(Device &device);
	bool loop();

	int wifiFailCount = 0;
	int wifiMaxFailedCount = 30;

private:
	ERROR_CALLBACK_SIGNATURE;
    const char *_error;
	static bool reportErrors;

#if defined(ESP8266) 
	ESP8266WiFiMulti wiFiMulti;
#elif defined(ESP32)
	WiFiMulti wiFiMulti
#endif
	unsigned long  reconnectInterval = 1000;
    const char *_ssid;
    const char *_password;
    const char *_hostname;

    const char *signPubKey;
    const char *sign;

    // Note: variables of type 'status' are NEVER assigned a value of 'FINISHED'.
    // 'FINISHED' is only used is comparisons (e.g. if(x_status>FINISHED)... ),
    // rather than always testing for SUCCESS || FAILED.
    enum status { NOT_STARTED, STARTED, FINISHED, SUCCESS, FAILED };
    status setClock_status = NOT_STARTED;
    AsyncWait setClock_AsyncWait;

	void checkClockStatus();
};

#endif