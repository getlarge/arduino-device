#ifndef Aloes_h
#define Aloes_h

class Aloes {
public:
	Aloes();
	void getDeviceId(Config &config);
	void setSensors(Config &config);
	void setSensorRoutes(Config &config, const char* objectId, const char* sensorId, const char* resourceId, size_t index);
	void presentSensors(Config &config);
	void setMessage(Message &message, char method[5], char objectId[5], char sensorId[4], char resourceId[5], char payload[100]);
	bool sendMessage(Config &config, Message &message);
	bool startStream(Config &config, Message &message, size_t length);
	virtual size_t writeStream(const uint8_t *payload, size_t length);
	bool endStream();
	void parseTopic(char* topic);
	void parseMessage(byte* payload);
#if ALOES_RECEIVE == ON
	//	static void onReceive(char* topic, byte* payload, unsigned int length);
	void onMessage(Message &message);
#endif

private:
};

#endif
