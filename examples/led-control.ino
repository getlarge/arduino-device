#include "config.h"
// const char* sensors[][60] = {
//   { "3306", "1", "5850", "digital_input"},
//   { "3311", "2", "5851", "float_input"}
// };
#include <AloesDevice.h>

#include <Adafruit_NeoPixel.h>

#define STRIP_COUNT 2

#if STRIP_COUNT >= 1
#define LED_PIN   D5  // D5 or D2 //  Pin for the pixel strand. Does not have to be analog.
#define LED_TOTAL 60  // 60 or 40 //  Change this to the number of LEDs in your strand.
#define LED_HALF  LED_TOTAL/2
Adafruit_NeoPixel strand = Adafruit_NeoPixel(LED_TOTAL, LED_PIN, NEO_GRB + NEO_KHZ800);
#endif
#if STRIP_COUNT >= 2
#define LED_PIN_2   D6  // D6
#define LED_TOTAL_2 60  // 60 or 40 // 
#define LED_HALF_2  LED_TOTAL_2/2
Adafruit_NeoPixel strand2 = Adafruit_NeoPixel(LED_TOTAL_2, LED_PIN_2, NEO_GRB + NEO_KHZ800);
#endif


uint16_t gradient = 0; // Used to iterate and loop through each color palette gradually
//  uint8_t volume = 0;
double volume = 0;
uint8_t lastVol = 0;      // Holds the value of volume from the previous loop() pass.
float maxVol = 300;     // Holds the largest volume recorded thus far to proportionally adjust the visual's responsiveness.
float knob = 1023.0;   // Holds the percentage of how twisted the trimpot is. Used for adjusting the max brightness.
float avgVol = 0;      // Holds the "average" volume-level to proportionally adjust the visual experience.
float avgBump = 0;     // Holds the "average" volume-change to trigger a "bump."
bool bump = false;     // Used to pass if there was a "bump" in volume
uint32_t ledColor;

void setupNeopixel();
void interpolateVolume(double average);
void bumpLed(double volume);
void turnOffLed();
void pulse();
void fade(float damper);
uint8_t split(uint32_t color, uint8_t i );
uint32_t rainbow(unsigned int i);


void setupNeopixel() {
  strand.begin(); //Initialize the LED strand object.
  //  strand.setBrightness(120);
  strand.show();  //Show a blank strand, just to get the LED's ready for use.
#if STRIP_COUNT >= 2
  strand2.begin();
  strand2.show();
#endif
  if (!ledColor) {
    ledColor = strand.Color(10, 250, 10);
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  fade(0.75);
  if (volume > 0) {
    uint16_t i;
    uint16_t j;
    int pixelsCount = map(volume, 0, maxVol, 0, strand.numPixels());
    //  aSerial.v().p(F("colorWipe pixels:")).pln(pixelsCount);

    for (j = pixelsCount; j < strand.numPixels(); j++) {
      strand.setPixelColor(j, strand.Color(0, 0, 0));
#if STRIP_COUNT >= 2
      strand2.setPixelColor(j, strand.Color(0, 0, 0));
#endif
    }
    for (i = 0; i < pixelsCount; i++) {
      strand.setPixelColor(i, c);
#if STRIP_COUNT >= 2
      strand2.setPixelColor(i, c);
#endif
    }
    //aSerial.v().p(F("colorWipe loop :")).pln(i);
    strand.setBrightness(255.0 * pow(volume / maxVol, 2));
    strand.show();
#if STRIP_COUNT >= 2
    strand2.setBrightness(255.0 * pow(volume / maxVol, 2));
    strand2.show();
#endif

  }
}

void interpolateVolume(double average) {
  volume = average;
  //  if (average > maxVol) return;
  //  aSerial.v().p(F("interpolateVolume , volume : ")).p(average).p(F(" avgVolume : ")).pln(avgVol);
  avgVol = (avgVol + volume) / 2.0;
  if (volume < avgVol / 2.0 || volume < 15) volume = 0;

  //  if (volume > maxVol) maxVol = volume;
  if (volume > maxVol) volume = maxVol;
  //This is where "gradient" is reset to prevent overflow.
  if (gradient > 1529) {
    gradient %= 1530;
    //Everytime a palette gets completed is a good time to readjust "maxVol," just in case
    //  the song gets quieter; we also don't want to lose brightness intensity permanently
    //  because of one stray loud sound.
    //  maxVol = (maxVol + volume) / 2.0;
  }

  //If there is a decent change in volume since the last pass, average it into "avgBump"
  if (volume - lastVol > avgVol - lastVol && avgVol - lastVol > 0) avgBump = (avgBump + (volume - lastVol)) / 2.0;
  bump = (volume - lastVol) > avgBump;
  //pulse();   //Calls the visual to be displayed with the globals as they are.
  colorWipe(ledColor, 20);
  gradient++;    //Increments gradient
  lastVol = volume; //Records current volume for next pass
}

void turnOffLed() {
  uint32_t black = strand.Color(0, 0, 0);
  strand.fill(black);
  strand.show();
#if STRIP_COUNT >= 2
  strand2.fill(black);
  strand2.show();
#endif
}

//PULSE
//Pulse from center of the strand
void pulse() {
  fade(0.75);  
  //  Serial.print("pulse : ");
  //  float average = map(sensorValue, 0, 3300, 0, 100);
  //  Serial.println(volume);
  //Advances the gradient to the next noticeable color if there is a "bump"
  if (bump) gradient += 64;

  //If it's silent, we want the fade effect to take over, hence this if-statement
  if (volume > 0) {
    uint32_t col = rainbow(gradient); //Our retrieved 32-bit color

    // These variables determine where to start and end the pulse since it starts from the middle of the strand.
    //  The quantities are stored in variables so they only have to be computed once.
    int start = LED_HALF - (LED_HALF * (volume / maxVol));
    int finish = LED_HALF + (LED_HALF * (volume / maxVol)) + strand.numPixels() % 2;
    //Listed above, LED_HALF is simply half the number of LEDs on your strand. ↑ this part adjusts for an odd quantity.

    for (int i = start; i < finish; i++) {

      //"damp" creates the fade effect of being dimmer the farther the pixel is from the center of the strand.
      //  It returns a value between 0 and 1 that peaks at 1 at the center of the strand and 0 at the ends.
      float damp = float(
                     ((finish - start) / 2.0) -
                     abs((i - start) - ((finish - start) / 2.0))
                   )
                   / float((finish - start) / 2.0);

      //Sets the each pixel on the strand to the appropriate color and intensity
      //  strand.Color() takes 3 values between 0 & 255, and returns a 32-bit integer.
      //  Notice "knob" affecting the brightness, as in the rest of the visuals.
      //  Also notice split() being used to get the red, green, and blue values.
      strand.setPixelColor(i, strand.Color(
                             split(col, 0) * pow(damp, 2.0) * knob,
                             split(col, 1) * pow(damp, 2.0) * knob,
                             split(col, 2) * pow(damp, 2.0) * knob
                           ));
    }
    //Sets the max brightness of all LEDs. If it's loud, it's brighter.
    //  "knob" was not used here because it occasionally caused minor errors in color display.
    strand.setBrightness(255.0 * pow(volume / maxVol, 2));
  }
  strand.show();
}

//Fades lights by multiplying them by a value between 0 and 1 each pass of loop().
void fade(float damper) {
  //"damper" must be between 0 and 1, or else you'll end up brightening the lights or doing nothing.
  if (damper >= 1) damper = 0.99;
  for (int i = 0; i < strand.numPixels(); i++) {
    //Retrieve the color at the current position.
    uint32_t col = (strand.getPixelColor(i)) ? strand.getPixelColor(i) : strand.Color(0, 0, 0);
    //If it's black, you can't fade that any further.
    if (col == 0) continue;
    float colors[3]; //Array of the three RGB values
    //Multiply each value by "damper"
    for (int j = 0; j < 3; j++) colors[j] = split(col, j) * damper;
    //Set the dampened colors back to their spot.
    strand.setPixelColor(i, strand.Color(colors[0] , colors[1], colors[2]));
#if STRIP_COUNT >= 2
    strand2.setPixelColor(i, strand2.Color(colors[0] , colors[1], colors[2]));
#endif
  }
}

void bumpLed(double volume) {
  interpolateVolume(volume);
  delay(250);
  fade(0.75);
  delay(250);
  turnOffLed();
}

uint8_t split(uint32_t color, uint8_t i ) {
  //0 = Red, 1 = Green, 2 = Blue
  if (i == 0) return color >> 16;
  if (i == 1) return color >> 8;
  if (i == 2) return color >> 0;
  return -1;
}

uint32_t rainbow(unsigned int i) {
  if (i > 1529) return rainbow(i % 1530);
  if (i > 1274) return strand.Color(255, 0, 255 - (i % 255));   //violet -> red
  if (i > 1019) return strand.Color((i % 255), 0, 255);         //blue -> violet
  if (i > 764) return strand.Color(0, 255 - (i % 255), 255);    //aqua -> blue
  if (i > 509) return strand.Color(0, 255, (i % 255));          //green -> aqua
  if (i > 255) return strand.Color(255 - (i % 255), 255, 0);    //yellow -> green
  return strand.Color(255, i, 0);                               //red -> yellow
}

//  CALLED on incoming mqtt/serial message
void Aloes::onMessage(Message &message) {
  if ( strcmp(message.omaObjectId, "3311") == 0) {
    if ( strcmp(message.omaResourceId, "5851") == 0) {
      if ( strcmp(message.method, "1") == 0 ) {
        double average = atof(message.payload);
        //  int average = atoi(message.payload);
        return interpolateVolume(average);
      } else if ( strcmp(message.method, "2") == 0 ) {
        char payload[10];
        dtostrf(lastVol, 10, 0, payload);
        aloes.setMessage(message, (char*)"1", message.omaObjectId, message.sensorId, message.omaResourceId, payload);
        return aloes.sendMessage(config, message);
      }
    } else if ( strcmp(message.omaResourceId, "5706") == 0) {
      if ( strcmp(message.method, "1") == 0 ) {
        if ( strcmp(message.payload, "red") == 0 ) {
          ledColor = strand.Color(255, 10, 10);
        } else if ( strcmp(message.payload, "blue") == 0 ) {
          ledColor = strand.Color(10, 10, 250);
        } else if ( strcmp(message.payload, "green") == 0 ) {
          ledColor = strand.Color(10, 250, 10);
        }
      } else if ( strcmp(message.method, "2") == 0 ) {
        // publish ledColor
        char payload[10];
        dtostrf(ledColor, 10, 0, payload);
        aloes.setMessage(message, (char*)"1", message.omaObjectId, message.sensorId, message.omaResourceId, payload);
        return aloes.sendMessage(config, message);
      }
    }
  } else if ( strcmp(message.omaObjectId, "3306") == 0 ) {
    if ( strcmp(message.omaResourceId, "5850") == 0) {
      if ( strcmp(message.method, "1") == 0 ) {
        if ( ( strcmp(message.payload, "true") == 0 || strcmp(message.payload, "1") == 0 )) {
          digitalWrite(STATE_LED, LOW);
          double volume = 100;
          return bumpLed(volume);
        } else if (( strcmp(message.payload, "false") == 0 || strcmp(message.payload, "0") == 0 )) {
          digitalWrite(STATE_LED, HIGH);
          return turnOffLed();
        }
      } else if ( strcmp(message.method, "2") == 0 ) {
        int val = digitalRead(STATE_LED);
        char payload[10];
	      itoa(val, payload, 10);
	      aloes.setMessage(message, (char*)"1", message.omaObjectId, message.sensorId, message.omaResourceId, payload);
	      return aloes.sendMessage(config, message);
      }
    }
  }
}


void setup() {
  initDevice();
  setupNeopixel();
}

void loop() {
  deviceRoutine();
}
