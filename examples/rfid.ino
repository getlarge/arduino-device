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

byte nuidPICC[4];

void setupRFID();
void printHex();
void printDec();
void readTag(char* sensorId);

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

void parseResult(byte *buffer, byte bufferSize, char* sensorId) {
  aloes.setPayload(buffer, bufferSize, "DEC");
  char* payload = aloes.getMsg(PAYLOAD);
  aloes.setMsg(METHOD, "1").setMsg(OBJECT_ID, "3341").setMsg(SENSOR_ID, sensorId).setMsg(RESOURCE_ID, "5527");
  //  aloes.setMessage("objectId", "3341");
  //  aloes.setMessage("sensorId", sensorId);
  //  aloes.setMessage("resourceId", "5527");
  aSerial.vvv().p(F("The NUID tag is : ")).pln(payload);

  aloes.sendMessage(MQTT);

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

}

void readTag(char* sensorId) {
  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }
  // Verify if the NUID has been readed
  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }
  //  aSerial.vvv().p(F("PICC type: "));
  // Dump debug info about the card; PICC_HaltA() is automatically called
  //  rfid.PICC_DumpToSerial(&(rfid.uid));

  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  aSerial.vvv().pln(rfid.PICC_GetTypeName(piccType));
  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    aSerial.vvv().pln(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  //  aSerial.vvvv().p(F("send to topic : ")).pln(postTopics[2]);
  parseResult(rfid.uid.uidByte, rfid.uid.size, sensorId);

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

//  CALLED on errors
void onError(modules module, char* error) {
  if (module == TRANSPORT) {
    aSerial.vv().p(F("[TRANSPORT] error : ")).pln(error);
  }
}

// CALLED on incoming http/mqtt/serial message
// MQTT pattern = "prefixedDevEui/+method/+objectId/+sensorId/+resourceId",
// HTTP pattern = "apiRoot/+collection/+path/#param"
void onMessage(transportLayer transportType, Message *message) {
  if (transportType == MQTT) {
    char* payload = message->get(PAYLOAD);
    const char* method = message->get(METHOD);
    const char* objectId = message->get(OBJECT_ID);
    const char* resourceId = message->get(RESOURCE_ID);
    const char* sensorId = message->get(SENSOR_ID);
    //  aSerial.vvv().p(F("onMessage : MQTT : ")).p(method).p(" ").pln(objectId);
    if ( strcmp(objectId, "3306") == 0 && strcmp(resourceId, "5850") == 0) {
      if ( strcmp(method, "1") == 0 ) {
        if ( ( strcmp(payload, "true") == 0 || strcmp(payload, "1") == 0 )) {
          digitalWrite(RELAY_SWITCH, HIGH);
          digitalWrite(STATE_LED, LOW);
          //  aloes.getFirmwareUpdate();
          return;
        } else if (( strcmp(payload, "false") == 0 ||  strcmp(payload, "0") == 0 )) {
          digitalWrite(RELAY_SWITCH, LOW);
          digitalWrite(STATE_LED, HIGH);
          return;
        }
      } else if ( strcmp(method, "2") == 0 ) {
        int val = digitalRead(RELAY_SWITCH);
        char newPayload[10];
        itoa(val, newPayload, 10);
        message->set(METHOD, "1").set(PAYLOAD, newPayload);
        if (aloes.sendMessage(MQTT)) {
          aSerial.vvv().p(F("Message sent"));
        } else {
          aSerial.vvv().p(F("Message failed to be sent"));
        }
        return;
      }
    } else if ( strcmp(objectId, "0") == 0 && strcmp(resourceId, "1234") == 0) {
      if ( strcmp(method, "1") == 0 ) {
        aloes.getFirmwareUpdate();
        return;
      }
    }
  } else if (transportType == HTTP) {
    const char* method = message->get(METHOD);
    const char* collection = message->get(COLLECTION);
    const char* path = message->get(PATH);
    //  const char* body = message->get(PAYLOAD);

    //  aSerial.vvv().p(F("onMessage : HTTP : ")).p(collection).p(" ").pln(path);
    return;
  }
}

void setup() {
  if (!initDevice()) {
    return;
  }
  setupRFID();
}

void loop() {
  if (!deviceRoutine()) {
    return;
  }
  readTag((char*)"3");
}