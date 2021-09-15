#include <WioLTEforArduino.h>
#include <limits.h>

#define INTERVAL  (5000)

WioCellular Wio;

void setup() {
  delay(200);

  SerialUSB.begin(115200);
  SerialUSB.println("");
  SerialUSB.println("--- START ---------------------------------------------------");
  
  SerialUSB.println("### I/O Initialize.");
  Wio.Init();
  
  SerialUSB.println("### Power supply ON.");
  Wio.PowerSupplyCellular(true);
  delay(500);

  SerialUSB.println("### Turn on or reset.");
  if (!Wio.TurnOnOrReset()) {
    SerialUSB.println("### ERROR! ###");
    return;
  }
  delay(3000);

  SerialUSB.println("### Setup completed.");
}

void loop() {
  SerialUSB.println("### Get RSSI.");
  int rssi = Wio.GetReceivedSignalStrength();
  if (rssi == INT_MIN) {
    SerialUSB.println("### ERROR! ###");
    goto err;
  }
  SerialUSB.print("RSSI:");
  SerialUSB.print(rssi);
  SerialUSB.println("");
  
err:
  delay(INTERVAL);
}
