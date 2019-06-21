#ifndef Secure_Credentials_h
#define Secure_Credentials_h

#define CA_CERT 0
static const char CA_CERT_PROG[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----

-----END CERTIFICATE-----
)EOF";


#if CLIENT_SECURE == 1
#define FINGERPRINT 0
const uint8_t fingerprint[20] = {0x5A, 0xCF, 0xFE, 0xF0, 0xF1, 0xA6, 0xF4, 0x5F, 0xD2, 0x11, 0x11, 0xC6, 0x1D, 0x2F, 0x0E, 0xBC, 0x39, 0x8D, 0x50, 0xE0};

#define CLIENT_CERT 0
// const char CLIENT_CERT_PROG[] PROGMEM = R"EOF(
//-----BEGIN CERTIFICATE-----
//-----END CERTIFICATE-----
//)EOF";
//
#define CLIENT_KEY 0
//// KEEP THIS VALUE PRIVATE AND SECURE!!!
//const char CLIENT_KEY_PROG[] PROGMEM = R"KEY(
//-----BEGIN RSA PRIVATE KEY-----
//-----END RSA PRIVATE KEY-----
//)KEY";
//
#define SERVER_KEY  0
//const char SERVER_KEY_PROG[] PROGMEM = R"KEY(
//-----BEGIN RSA PUBLIC KEY-----
//-----END RSA PUBLIC KEY-----
//)KEY";
#endif

#endif