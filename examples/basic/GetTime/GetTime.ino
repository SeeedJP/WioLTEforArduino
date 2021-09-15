#include <WioLTEforArduino.h>

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
  char str[100];
  sprintf(str, "%04d/%02d/%02d %02d:%02d:%02d", now.tm_year + 1900, now.tm_mon + 1, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec);
  SerialUSB.println(str);

err:
  delay(INTERVAL);
}
