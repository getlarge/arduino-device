#include "config.h"

#if CLIENT_SECURE == 1
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

#define NR_KNOWN_KEYS 8
// Known keys, see:
// https://code.google.com/p/mfcuk/wiki/MifareClassicDefaultKeys
byte knownKeys[NR_KNOWN_KEYS][MFRC522::MF_KEY_SIZE] = {
    {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, // FF FF FF FF FF FF = factory default
    {0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5}, // A0 A1 A2 A3 A4 A5
    {0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5}, // B0 B1 B2 B3 B4 B5
    {0x4d, 0x3a, 0x99, 0xc3, 0x51, 0xdd}, // 4D 3A 99 C3 51 DD
    {0x1a, 0x98, 0x2c, 0x7e, 0x45, 0x9a}, // 1A 98 2C 7E 45 9A
    {0xd3, 0xf7, 0xd3, 0xf7, 0xd3, 0xf7}, // D3 F7 D3 F7 D3 F7
    {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}, // AA BB CC DD EE FF
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}  // 00 00 00 00 00 00
};

bool readingTag = false;
bool writingTag = false;
bool goWriteTag = false;
byte nuidPICC[4];
char lastUid[10];

void setupRFID();
void printHex();
void printDec();
bool tryKey(MFRC522::MIFARE_Key *key);
bool readBlock(MFRC522::MIFARE_Key *key, int blockId, byte *buffer, byte size);
bool readTag(int from);
bool readUid();
char *parseUid(byte *buffer, byte bufferSize);
bool writeBlock(MFRC522::MIFARE_Key *key, int blockId, byte *buffer,
                size_t size);
bool writeTag();
bool writeTag(char *value, int from);

void setupRFID() {
  SPI.begin();     // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522
}

void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
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
    buffer[i * 2 + 0] = nib1 < 0xA ? '0' + nib1 : 'A' + nib1 - 0xA;
    buffer[i * 2 + 1] = nib2 < 0xA ? '0' + nib2 : 'A' + nib2 - 0xA;
  }
  buffer[length * 2] = '\0';
}

char *parseUid(byte *buffer, byte bufferSize) {
  //  aloes.setPayload(buffer, bufferSize, "DEC");
  if (buffer[0] != nuidPICC[0] || buffer[1] != nuidPICC[1] ||
      buffer[2] != nuidPICC[2] || buffer[3] != nuidPICC[3]) {
    aSerial.vvv().pln(F("A new card has been detected"));
    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = buffer[i];
    }
    char *newUid = new char[bufferSize * 2];
    arrayToString(buffer, bufferSize, newUid);
    aSerial.vvv().p(F("The NUID tag is : ")).pln(newUid);
    //    aSerial.vvv().pln(F("The NUID tag is:"));
    //    aSerial.vvv().p(F("In hex : "));
    //    printHex(buffer, bufferSize);
    //    aSerial.vvv().pln(F(""));
    //    aSerial.vvv().p(F("In dec : "));
    //    printDec(buffer, bufferSize);
    //    aSerial.vvv().pln(F(""));
    strlcpy(lastUid, newUid, strlen(newUid) + 1);
    return lastUid;
  } else {
    aSerial.vvv().pln(F("Card read previously."));
    return lastUid;
  }
}

bool readBlock(MFRC522::MIFARE_Key *key, int blockId, byte *buffer, byte size) {
  aSerial.vvvv().pln(F("Authenticating using key A..."));
  MFRC522::StatusCode status;
  status = (MFRC522::StatusCode)rfid.PCD_Authenticate(
      MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockId, key, &(rfid.uid));
  if (status != MFRC522::STATUS_OK) {
    aSerial.vvv()
        .p(F("PCD_Authenticate() failed: "))
        .pln(rfid.GetStatusCodeName(status));
    return false;
  }

  status = (MFRC522::StatusCode)rfid.MIFARE_Read(blockId, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    aSerial.vvv()
        .p(F("MIFARE_Read() failed: "))
        .pln(rfid.GetStatusCodeName(status));
    return false;
  }
  aSerial.vvv().pln(F("MIFARE_Read() success"));
  return true;
}

bool tryKey(MFRC522::MIFARE_Key *key) {
  bool result = false;
  byte buffer[18];
  byte block = 0;
  MFRC522::StatusCode status;
  // aSerial.vvv().pln(F("Try to read RFID, using the following key : "));

  // Serial.println(F("Authenticating using key A..."));
  status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, key,
                                 &(rfid.uid));
  if (status != MFRC522::STATUS_OK) {
    // Serial.print(F("PCD_Authenticate() failed: "));
    // Serial.println(mfrc522.GetStatusCodeName(status));
    return false;
  }

  // Read block
  byte byteCount = sizeof(buffer);
  status = rfid.MIFARE_Read(block, buffer, &byteCount);
  if (status != MFRC522::STATUS_OK) {
    // Serial.print(F("MIFARE_Read() failed: "));
    // Serial.println(rfid.GetStatusCodeName(status));
  } else {
    // Successful read
    result = true;
    aSerial.vvv().pln(F("Succes with key :"));
    printHex((*key).keyByte, MFRC522::MF_KEY_SIZE);
    // Dump block data
    aSerial.vvv().p(F("Block :")).p(block).pln(" : ");
    printHex(buffer, 16);
  }

  rfid.PICC_HaltA();      // Halt PICC
  rfid.PCD_StopCrypto1(); // Stop encryption on PCD
  return result;
}

bool readTag(int from) {
  if (writingTag) {
    return false;
  }

  MFRC522::MIFARE_Key key;
  bool foundKey = false;
  for (byte k = 0; k < NR_KNOWN_KEYS; k++) {
    for (byte i = 0; i < MFRC522::MF_KEY_SIZE; i++) {
      key.keyByte[i] = knownKeys[k][i];
    }
    if (tryKey(&key)) {
      foundKey = true;
      break;
    }
    if (!rfid.PICC_IsNewCardPresent())
      break;
    if (!rfid.PICC_ReadCardSerial())
      break;
  }
  if (!foundKey)
    return false;
  //  if (!rfid.PICC_IsNewCardPresent()) {
  //    return false;
  //  }
  //  // Verify if the NUID has been readed
  //  if (!rfid.PICC_ReadCardSerial()) {
  //    return false;
  //  }

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

  bool readingStatus = true;
  int N = 7;
  char tagValue[N * 16];
  for (int i = 0; i < N; ++i) {
    int blockId = i + from;
    if (blockId >= 11)
      blockId = i + from + 1;
    if (blockId > 14)
      break;
    aSerial.vvvv().p(F("READ BLOCKID :")).pln(blockId);
    byte buffer[18];
    byte size = sizeof(buffer);
    if (readBlock(&key, blockId, buffer, size)) {
      int length = strlen((char *)buffer);
      char *tmp = (char *)buffer;
      tmp[length + 1] = '\0';
      aSerial.vvvv()
          .p(F("Reading from block: "))
          .p(blockId)
          .p(F(" content : "))
          .p(tmp)
          .p(" Length : ")
          .pln(strlen(tmp));
      strcat(tagValue, tmp);
    } else {
      readingStatus = false;
    }
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  readingTag = false;
  if (readingStatus == false) {
    return false;
  }
  aSerial.vv()
      .p(F("Reading from tag : "))
      .p(tagValue)
      .p(" Length : ")
      .pln(strlen(tagValue));
  char *uid = parseUid(rfid.uid.uidByte, rfid.uid.size);
  char payload[strlen(tagValue) + strlen(uid) + 3];
  strlcpy(payload, uid, strlen(uid) + 1);
  if (strlen(tagValue) > 0) {
    strcat(payload, "-");
    strcat(payload, tagValue);
  }
  //  aSerial.vv().p(F("PAYLOAD ")).p(payload).p(" Length :
  //  ").pln(strlen(payload));
  aloes.setMsg(PAYLOAD, payload);
  return true;
}

bool readUid() {
  if (writingTag) {
    return false;
  }
  MFRC522::MIFARE_Key key;
  bool foundKey = false;
  for (byte k = 0; k < NR_KNOWN_KEYS; k++) {
    for (byte i = 0; i < MFRC522::MF_KEY_SIZE; i++) {
      key.keyByte[i] = knownKeys[k][i];
    }
    if (tryKey(&key)) {
      foundKey = true;
      break;
    }
    if (!rfid.PICC_IsNewCardPresent()) {
      break;
    }
    if (!rfid.PICC_ReadCardSerial()) {
      break;
    }
  }
  if (!foundKey) {
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

  char *uid = parseUid(rfid.uid.uidByte, rfid.uid.size);
  //  aSerial.vv().p(F("PAYLOAD ")).p(payload).p(" Length :
  //  ").pln(strlen(payload));
  aloes.setMsg(PAYLOAD, uid);
  return true;
}

bool writeBlock(MFRC522::MIFARE_Key *key, int blockId, byte *buffer,
                size_t size) {
  MFRC522::StatusCode status;
  aSerial.vvvv().pln(F("Authenticating using key A..."));
  status = (MFRC522::StatusCode)rfid.PCD_Authenticate(
      MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockId, key, &(rfid.uid));
  if (status != MFRC522::STATUS_OK) {
    aSerial.vvv()
        .p(F("PCD_Authenticate() failed: "))
        .pln(rfid.GetStatusCodeName(status));
    return false;
  }

  status = (MFRC522::StatusCode)rfid.MIFARE_Write(blockId, buffer, size);
  if (status != MFRC522::STATUS_OK) {
    aSerial.vvv()
        .p(F("MIFARE_Write() failed: "))
        .pln(rfid.GetStatusCodeName(status));
    return false;
  }
  aSerial.vvv().pln(F("MIFARE_Write() success"));
  return true;
}

bool writeTag(char *value, int from) {
  if (readingTag) {
    return false;
  }

  MFRC522::MIFARE_Key key;
  bool foundKey = false;
  for (byte k = 0; k < NR_KNOWN_KEYS; k++) {
    // Copy the known key into the MIFARE_Key structure
    for (byte i = 0; i < MFRC522::MF_KEY_SIZE; i++) {
      key.keyByte[i] = knownKeys[k][i];
    }
    if (tryKey(&key)) {
      foundKey = true;
      // Found and reported on the key and block,
      // no need to try other keys for this PICC
      break;
    }
    if (!rfid.PICC_IsNewCardPresent()) {
      break;
    }
    if (!rfid.PICC_ReadCardSerial()) {
      break;
    }
  }
  if (!foundKey) {
    return false;
  }

  //  if (!rfid.PICC_IsNewCardPresent()) {
  //    return false;
  //  }
  //  // Verify if the NUID has been readed
  //  if (!rfid.PICC_ReadCardSerial()) {
  //    return false;
  //  }

  int maxBlockSize = 16;
  double length = strlen(value);
  double ratio = (length) / maxBlockSize;
  int N = ceil(ratio);
  aSerial.vv()
      .p(F("Value to write : "))
      .p(value)
      .p(F(" block counts :"))
      .pln(N);

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
  //  Write value over N separate blocks starting from 'from'
  bool writeStatus = true;
  for (int i = 0; i < N; ++i) {
    // int blockId = i + 8;
    int blockId = i + from;
    // if (blockId >= 11) blockId = i + 9;
    if (blockId >= 11) {
      blockId += 1;
    }
    if (blockId > 14) {
      break;
    }
    unsigned char content[maxBlockSize];
    for (int j = 0; j < maxBlockSize - 1; ++j) {
      int count = j + ((maxBlockSize - 1) * i);
      if (count > length)
        break;
      if (count < length) {
        content[j] = value[count];
        if (j == maxBlockSize - 2) {
          content[j + 1] = '\0';
        }
      } else if (count == length) {
        content[j] = '\0';
      }
    }
    //  byte byteArray[maxBlockSize * 2];
    //  strcat((char*)(&byteArray[0]), content);
    aSerial.vvvv()
        .p(F("Writing to block : "))
        .p(blockId)
        .p(F(" content "))
        .pln((char *)content)
        .p(" length : ")
        .pln(strlen((char *)content));
    if (!writeBlock(&key, blockId, content, maxBlockSize)) {
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
void onError(modules moduleName, char *error) {
  //  if (moduleName == EXEC) {
  //    aSerial.vv().p(F("[EXEC] error : ")).pln(error);
  //  } else if (moduleName == TRANSPORT) {
  //    aSerial.vv().p(F("[TRANSPORT] error : ")).pln(error);
  //  }
}

// CALLED on incoming http/mqtt/serial message
// MQTT pattern = "prefixedDevEui/+method/+objectId/+sensorId/+resourceId",
// HTTP pattern = "apiRoot/+collection/+path/#param"
void onMessage(transportLayer type, Message *message) {
  if (type == MQTT) {
    const char *method = message->getMethod();
    const char *objectId = message->getObjectId();
    const char *resourceId = message->getResourceId();
    const char *nodeId = message->getNodeId();
    const char *sensorId = message->getSensorId();
    char *payload = message->get(PAYLOAD);
    aSerial.vvv().p(F("onMessage : MQTT : ")).p(method).p(" ").pln(objectId);
    if (strcmp(objectId, "3306") == 0 && strcmp(resourceId, "5850") == 0) {
      if (strcmp(method, "1") == 0) {
        if ((strcmp(payload, "true") == 0 || strcmp(payload, "1") == 0)) {
          digitalWrite(RELAY_SWITCH, HIGH);
          digitalWrite(STATE_LED, LOW);
          aSerial.vvv().p(F("Value to write : ")).pln(payload);

        } else if ((strcmp(payload, "false") == 0 ||
                    strcmp(payload, "0") == 0)) {
          digitalWrite(RELAY_SWITCH, LOW);
          digitalWrite(STATE_LED, HIGH);
          aSerial.vvv().p(F("Value to write : ")).pln(payload);
        }
      } else if (strcmp(method, "2") == 0) {
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
    } else if (strcmp(objectId, "3341") == 0 &&
               strcmp(resourceId, "5527") == 0) {
      if (strcmp(method, "1") == 0) {
        goWriteTag = true;
      }
    } else if (strcmp(objectId, "0") == 0 && strcmp(resourceId, "1234") == 0) {
      if (strcmp(method, "1") == 0) {
        // updateFirmware = true;
        aloes.getFirmwareUpdate();
      }
    }
    return;
  }
  if (type == HTTP) {
    // if contenttype = json
    const char *method = message->getMethod();
    const char *collection = message->getCollection();
    const char *path = message->getPath();
    //  const char* body = message->getPayload();
    aSerial.vvv().p(F("onMessage : HTTP : ")).p(collection).p(" ").pln(path);
    return;
  }
}

void setup() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(DEFAULT_WIFI_SSID, DEFAULT_WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  if (!aloesSetup()) {
    return;
  }
  setupRFID();
}

void loop() {
  if (!aloesLoop()) {
    return;
  }

  if (!goWriteTag) {
    // if (readTag(8)) {
    if (readUid()) {
      // adapt sensor index to your own sensors.json
      int objectId = aloes.device->sensors->get(1, S_OBJECT_ID);
      int nodeId = aloes.device->sensors->get(1, S_NODE_ID);
      int sensorId = aloes.device->sensors->get(1, S_SENSOR_ID);
      int resourceId = aloes.device->sensors->get(1, S_RESOURCE_ID);
      aloes.setMsg(METHOD, "1")
          .setMsg(OBJECT_ID, objectId)
          .setMsg(NODE_ID, nodeId)
          .setMsg(SENSOR_ID, sensorId)
          .setMsg(RESOURCE_ID, resourceId);
      aloes.sendMessage(MQTT);
      return;
    }
  } else if (goWriteTag) {
    char *payload = aloes.getMsg(PAYLOAD);
    if (!writeTag(payload, 8)) {
      MilliSec lastWriteAttempt = millis();
      static AsyncWait retryWriteTag;
      if (writingTag == true) {
        goWriteTag = false;
        return;
      }
      if (retryWriteTag.isWaiting(lastWriteAttempt)) {
        return;
      }
      if (!writeTag(payload, 8)) {
        aSerial.vv().pln(F("Trying to write on tag ..."));
        retryWriteTag.startWaiting(lastWriteAttempt, 1000);
        return;
      }
      goWriteTag = false;
    }
    goWriteTag = false;
  }
}