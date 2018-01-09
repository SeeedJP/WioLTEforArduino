#include <WioLTEforArduino.h>

#define INTERVAL  (5000)

WioLTE Wio;

void setup() {
  delay(200);

  SerialUSB.println("");
  SerialUSB.println("--- START ---------------------------------------------------");
  
  SerialUSB.println("### I/O Initialize.");
  Wio.Init();
  
  SerialUSB.println("### Power supply ON.");
  Wio.PowerSupplyLTE(true);
  delay(500);

  SerialUSB.println("### Turn on or reset.");
  if (!Wio.TurnOnOrReset()) {
    SerialUSB.println("### ERROR! ###");
    return;
  }
  delay(3000);
  
  SerialUSB.println("### Sync time.");
  if (!Wio.SyncTime("ntp.nict.jp")) {
    SerialUSB.println("### ERROR! ###");
    return;
  }

  SerialUSB.println("### Setup completed.");
}

void loop() {
  SerialUSB.println("### Get time.");
  struct tm now;
  if (!Wio.GetTime(&now)) {
    SerialUSB.println("### ERROR! ###");
    goto err;
  }
  SerialUSB.print("UTC:");
  SerialUSB.println(asctime(&now));

err:
  delay(INTERVAL);
}
