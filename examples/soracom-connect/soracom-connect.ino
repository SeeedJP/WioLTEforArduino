#include <wiolte-driver.h>

void setup() {
  delay(200);

  SerialUSB.println("");
  SerialUSB.println("--- START ---------------------------------------------------");
  
  WioLTE wio;
  
  SerialUSB.println("### I/O Initialize.");
  wio.Init();
  
  SerialUSB.println("### Power supply ON.");
  wio.PowerSupplyLTE(true);
  delay(5000);

  SerialUSB.println("### Turn on or reset.");
  if (!wio.TurnOnOrReset()) {
    SerialUSB.println("### ERROR! ###");
    return;
  }

  SerialUSB.println("### Connect the \"soracom.io\".");
  delay(5000);
  if (!wio.Activate("soracom.io", "sora", "sora")) {
    SerialUSB.println("### ERROR! ###");
    return;
  }

  SerialUSB.println("### Finish.");
}

void loop() {
}

