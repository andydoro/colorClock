// test all segments

void testAllSegments(void) {
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
}
