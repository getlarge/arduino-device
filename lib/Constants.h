#ifndef Constants_h
#define Constants_h

struct Message {
  char method[5];
  char omaObjectId[5];
  char sensorId[4];
  char omaResourceId[5];
  char* payload;
};

String postTopics[80] = {};

bool callConfigMode = false;
bool manualConfig = false;
int configMode = 0;
unsigned long debouncerInterval = 2000;

#endif
