#include "config.h"

#if CLIENT_SECURE == 1 && defined(HTTP_CLIENT_SECURE) || defined(MQTT_CLIENT_SECURE)
#include "secure-credentials.h"
#endif

#include <AloesDevice.h>

#include <SPI.h>
#include <MFRC522.h>

#if defined(ESP8266)
#define SS_PIN D8
#define RST_PIN D0
#elif defined(ESP32)
// todo fix pin config
#define SS_PIN 18
#define RST_PIN 10
#endif

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

bool readingTag = false;
bool writingTag = false;
bool goWriteTag = false;
byte nuidPICC[4];

void setupRFID();
void printHex();
void printDec();
bool readBlock(int blockId, byte* buffer, byte size);
bool readTag(char* sensorId);
bool parseResult(byte *buffer, byte bufferSize, char* sensorId);
bool writeBlock(int blockId, byte* buffer, size_t size);
bool writeTag();
bool writeTag(char* value);

void setupRFID() {
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  //  rfid.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details

  aSerial.vvv().pln(F("Init rfid, using the following key : "));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
  aSerial.vvv().pln(F(""));
}

void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    //  Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    //  Serial.print(buffer[i], HEX);
    aSerial.vvv().p(buffer[i] < 0x10 ? " 0" : " ");
    aSerial.vvv().p(buffer[i], HEX);
  }
}

void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    aSerial.vvv().p(buffer[i] < 0x10 ? " 0" : " ");
    aSerial.vvv().p(buffer[i], DEC);
  }
}

void arrayToString(byte array[], unsigned int length, char buffer[]) {
  for (unsigned int i = 0; i < length; i++) {
    byte nib1 = (array[i] >> 4) & 0x0F;
    byte nib2 = (array[i] >> 0) & 0x0F;
    buffer[i * 2 + 0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
    buffer[i * 2 + 1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
  }
  buffer[length * 2] = '\0';
}

bool parseResult(byte *buffer, byte bufferSize, char* sensorId) {
  //  aloes.setPayload(buffer, bufferSize, "DEC");
  char *newValue = new char[bufferSize * 2];
  arrayToString(buffer, bufferSize, newValue);
  aSerial.vvv().p(F("The NUID tag is : ")).pln(newValue);
  aloes.setMsg(PAYLOAD, newValue);
  aloes.setMsg(METHOD, "1").setMsg(OBJECT_ID, "3341").setMsg(SENSOR_ID, sensorId).setMsg(RESOURCE_ID, "5527");
  //  char* payload = aloes.getMsg(PAYLOAD);
  if (buffer[0] != nuidPICC[0] ||
      buffer[1] != nuidPICC[1] ||
      buffer[2] != nuidPICC[2] ||
      buffer[3] != nuidPICC[3] ) {
    aSerial.vvv().pln(F("A new card has been detected"));
    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = buffer[i];
    }
    //    aSerial.vvv().pln(F("The NUID tag is:"));
    //    aSerial.vvv().p(F("In hex : "));
    //    printHex(buffer, bufferSize);
    //    aSerial.vvv().pln(F(""));
    //    aSerial.vvv().p(F("In dec : "));
    //    printDec(buffer, bufferSize);
    //    aSerial.vvv().pln(F(""));

  } else aSerial.vvv().pln(F("Card read previously."));
  return true;
}

bool readBlock(int blockId, byte* buffer, byte size) {
  aSerial.vvvv().pln(F("Authenticating using key A..."));
  MFRC522::StatusCode status;
  status = (MFRC522::StatusCode) rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockId, &key, &(rfid.uid));
  if (status != MFRC522::STATUS_OK) {
    aSerial.vvv().p(F("PCD_Authenticate() failed: ")).pln(rfid.GetStatusCodeName(status));
    return false;
  }

  status = (MFRC522::StatusCode) rfid.MIFARE_Read(blockId, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    aSerial.vvv().p(F("MIFARE_Read() failed: ")).pln(rfid.GetStatusCodeName(status));
    return false;
  }
  aSerial.vvv().pln(F("MIFARE_Read() success"));
  return true;
}

bool readTag(char* sensorId) {
  if (writingTag) {
    return false;
  }
  if (!rfid.PICC_IsNewCardPresent()) {
    return false;
  }
  // Verify if the NUID has been readed
  if (!rfid.PICC_ReadCardSerial()) {
    return false;
  }

  readingTag = true;
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  aSerial.vvv().pln(rfid.PICC_GetTypeName(piccType));
  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    aSerial.vvv().pln(F("Your tag is not of type MIFARE Classic."));
    readingTag = false;
    return false;
  }

  int N = 7;
  char tagValue[N * 18];
  for (int i = 0; i < N; ++i) {
    int blockId = i + 8;
    if (blockId >= 11) blockId = i + 9;
    if (blockId > 14) break;
    aSerial.vvvv().p(F("READ BLOCKID :")).pln(blockId);
    byte buffer[18];
    byte size = sizeof(buffer);
    if (readBlock(blockId, buffer, size)) {
      int length = strlen((char*)buffer);
      char * tmp = (char*)buffer;
      tmp[length + 1] = '\0';
      aSerial.vvvv().p(F("Reading from tag : ")).pln(tmp).p(" LENGTH : ").pln(strlen(tmp));
      strcat(tagValue, tmp);
    }
  }
  aSerial.vv().p(F("Reading from tag : ")).p(tagValue).p(" Length : ").pln(strlen(tagValue));
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  if (parseResult(rfid.uid.uidByte, rfid.uid.size, sensorId)) {
    readingTag = false;
    return true;
  }
  readingTag = false;
  return false;
}

bool writeBlock(int blockId, byte* buffer, size_t size) {
  MFRC522::StatusCode status;
  aSerial.vvvv().pln(F("Authenticating using key A..."));
  status = (MFRC522::StatusCode) rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockId, &key, &(rfid.uid));
  if (status != MFRC522::STATUS_OK) {
    aSerial.vvv().p(F("PCD_Authenticate() failed: ")).pln(rfid.GetStatusCodeName(status));
    return false;
  }

  status = (MFRC522::StatusCode) rfid.MIFARE_Write(blockId, buffer, size);
  if (status != MFRC522::STATUS_OK) {
    aSerial.vvv().p(F("MIFARE_Write() failed: ")).pln(rfid.GetStatusCodeName(status));
    return false;
  }
  aSerial.vvv().pln(F("MIFARE_Write() success"));
  return true;
}

bool writeTag(char* value) {
  if (readingTag) {
    return false;
  }
  if (!rfid.PICC_IsNewCardPresent()) {
    return false;
  }
  // Verify if the NUID has been readed
  if (!rfid.PICC_ReadCardSerial()) {
    return false;
  }

  int maxBlockSize = 16;
  double length = strlen(value);
  double ratio = (length) / maxBlockSize;
  int N = ceil(ratio);
  aSerial.vv().p(F("Value to write : ")).p(value).p(F(" block counts :")).pln(N);

  writingTag = true;
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  aSerial.vvv().pln(rfid.PICC_GetTypeName(piccType));
  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    aSerial.vvv().pln(F("Your tag is not of type MIFARE Classic."));
    writingTag = false;
    return false;
  }

  //  Convert value to a list of N x 16 characters
  //  Write value over N separate blocks starting from 8th
  bool writeStatus = true;
  for (int i = 0; i < N; ++i) {
    int blockId = i + 8;
    if (blockId >= 11) blockId = i + 9;
    if (blockId > 14) break;
    unsigned char content[maxBlockSize];
    aSerial.vvvv().p(F("WRITE BLOCKID :")).pln(blockId);
    for (int j = 0; j < maxBlockSize; ++j) {
      int count = j + (maxBlockSize * i);
      if (count > length) break;
      if (count < length) {
        content[j] = value[count];
        if (j == maxBlockSize - 1) {
          content[j + 1] = '\0';
        }
      } else if (count == length) {
        content[j] = '\0';
      }
    }
    //  byte byteArray[maxBlockSize * 2];
    //  strcat((char*)(&byteArray[0]), content);
    aSerial.vvvv().p(F("Writing to tag : ")).pln((char*)content).p(" LENGTH : ").pln(strlen((char*)content));
    if (!writeBlock(blockId, content, maxBlockSize)) {
      writeStatus = false;
    }
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  writingTag = false;
  if (writeStatus == false) {
    return false;
  }
  return true;
}


//  CALLED on errors
void onError(modules module, char* error) {
  //  if (module == EXEC) {
  //    aSerial.vv().p(F("[EXEC] error : ")).pln(error);
  //  } else if (module == TRANSPORT) {
  //    aSerial.vv().p(F("[TRANSPORT] error : ")).pln(error);
  //  }
}

// CALLED on incoming http/mqtt/serial message
// MQTT pattern = "prefixedDevEui/+method/+objectId/+sensorId/+resourceId",
// HTTP pattern = "apiRoot/+collection/+path/#param"
void onMessage(transportLayer transportType, Message *message) {
  if (transportType == MQTT) {
    const char* method = message->getMethod();
    const char* objectId = message->getObjectId();
    const char* resourceId = message->getResourceId();
    const char* sensorId = message->getSensorId();
    char* payload = message->get(PAYLOAD);
    aSerial.vvv().p(F("onMessage : MQTT : ")).p(method).p(" ").pln(objectId);
    if ( strcmp(objectId, "3306") == 0 && strcmp(resourceId, "5850") == 0) {
      if ( strcmp(method, "1") == 0 ) {
        if ( ( strcmp(payload, "true") == 0 || strcmp(payload, "1") == 0 )) {
          digitalWrite(RELAY_SWITCH, HIGH);
          digitalWrite(STATE_LED, LOW);
          aSerial.vvv().p(F("Value to write : ")).pln(payload);

        } else if (( strcmp(payload, "false") == 0 ||  strcmp(payload, "0") == 0 )) {
          digitalWrite(RELAY_SWITCH, LOW);
          digitalWrite(STATE_LED, HIGH);
          aSerial.vvv().p(F("Value to write : ")).pln(payload);

        }
      } else if ( strcmp(method, "2") == 0 ) {
        int val = digitalRead(RELAY_SWITCH);
        char newPayload[10];
        itoa(val, newPayload, 10);
        //  message->setMethod((char*)"1");
        //  message->setPayload(newPayload);
        message->set(METHOD, "1").set(PAYLOAD, newPayload);
        if (aloes.sendMessage(MQTT)) {
          aSerial.vvv().p(F("Message sent"));
        } else {
          aSerial.vvv().p(F("Message failed to be sent"));
        }
      }
    } else if ( strcmp(objectId, "3341") == 0 && strcmp(resourceId, "5527") == 0) {
      if ( strcmp(method, "1") == 0 ) {
        goWriteTag = true;
      }
    } else if ( strcmp(objectId, "0") == 0 && strcmp(resourceId, "1234") == 0) {
      if ( strcmp(method, "1") == 0 ) {
        // updateFirmware = true;
        aloes.getFirmwareUpdate();
      }
    }
    return;
  }
  if (transportType == HTTP) {
    // if contenttype = json
    const char* method = message->getMethod();
    const char* collection = message->getCollection();
    const char* path = message->getPath();
    //  const char* body = message->getPayload();
    aSerial.vvv().p(F("onMessage : HTTP : ")).p(collection).p(" ").pln(path);
    return;
  }
}

void setup() {
  if (initDevice()) {
    setupRFID();
  }
}

void loop() {
  if (deviceRoutine()) {
    if (!goWriteTag) {
      if (readTag((char*)"3")) {
        aloes.sendMessage(MQTT);
        return;
      }
    } else if (goWriteTag) {
      char* payload = aloes.getMsg(PAYLOAD);
      if (!writeTag(payload)) {
        MilliSec lastWriteAttempt = millis();
        static AsyncWait retryWriteTag;
        if (writingTag == true) {
          goWriteTag = false;
          return;
        }
        if (retryWriteTag.isWaiting(lastWriteAttempt)) {
          return;
        }
        if (!writeTag(payload)) {
          retryWriteTag.startWaiting(lastWriteAttempt, 1000);
          return;
        }
      }
      goWriteTag = false;
    }
  }
}