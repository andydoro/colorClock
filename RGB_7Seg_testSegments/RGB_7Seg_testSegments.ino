/*

  BGR order…

  C, D, E, F, G, A, B

  skip 0, 1, 2 or LED #0

*/

// PWM driver library
#include "Adafruit_TLC5947.h"

// DISPLAY STUFF
// =======
// How many boards do you have chained?
#define NUM_TLC5974 4

#define DATAPIN 12
#define CLKPIN 10
#define LATCHPIN 11

Adafruit_TLC5947 TLC = Adafruit_TLC5947(NUM_TLC5974, CLKPIN, DATAPIN, LATCHPIN);

int i = 0;
int j = 1;

void setup() {
  // put your setup code here, to run once:

  //Serial.begin(115200);

  TLC.begin();
}

void loop() {
  // put your main code here, to run repeatedly:

  TLC.setLED(i, 4095, 0, 0);
  TLC.setLED(i + 1, 0, 4095, 0);
  TLC.setLED(i + 2, 0, 0, 4095);

  TLC.write();

  i += 3;

  if (i > 30) {
    i = j;
    j++;
    if (j > 2) {
      j = 0;
    }
  }

  delay(20);

}
