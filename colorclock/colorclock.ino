/***************************************************

  HARDWARE
  ========
  - RGB 7-segment x 4
  https://www.adafruit.com/product/1399
  - using custom made backpack based on TLC5947
  - DS1307 RTC


  TO DO:
    - write a library for backpack?
    - get rid of first 0? maybe?

  DONE:
    - 12/24 hour time flag
    - DST correction
    - if we want other digits besides #4 to transition, gonna have to do more complicated programming :)
  maybe use Unixtime addition, do some quick figuring out which digits will change?
    - dim at nighttime
    - fade dots instead of blinking?

 ****************************************************/

// PWM driver library
#include "Adafruit_TLC5947.h"

// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "RTClib.h"
#include "DST_RTC.h"

// TIME STUFF
// =======
// Do you live in a country or territory that observes Daylight Saving Time?
// https://en.wikipedia.org/wiki/Daylight_saving_time_by_country
// This is programmed for DST in the US / Canada. If your territory's DST operates differently,
// you'll need to modify the code in the calcTheTime() function to make this work properly.
//const boolean OBSERVE_DST = true;
const boolean DISPLAY_24HOURTIME = true; // display 24 hour time?

#define SECONDS_PER_HOUR 3600

RTC_DS1307 RTC; // clock object
DST_RTC DST;

DateTime theTime;

byte clockDigits[4];

// when does it start transitioning?
#define TRANSTIMECUTOFF 55


// DISPLAY STUFF
// =======
// How many boards do you have chained?
#define NUM_TLC5974 4

#define DATAPIN 12
#define CLKPIN 10
#define LATCHPIN 11
//#define oe -1  // set to -1 to not use the enable pin (its optional)


Adafruit_TLC5947 TLC = Adafruit_TLC5947(NUM_TLC5974, CLKPIN, DATAPIN, LATCHPIN);

// how to display numerals, bitmasks

// numerals mask
// B A G F E D C
// last 0 is filler for spare 0, 1, 2 output
const byte displayNum[10] = {
  B11011110, // 0
  B10000010, // 1
  B11101100, // 2
  B11100110, // 3
  B10110010, // 4
  B01110110, // 5
  B01111110, // 6
  B11000010, // 7
  B11111110, // 8
  B11110110  // 9
};

// where do the dots go? these use the spare 3 PWMs from each backpack
// to do: create function/library to get this spareLED From backpack number
// these are multiples of 8;   0, 8, 16, etc.
const byte theDots[2] = {8, 16};

// hold color values for each segment
int colorVal[8 * NUM_TLC5974];


// DOTS LED FADE
int ledFade; // for PWM brightness
boolean fadeDirection = true; // flip back and forth for pulsing effect

const int normalFadeAmount = 32;

int fadeAmount = normalFadeAmount;
byte fadeCounter = 0;


// night dimming
// brightness based on time of day- could try warmer colors at night?
// 0-4095
#define DAYBRIGHTNESS 4095
#define NIGHTBRIGHTNESS 3072

// cutoff times for day / night brightness. feel free to modify.
#define MORNINGCUTOFF 7  // when does daybrightness begin?   7am
#define NIGHTCUTOFF 22 // when does nightbrightness begin? 10pm

int numBright = DAYBRIGHTNESS;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  RTC.begin();

  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
    // DST? If we're in it, let's subtract an hour from the RTC time to keep our DST calculation correct. This gives us
    // Standard Time which our DST check will add an hour back to if we're in DST.
    DateTime standardTime = RTC.now();
    if (DST.checkDST(standardTime) == true) { // check whether we're in DST right now. If we are, subtract an hour.
      standardTime = standardTime.unixtime() - 3600;
    }
    RTC.adjust(standardTime);
  }

  //Serial.println("TLC5974 test");
  TLC.begin();


  delay(500);

  // test all segments

  int i = 0;
  int j = 1;

  for (int k = 0; k < 64; k++) {
    TLC.setLED(i, 4095, 0, 0); // R
    TLC.setLED(i + 1, 0, 4095, 0); // G
    TLC.setLED(i + 2, 0, 0, 4095); // B

    TLC.write();

    i += 3;

    if (i > 30) {
      i = j;
      j++;
      if (j > 2) {
        j = 0;
      }
    }

    delay(40);
  }

  // clear
  for (int i = 0; i < 8 * NUM_TLC5974; i++) {
    TLC.setLED(i, 0, 0, 0);
  }
  TLC.write();

  delay(500);


  // pick random values for segments
  for (byte i = 0; i < 8 * NUM_TLC5974; i++) {
    colorVal[i] = random(4096);
  }

  delay(500);

  Serial.println("startup seq");
  for (byte i = 0; i < NUM_TLC5974; i++) {
    countUp(i, 9, 100);
  }

  //theTime = RTC.now();
  theTime = DST.calculateTime(RTC.now()); // takes into account DST

  byte minutes = theTime.minute();
  byte hours = theTime.hour();
  if (DISPLAY_24HOURTIME == false) {
    hours = calc12hour(hours);
  }

  clockDigits[0] = hours / 10;
  clockDigits[1] = hours % 10;
  clockDigits[2] = minutes / 10;
  clockDigits[3] = minutes % 10;

  for (byte i = 0; i < NUM_TLC5974; i++) {
    countUp(i, clockDigits[i], 50);
  }
}

void loop() {

  // get time from the RTC
  //theTime = RTC.now();
  theTime = DST.calculateTime(RTC.now()); // takes into account DST

  printTheTime(theTime);

  byte seconds = theTime.second();
  byte minutes = theTime.minute();
  byte hours = theTime.hour();


  // adjust brightness based on hour
  // check less often? like once a minute?
  if (theTime.second() == 0) {
    if (hours < MORNINGCUTOFF || hours >= NIGHTCUTOFF) {
      numBright = NIGHTBRIGHTNESS;
    } else {
      numBright = DAYBRIGHTNESS;
    }
  }

  if (DISPLAY_24HOURTIME == false) {
    hours = calc12hour(hours);
  }

  clockDigits[0] = hours / 10;
  clockDigits[1] = hours % 10;
  clockDigits[2] = minutes / 10;
  clockDigits[3] = minutes % 10;

  int dryft = pow(seconds, 1.3);

  if (seconds > TRANSTIMECUTOFF) { // numerals about to change

    // figure out which digits are changing
    DateTime futureTime = theTime.unixtime() + 30; // calculate time 30 seconds in the future
    byte futureHour = futureTime.hour();
    byte futureMin = futureTime.minute();
    if (DISPLAY_24HOURTIME == false) {
      futureHour = calc12hour(futureHour);
    }

    byte futureDigits[4] = {
      futureHour / 10,
      futureHour % 10,
      futureMin / 10,
      futureMin % 10 // kind of redundant since this digit is totally predicatable?
    };
    // compare digits
    for (byte i = 0; i < 4; i++) {
      if (clockDigits[i] != futureDigits[i]) {
        showNumeralTrans(i, clockDigits[i], futureDigits[i], dryft, seconds);
      } else {
        showNumeral(i, clockDigits[i], dryft);
      }
    }
  } else {
    for (byte i = 0; i < 4; i++) {
      showNumeral(i, clockDigits[i], dryft);
    }
  }

  // fade the dots
  fadeDots(dryft);

  //Serial.println(fadeCounter);

  // write our changes
  TLC.write();

}
