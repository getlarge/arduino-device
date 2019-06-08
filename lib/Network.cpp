#include "Network.h"

Network::Network(Config &config) {
  //  checkSerial();
}

void Network::checkSerial() {
  while (Serial.available() == 0 && millis() < 4000);
  //On timeout or availability of data, we come here.
  if (Serial.available() > 0) {
    //If data is available, we enter here.
    int test = Serial.read(); //We then clear the input buffer
    Serial.println("DEBUG"); //Give feedback indicating mode
    Serial.println(DEBUG);
    aSerial.on();
  } else {
    aSerial.off();
    //Serial.setDebugOutput(false);
  }
}

void Network::connect(Config &config) {
  String ssid = WiFi.SSID();
  String pass = WiFi.psk();
  //  IPAddress _ip, _gw, _sn;
  //  _ip.fromString(config.staticIp);
  //  _gw.fromString(config.staticGw);
  //  _sn.fromString(config.staticSn);
  //  String hostname(config.deviceName);
  WiFi.hostname(config.deviceName);
  //  WiFi.mode(WIFI_AP_STA);
  WiFi.mode(WIFI_STA);
  helpers.startTick(0.3);

  if ((strcmp(defaultWifiSSID, "") != 0) && (strcmp(defaultWifiPass, "") != 0)) {
    aSerial.vvv().p(F("Connecting to wifi :")).pln(defaultWifiSSID);
    WiFi.begin(defaultWifiSSID, defaultWifiPass);
  } else {
    aSerial.vvv().p(F("Connecting to wifi :")).pln(ssid);
    WiFi.begin(ssid.c_str(), pass.c_str());
  }
  wifiFailCount = 0;

  if (!MDNS.begin(config.deviceName)) {
    aSerial.vvv().pln(F("Error setting up MDNS responder!"));
  } else {
    aSerial.vvv().pln(F("mDNS responder started"));
  }

  while (WiFi.status() != WL_CONNECTED) {
    aSerial.vvv().p(F(" . "));
    wifiFailCount += 1;
    if (wifiFailCount > wifiMaxFailedCount && configMode == 0) {
      wifiFailCount = 0;
      callConfigMode = true;
      return;
      //  return manager.configManager(config);
    }
    delay(reconnectInterval);
  }
  // if (configMode == 1) {quitConfigMode();}
  helpers.stopTick();
  wifiFailCount = 0;
  aSerial.vv().p(F("WiFi connected. IP Address : ")).pln(WiFi.localIP());
}

bool Network::reconnect() {
  WiFi.disconnect();
  //  WiFi.mode(WIFI_AP_STA);
  WiFi.mode(WIFI_STA);

  String ssid = WiFi.SSID();
  String pass = WiFi.psk();
  //  IPAddress _ip, _gw, _sn;
  //  _ip.fromString(config.staticIp);
  //  _gw.fromString(config.staticGw);
  //  _sn.fromString(config.staticSn);
  aSerial.vvv().p(F("Connecting to wifi :")).pln(ssid);
  if ((strcmp(defaultWifiSSID, "") != 0) && (strcmp(defaultWifiPass, "") != 0)) {
    WiFi.begin(defaultWifiSSID, defaultWifiPass);
  } else {
    WiFi.begin(ssid.c_str(), pass.c_str());
  }
  wifiFailCount = 0;
  // Wait for connection
  helpers.startTick(0.3);
  for (int i = 0; i < wifiMaxFailedCount; i++) {
    if ( WiFi.status() != WL_CONNECTED ) {
      aSerial.vvv().p(F(" . "));
      delay (reconnectInterval);
    }
  }
  if (WiFi.status() == WL_CONNECTED && configMode == 1) {
    callConfigMode = false;
      //  return manager.quitConfigMode();
  }
  helpers.stopTick();
}



/// TIME
#if NTP_SERVER == 1
void digitalClockDisplay() {
  aSerial.vvv().p(hour());
  printDigits(minute());
  printDigits(second());
  aSerial.vvv().p("").p(day()).p(".").p(month()).p(".").pln(year());
}

void printDigits(int digits) {
  // utility for digital clock display: prints preceding colon and leading 0
  aSerial.vvv().p(":");
  if (digits < 10)
    aSerial.vvv().p('0');
  aSerial.vvv().p(digits);
}

void Network::setup() {
#if NTP_SERVER == 1
  aSerial.vvv().println(F("Starting UDP"));
  Udp.begin(localPort);
  aSerial.vvv().print(F("Local port : ")).println(Udp.localPort());
  aSerial.vvvv().println(F("Waiting for sync "));
  setSyncProvider(getNtpTime);
  setSyncInterval(300);
  //  delay(100);
#endif
}

void Network::loop() {
#if NTP_SERVER == 1
  if (timeStatus() != timeNotSet) {
    if (now() != prevDisplay) { //update the display only if time has changed
      prevDisplay = now();
      digitalClockDisplay();
    }
  }
#endif
}

// send an NTP request to the time server at the given address
void Network::sendNTPpacket(IPAddress& address) {
  aSerial.vvv().pln(F("Sending NTP packet ..."));
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

time_t Network::getNtpTime() {
  IPAddress ntpServerIP; // NTP server's ip address
  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  aSerial.vv().pln(F("Transmit NTP Request"));
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  aSerial.vv().p(ntpServerName).p(" : ").pln(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      aSerial.vvv().pln(F("Receiving NTP packet ..."));
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  aSerial.vv().pln(F("No NTP response"));
  return 0; // return 0 if unable to get the time
}

#endif
