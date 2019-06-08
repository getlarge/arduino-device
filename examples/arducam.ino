#include "config.h"
// const char* sensors[][60] = {
//   { "3306", "1", "5850", "digital_input"},
//   { "3349", "2", "5910", "buffer_input"},
// };

#include <AloesDevice.h>

#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include "memorysaver.h" // config file in Arducam library

#if !(defined ESP8266 )
#error Please select the ArduCAM ESP8266 UNO board in the Tools/Board
#endif

#define CS 16

ArduCAM myCAM(OV2640, CS);

static const size_t camBufferSize = 2048; // 4096; //2048; //1024;
bool transmitNow = false, transmitStream = false;
bool timelapse = false, base64encoding = false;
int resolution = 6; int fpm = 3;
unsigned long minDelayBetweenframes = 1000, lastPictureAttempt = 0;
char camResolution[4] = "6";
char camFpm[4] = "3";

void arducamInit();
void setCamResolution(int reso);
void setFPM(int interv);
void startCapture();
void camCapture(ArduCAM myCAM, char *sensorId);
void serverCapture(ArduCAM myCAM, char *sensorId);
void serverStream(ArduCAM myCAM, char *sensorId);


void arducamInit() {
  uint8_t vid, pid;
  uint8_t temp;
#if defined(__SAM3X8E__)
  Wire1.begin();
#else
  Wire.begin();
#endif
  digitalWrite(STATE_LED, HIGH);
  pinMode(CS, OUTPUT);
  SPI.begin();
  SPI.setFrequency(4000000); //4MHz
  //Check if the ArduCAM SPI bus is OK
  myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM.read_reg(ARDUCHIP_TEST1);
  if (temp != 0x55) {
    aSerial.v().println(F("SPI1 interface Error!"));
    while (1);
  }

  //Check if the camera module type is OV2640
  myCAM.wrSensorReg8_8(0xff, 0x01);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
  if ((vid != 0x26 ) && (( pid != 0x41 ) || ( pid != 0x42 ))) {
    aSerial.v().print(F("Can't find OV2640 module! pid :")).println(String(pid));
  }
  else {
    aSerial.vvv().println(F("OV2640 detected."));
    myCAM.set_format(JPEG);
    //  myCAM.set_format(RAW);
    myCAM.InitCAM();
    //  setBufferSize(camBufferSize);
    setFPM(atoi(camFpm));
    setCamResolution(atoi(camResolution));
    myCAM.clear_fifo_flag();
  }
  //  Dir dir = SPIFFS.openDir("/pics");
  //  while (dir.next()) {
  //    fileCount++;
  //  }
  //  FSInfo fs_info;
  //  SPIFFS.info(fs_info);
  //  fileTotalKB = (int)fs_info.totalBytes;
  //  fileUsedKB = (int)fs_info.usedBytes;
}

/// CAM SETTINGS
void setCamResolution(int reso) {
  switch (reso) {
    case 0:
      myCAM.OV2640_set_JPEG_size(OV2640_160x120);
      aSerial.vvv().println(F("Resolution set to 160x120"));
      break;
    case 1:
      myCAM.OV2640_set_JPEG_size(OV2640_176x144);
      aSerial.vvv().println(F("Resolution set to 176x144"));
      break;
    case 2:
      myCAM.OV2640_set_JPEG_size(OV2640_320x240);
      aSerial.vvv().println(F("Resolution set to 320x240"));
      break;
    case 3:
      myCAM.OV2640_set_JPEG_size(OV2640_352x288);
      aSerial.vvv().println(F("Resolution set to 352x288"));
      break;
    case 4:
      myCAM.OV2640_set_JPEG_size(OV2640_640x480);
      aSerial.vvv().println(F("Resolution set to 640x480"));
      break;
    case 5:
      myCAM.OV2640_set_JPEG_size(OV2640_800x600);
      aSerial.vvv().println(F("Resolution set to 800x600"));
      break;
    case 6:
      myCAM.OV2640_set_JPEG_size(OV2640_1024x768);
      aSerial.vvv().println(F("Resolution set to 1024x768"));
      break;
    case 7:
      myCAM.OV2640_set_JPEG_size(OV2640_1280x1024);
      aSerial.vvv().println(F("Resolution set to 1280x1024"));
      break;
    case 8:
      myCAM.OV2640_set_JPEG_size(OV2640_1600x1200);
      aSerial.vvv().println(F("Resolution set to 1600x1200"));
      break;
  }
}

void setFPM(int interv) {
  aSerial.vvv().print(F("FPM set to : ")).println(interv);
  switch (interv) {
    case 0:
      minDelayBetweenframes = 1000;
      break;
    case 1:
      minDelayBetweenframes = (5 * 1000);
      break;
    case 2:
      minDelayBetweenframes = (10 * 1000);
      break;
    case 3:
      minDelayBetweenframes = (15 * 1000);
      break;
    case 4:
      minDelayBetweenframes = (30 * 1000);
      break;
  }
}

void startCapture() {
  myCAM.clear_fifo_flag();
  myCAM.start_capture();
}

void camCapture(ArduCAM myCAM, char* sensorId) {
  size_t len = myCAM.read_fifo_length();
  if (len >= 0x07ffff) {
    aSerial.vv().println(F("Over size."));
    return;
  } else if (len == 0 ) {
    aSerial.vv().println(F("Size is 0."));
    return;
  }
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();
#if !(defined (ARDUCAM_SHIELD_V2) && defined (OV2640_CAM))
  SPI.transfer(0xFF);
#endif
  aloes.setMessage(message, (char*)"1", (char*)"3349", sensorId, (char*)"5910", (char*)"");
  aloes.startStream(config, message, len);
  //  mqttClient.beginPublish(postTopics[1].c_str(), len, false);
  static uint8_t buffer[camBufferSize] = {0xFF};

  while (len) {
    size_t will_copy = (len < camBufferSize) ? len : camBufferSize;
    SPI.transferBytes(&buffer[0], &buffer[0], will_copy);
    aloes.writeStream(&buffer[0], will_copy);
    len -= will_copy;
  }
  aloes.endStream();
  myCAM.CS_HIGH();
}

void serverCapture(ArduCAM myCAM, char* sensorId) {
  startCapture();
  digitalWrite(STATE_LED, LOW);
  aSerial.vv().println(F("Start Capturing"));
  int total_time = 0;
  total_time = millis();
  while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK));
  total_time = millis() - total_time;
  aSerial.vvv().print(F("Capturing time (in ms) : ")).println(total_time, DEC);
  total_time = 0;
  aSerial.vv().println(F("Capture done!"));
  total_time = millis();
  camCapture(myCAM, sensorId);
  total_time = millis() - total_time;
  aSerial.vvv().print(F("Sending time (in ms) : ")).println(total_time, DEC);
  aSerial.vvv().print(F("after capture heap size : ")).println(ESP.getFreeHeap());
  digitalWrite(STATE_LED, HIGH);
}

void serverStream(ArduCAM myCAM, char* sensorId) {
  transmitNow = false;
  while (1) {
    startCapture();
    while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK));
    size_t len = myCAM.read_fifo_length();
    if (len >= 0x07ffff) {
      aSerial.vv().println(F("Over size."));
      continue;
    } else if (len == 0 ) {
      aSerial.vv().println(F("Size is 0."));
      continue;
    }
    myCAM.CS_LOW();
    myCAM.set_fifo_burst();
#if !(defined (ARDUCAM_SHIELD_V2) && defined (OV2640_CAM))
    SPI.transfer(0xFF);
#endif
    aloes.setMessage(message, (char*)"1", (char*)"3349", sensorId, (char*)"5910", (char*)"");
    aloes.startStream(config, message, len);
    //  aloes.startStream(config, postTopics[1].c_str(), len, false);
    static uint8_t buffer[camBufferSize] = {0xFF};

    while (len) {
      size_t will_copy = (len < camBufferSize) ? len : camBufferSize;
      SPI.transferBytes(&buffer[0], &buffer[0], will_copy);
      aloes.writeStream(&buffer[0], will_copy);
      len -= will_copy;
    }
    aloes.endStream();
    myCAM.CS_HIGH();
  }
}

//  CALLED on incoming mqtt/serial message
void Aloes::onMessage(Message &message) {
  //  aSerial.vv().p(F("onMessage : ")).pln(message.sensorId);
  if ( strcmp(message.omaObjectId, "3306") == 0 && strcmp(message.omaResourceId, "5850") == 0) {
    if ( strcmp(message.method, "1") == 0 ) {
      if ( ( strcmp(message.payload, "true") == 0 || strcmp(message.payload, "1") == 0 )) {
        digitalWrite(RELAY_SWITCH, HIGH);
        digitalWrite(STATE_LED, LOW);
      } else if (( strcmp(message.payload, "false") == 0 ||  strcmp(message.payload, "0") == 0 )) {
        digitalWrite(RELAY_SWITCH, LOW);
        digitalWrite(STATE_LED, HIGH);
      }
    } else if ( strcmp(message.method, "2") == 0 ) {
      int val = digitalRead(RELAY_SWITCH);
      char payload[10];
      itoa(val, payload, 10);
      aloes.setMessage(message, (char*)"1", message.omaObjectId, message.sensorId, message.omaResourceId, payload);
      aloes.sendMessage(config, message);
      return;
    }
  } else if ( strcmp(message.omaObjectId, "3349") == 0 && strcmp(message.omaResourceId, "5911") == 0) {
    if ( strcmp(message.method, "1") == 0 ) {
      if ( strcmp(message.payload, "true") == 0 ) {
        return serverCapture(myCAM, (char*)message.sensorId);
        //  return;
      }
    } else if ( strcmp(message.method, "4") == 0 ) {
      if ( strcmp(message.payload, "1") == 0 ) {
        transmitStream = true;
        serverStream(myCAM, message.sensorId);
        return;
      } else if ( strcmp(message.payload, "0") == 0) {
        transmitStream = false;
        return;
      }
    }
  } else if ( ( strcmp(message.method, "1") == 0 ) && ( strcmp(message.omaObjectId, "2000") == 0 ) ) {
    if ( strcmp(message.omaResourceId, "reso") == 0 ) {
      //  int reso = atoi(message.payload);
      const char* reso = message.payload;
      aSerial.vv().p(F("Change resolution to : ")).pln(reso);
      if ( atoi(reso) >= 0 || atoi(reso) <= 8 ) {
        strlcpy(camResolution, reso,  sizeof(camResolution));
        setCamResolution(atoi(reso));
        //  return saveConfig(configFileName, config);
      }
    }
    if ( strcmp(message.omaResourceId, "fpm") == 0 ) {
      //  int fpm = atoi(message.payload);
      const char* fpm = message.payload;
      aSerial.vv().p(F("Change FPM to : ")).pln(fpm);
      if ( atoi(fpm) >= 0 || atoi(fpm) <= 4 ) {
        strlcpy(camFpm, fpm,  sizeof(camFpm));
        setFPM(atoi(fpm));
        //  return saveConfig(configFileName, config);
      }
    }
    if ( strcmp(message.omaResourceId, "update") == 0 ) {
      if (strcmp(message.payload, "ota") == 0) {
        //  strtok(s, "-");
        //  extract otaSignal, otaType, otaUrl, and fingerprint ( for httpsUpdate )
        //  if the int correspond to waited value, update otaSignal, otaFile and setReboot()
        //  return getUpdated();
      }

    }
  }
}

void setup() {
  initDevice();
  arducamInit();
}

void loop() {
  deviceRoutine();
}
