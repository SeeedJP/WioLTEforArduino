#include <wiolte-driver.h>

#define INTERVAL  (50)

WioLTE Wio;
int Hue = 0;

void setup() {
  delay(200);

  SerialUSB.println("");
  SerialUSB.println("--- START ---------------------------------------------------");
  
  SerialUSB.println("### I/O Initialize.");
  Wio.Init();
}

void loop() {
  int r;
  int g;
  int b;
  
  if (Hue < 60) {
    r = 255;
    g = Hue * 255 / 60;
    b = 0;
  }
  else if (Hue < 120) {
    r = (120 - Hue) * 255 / 60;
    g = 255;
    b = 0;
  }
  else if (Hue < 180) {
    r = 0;
    g = 255;
    b = (Hue - 120) * 255 / 60;
  }
  else if (Hue < 240) {
    r = 0;
    g = (240 - Hue) * 255 / 60;
    b = 255;
  }
  else if (Hue < 300) {
    r = (Hue - 240) * 255 / 60;
    g = 0;
    b = 255;
  }
  else {
    r = 255;
    g = 0;
    b = (360 - Hue) * 255 / 60;
  }
  
  Wio.LedSetRGB(r, g, b);

  Hue += 10;
  if (Hue >= 360) Hue = 0;
  
  delay(INTERVAL);
}
