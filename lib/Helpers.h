#ifndef Helpers_h
#define Helpers_h

#include <Ticker.h>
#include <Bounce2.h>

class Helpers {
public:
	Ticker ticker;
	Helpers();
	static void tick();
	void startTick(int duration);
	void stopTick();
	void generateId(Config &config);
	void setPins();
	void checkButton();
	void readConfigButton();
	void setReboot();
	void setDefault();
	void setPinsRebootUart();
	void loop();
private:
	Bounce debouncer = Bounce();
	int buttonState;
	unsigned long buttonPressTimeStamp;
};

#endif