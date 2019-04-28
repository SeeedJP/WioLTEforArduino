#include <WioLTEforArduino.h>

#define INTERVAL  (5000)

WioLTE Wio;

void setup() {
  SerialUSB.println("");
  SerialUSB.println("--- START ---------------------------------------------------");

  SerialUSB.println("### I/O Initialize.");
  Wio.Init();

  SerialUSB.println("### Power supply ON.");
  Wio.PowerSupplyLTE(true);
  delay(500);

  SerialUSB.println("### Power supply GNSS.");
  Wio.PowerSupplyGNSS(true);
  delay(500);

  SerialUSB.println("### Turn on or reset.");
  if (!Wio.TurnOnOrReset()) {
    SerialUSB.println("### ERROR! ###");
    //return;
  }

  SerialUSB.println("### Enable GNSS.");
  if (!Wio.enableGNSS()) {
    SerialUSB.println("### ERROR! ###");
  }

  SerialUSB.println("### Setup completed.");
}

void loop() {
  double longitude = 0.0;
  double latitude = 0.0;
  double altitude = 0.0;
  char utcTime[12];

  SerialUSB.println("### Get GNSS location.");
  bool getLocation = Wio.getGNSSLocation(&longitude, &latitude, &altitude, utcTime);

  if (getLocation) {
    SerialUSB.print("    long: ");
    SerialUSB.println(longitude, 6);
    SerialUSB.print("    lat: ");
    SerialUSB.println(latitude, 6);
    SerialUSB.print("    altitude: ");
    SerialUSB.println(altitude, 6);
    SerialUSB.print("    UTC time: ");
    SerialUSB.println(utcTime);

  } else {
    SerialUSB.println("### ERROR! ###");
  }

  delay(INTERVAL);
}
