
// Fill the dots one after the other with a color
/*
  void colorWipe(uint16_t r, uint16_t g, uint16_t b, uint8_t wait) {
  for (uint16_t i = 0; i < 8 * NUM_TLC5974; i++) {
    tlc.setLED(i, r, g, b);
    tlc.write();
    delay(wait);
  }
  }

  // Slightly different, this makes the rainbow equally distributed throughout
  void rainbowCycle(uint8_t wait) {
  uint32_t i, j;
  for (j = 0; j < 4096; j++) { // 1 cycle of all colors on wheel
    for (i = 0; i < 8 * NUM_TLC5974; i++) {
      Wheel(i, ((i * 4096 / (8 * NUM_TLC5974)) + j) & 4095);
    }
    tlc.write();
    delay(wait);
  }
  }
*/

// Like wheel but can control brightness
void WheelBright(uint8_t ledn, int WheelPos, int brightness) {


  if (WheelPos < 1365) {
    TLC.setLED(ledn, constrain(3 * WheelPos + brightness - 4095, 0, 4095), constrain(brightness - (3 * WheelPos), 0, 4095), 0);
  }
  else if (WheelPos < 2731) {
    WheelPos -= 1365;
    TLC.setLED(ledn, constrain(brightness - (3 * WheelPos), 0, 4095), 0, constrain(3 * WheelPos + brightness - 4095, 0, 4095));
  }
  else {
    WheelPos -= 2731;
    TLC.setLED(ledn, 0, constrain(3 * WheelPos + brightness - 4095, 0, 4095), constrain(brightness - (3 * WheelPos), 0, 4095));
  }
}


// Input a value 0 to 4095 to get a color value.
// The colours are a transition r - g - b - back to r.
void Wheel(uint8_t ledn, uint16_t WheelPos) {
  if (WheelPos < 1365) {
    TLC.setLED(ledn, 3 * WheelPos, 4095 - 3 * WheelPos, 0);
  }
  else if (WheelPos < 2731) {
    WheelPos -= 1365;
    TLC.setLED(ledn, 4095 - 3 * WheelPos, 0, 3 * WheelPos);
  }
  else {
    WheelPos -= 2731;
    TLC.setLED(ledn, 0, 3 * WheelPos, 4095 - 3 * WheelPos);
  }
}

void showNumeral(byte digit, byte number, int drift) {

  for (uint16_t i = 1 + (8 * digit); i < 1 + 8 * (digit + 1); i++) {
    boolean mask = bitRead(displayNum[number], i % 8);
    //Serial.print(mask);
    switch (mask) {
      case 0:
        TLC.setLED(i, 0, 0, 0);
        break;
      case 1:
        driftColor(i, drift);
        break;
    }
  }
  //Serial.println(" ");
}


//  show Numeral, but this time transition to next numeral
void showNumeralTrans(byte digit, byte oldNumber, byte newNumber, int drift, byte secs) {
  secs = secs - TRANSTIMECUTOFF;
  for (uint16_t i = 1 + (8 * digit); i < 1 + 8 * (digit + 1); i++) {
    boolean mask = bitRead(displayNum[oldNumber], i % 8);
    boolean maskTrans = bitRead(displayNum[newNumber], i % 8);

    // if both off, set to off
    if (!mask && !maskTrans)  {
      TLC.setLED(i, 0, 0, 0);
    }
    // if both on, stay on!
    else if (mask && maskTrans) {
      driftColor(i, drift);
    }
    // flickring on?
    else if (!mask && maskTrans) {
      if (random(100) < secs * 10) {
        driftColor(i, drift);
      }
      else {
        TLC.setLED(i, 0, 0, 0);
      }
    }
    // flickering off
    else if (mask && !maskTrans) {
      if (random(100) > secs * 10) {

        driftColor(i, drift);
      }
      else {
        TLC.setLED(i, 0, 0, 0);
      }
    }
  }

}


void countUp (byte digit, int countUpNumber, int wait) {
  for (int i = 0; i <= countUpNumber; i++) {
    showNumeral(digit, i, 4096);
    TLC.write();
    delay(wait);
  }
}


int rectifyVal (int colorValue) {
  if ((colorValue) < 0) {
    colorValue = 4096 + colorValue;
  }
  colorValue = colorValue % 4096;
  return colorValue;
}

void driftColor(byte colorIndex, int drift) {
  colorVal[colorIndex] += random(-1 * drift, drift);
  colorVal[colorIndex] = rectifyVal(colorVal[colorIndex]);
  WheelBright(colorIndex, colorVal[colorIndex], numBright);
}

void fadeDots(int _drift) {
  for (byte j = 0; j < 2; j++) {
    
    byte i = theDots[j]; // gets spareLED location from defined array

    colorVal[i] += random(-1 * _drift, _drift);
    colorVal[i] = rectifyVal(colorVal[i]);

    ledFade = constrain(ledFade, 0, 4095); // keep within bounds

    //Serial.println(ledFade);

    WheelBright(i, colorVal[i], ledFade);

    //Serial.print("colorval: ");
    //Serial.println(colorVal[i]);

  }

  if (fadeDirection == true) {
    ledFade += fadeAmount;
  } else {
    ledFade -= fadeAmount;
  }
  // if we go over... change directions
  if (ledFade >= 4095 || ledFade <= 0) {
    fadeDirection = !fadeDirection;
  }
}
