#include "config.h"
// const char* sensors[][60] = {
//   { "3306", "1", "5850", "digital_input"},
//   { "3306", "2", "5850", "digital_input"},
//   { "3341", "3", "5527", "text_input"},
// };

#include <AloesDevice.h>

#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN D8
#define RST_PIN D0

byte nuidPICC[4];

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

void setupRFID();
void printHex();
void printDec();
void readTag();


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

/**
   Helper routine to dump a byte array as hex values to Serial.
*/
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    //  Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    //  Serial.print(buffer[i], HEX);
    aSerial.vvv().p(buffer[i] < 0x10 ? " 0" : " ");
    aSerial.vvv().p(buffer[i], HEX);
  }
}

/**
   Helper routine to dump a byte array as dec values to Serial.
*/
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    //static const size_t len = 1;
    aSerial.vvv().p(buffer[i] < 0x10 ? " 0" : " ");
    aSerial.vvv().p(buffer[i], DEC);
  }
}

void readTag() {
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;

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

  aSerial.vvvv().p(F("send to topic : ")).pln(postTopics[2]);

  mqttClient.publish(postTopics[2].c_str(), rfid.uid.uidByte, rfid.uid.size);
  if (rfid.uid.uidByte[0] != nuidPICC[0] ||
      rfid.uid.uidByte[1] != nuidPICC[1] ||
      rfid.uid.uidByte[2] != nuidPICC[2] ||
      rfid.uid.uidByte[3] != nuidPICC[3] ) {
    aSerial.vvv().pln(F("A new card has been detected"));

    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }
    aSerial.vvv().pln(F("The NUID tag is:"));
    aSerial.vvv().p(F("In hex : "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    aSerial.vvv().pln(F(""));
    aSerial.vvv().p(F("In dec : "));
    printDec(rfid.uid.uidByte, rfid.uid.size);
    aSerial.vvv().pln(F(""));

  }
  else aSerial.vvv().pln(F("Card read previously."));

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}


//  CALLED on incoming mqtt/serial message
void Aloes::onMessage(Message &message) {
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
      return aloes.sendMessage(config, message);
    }
  }
}

void setup() {
  initDevice();
  setupRFID();
}

void loop() {
  deviceRoutine();
}
