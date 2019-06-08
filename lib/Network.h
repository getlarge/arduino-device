#ifndef Network_h
#define Network_h

#if NTP_SERVER == 1
#include <WiFiUdp.h>
#endif

#include "Helpers.h"
//	#include "Manager.h"

class Network {
public:
	Network(Config &config);
	void checkSerial();
	void connect(Config &config);
	bool reconnect();
#if NTP_SERVER == 1
	time_t getNtpTime();
	time_t prevDisplay = 0; // when the digital clock was displayed
	void digitalClockDisplay();
	void printDigits(int digits);
	void sendNTPpacket(IPAddress &address);
#endif
	void loop();
	int wifiFailCount = 0;
	int wifiMaxFailedCount = 30;

private:
	unsigned long  reconnectInterval = 500;
#if NTP_SERVER == 1
	WiFiUDP Udp;
	const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
	byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets
	static const char ntpServerName[] = "fr.pool.ntp.org";
	const int timeZone = 1;     // Central European Time
	unsigned int localPort = 8888;  // local port to listen for UDP packets
#endif

};

#endif